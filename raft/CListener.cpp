#include "Log.h"
#include "CListener.h"
#include "CParser.h"
#include "CNode.h"

CListener::CListener(CNode* cur_node) : _cur_node(cur_node) {

}


CListener::~CListener() {

}

bool CListener::Init(std::string ip, int port) {
	// set call back
	_net.SetAcceptCallback(std::bind(&CListener::_AcceptCallBack, this, std::placeholders::_1, std::placeholders::_2));
	_net.SetReadCallback(std::bind(&CListener::_ReadCallBack, this, std::placeholders::_1, std::placeholders::_2));
	_net.SetWriteCallback(std::bind(&CListener::_WriteCallBack, this, std::placeholders::_1, std::placeholders::_2));

	// start net with 2 thread
	_net.Init(2);
	_net.ListenAndAccept(port, ip);

	return true;
}

void CListener::SendMsg(Time version, int status) {
	std::unique_lock<std::mutex> lock(_send_mutex);
	while (!_send_queue.empty()) {
		auto item = _send_queue.front();
		if (item.first < version) {
			item.second(status);
			_send_queue.pop();
		
		} else {
			break;
		}
	}
}

void CListener::SendMsg(const std::string& ip_port, ClientMsg& msg) {
	// make msg string
	std::string msg_str = CParser::Encode(msg);

	std::unique_lock<std::mutex> lock(_socket_mutex);
	auto iter = _socket_map.find(ip_port);
	if (iter != _socket_map.end()) {
		iter->second->SyncWrite(msg_str.c_str(), msg_str.length());
	}
}

// net io
void CListener::_ReadCallBack(CMemSharePtr<CSocket>& socket, int err) {
	std::string ip = socket->GetAddress();
	short port = socket->GetPort();
	ip.append(":");
	ip.append(std::to_string(port));

	if (!_cur_node->IsLeader()) {
		ClientMsg msg;
		msg._head._status = RAFT_RELEADER;
		msg._msg = _cur_node->GetLeaderInfo();
		SendMsg(ip, msg);
		LOG_INFO("Not a leader get a msg from %s", ip.c_str());
		return;
	}


	if (err & EVENT_ERROR_CLOSED) {
		LOG_ERROR("a connect lost msg. err:%d, ip:%s", err, ip.c_str());
		std::unique_lock<std::mutex> lock(_socket_mutex);
		auto iter = _socket_map.find(ip);
		if (iter != _socket_map.end()) {
			_socket_map.erase(iter);
		}
		return;
	}

	if (err != EVENT_ERROR_NO) {
		LOG_ERROR("Read msg error. err:%d, ip:%s ", err, ip.c_str());
	}
	int len = socket->_read_event->_off_set;
	if (len < client_header_len) {
		socket->SyncRead();
		return;
	}

	char buf[4096] = {0};
	// read header
	len = socket->_read_event->_buffer->ReadNotClear(buf, header_len);
	if (len < client_header_len) {
		socket->SyncRead();
		return;
	}

	ClientMsg msg;
	msg = CParser::DecodeClient(buf);
	int body_len = msg._head._body_len;
	if (body_len > 0) {
		len = socket->_read_event->_buffer->GetCanReadSize();
		// not recv a complete msg
		if (len < body_len + header_len) {
			socket->SyncRead();
			return;

		} else {
			socket->_read_event->_buffer->Read(buf, body_len + header_len);
			msg = CParser::DecodeClient(buf);
		}

	} else {
		// clean the buffer
		socket->_read_event->_buffer->Read(buf, header_len);
	}
	
	LOG_INFO("get a msg from %s", ip.c_str());
	
	std::pair<Time, SendCallBack> item;

	BinLog log;
	log.first = _cur_node->_bin_log.GetUTC();
	log.second = msg._msg;

	item.first = log.first;
	item.second = [socket](int status) {
		if (!socket.Expired()) {
			return;
		}
		ClientMsg msg;
		msg._head._status = status;
		msg._head._body_len = 0;

		std::string msg_str = CParser::Encode(msg);
		socket->SyncWrite(msg_str.c_str(), msg_str.length());
	};

	{
		std::unique_lock<std::mutex> lock(_send_mutex);
		_send_queue.push(item);
	}
	_cur_node->HandleClient(log);
    socket->SyncRead();
}

void CListener::_WriteCallBack(CMemSharePtr<CSocket>& socket, int err) {
	std::string ip = socket->GetAddress();
	short port = socket->GetPort();
	ip.append(":");
	ip.append(std::to_string(port));

	if (err & EVENT_ERROR_CLOSED) {
		LOG_ERROR("a connect lost msg. err:%d, ip:%s", err, ip.c_str());
		std::unique_lock<std::mutex> lock(_socket_mutex);
		auto iter = _socket_map.find(ip);
		if (iter != _socket_map.end()) {
			_socket_map.erase(iter);
		}
		return;
	}

	LOG_INFO("send a msg to %s", ip.c_str());;
}

void CListener::_AcceptCallBack(CMemSharePtr<CSocket>& socket, int err) {
	std::string ip = socket->GetAddress();
	short port = socket->GetPort();
	ip.append(":");
	ip.append(std::to_string(port));

	_socket_map[ip] = socket;
	socket->SyncRead();
	LOG_INFO("recv a connect from ip  %s", ip.c_str());
}