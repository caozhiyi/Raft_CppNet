#include "Log.h"
#include "CClient.h"
#include "CParser.h"
#include "common.h"
#include "CNode.h"

CClient::CClient(CNode* cur_node) : _cur_node(cur_node) {

}


CClient::~CClient() {

}

bool CClient::Init(std::string ip, int port) {
	// set call back
	_net.SetReadCallback(std::bind(&CClient::_ReadCallBack, this, std::placeholders::_1, std::placeholders::_2));
	_net.SetWriteCallback(std::bind(&CClient::_WriteCallBack, this, std::placeholders::_1, std::placeholders::_2));

	// start net with 1 thread
	_net.Init(1);
	_socket =  _net.Connection(port, ip);

	return true;
}

void CClient::SendMsg(const std::string& msg) {
	// make msg string
	std::string msg_str = CParser::Encode(msg);

	std::unique_lock<std::mutex> lock(_socket_mutex);
	auto iter = _socket_map.find(ip_port);
	if (iter != _socket_map.end()) {
		iter->second->SyncWrite(msg_str.c_str(), msg_str.length());
	}
}

// net io
void CClient::_ReadCallBack(CMemSharePtr<CSocket>& socket, int err) {
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
	int body_len = msg->_head._body_len;
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
	if (msg._head._status == RAFT_RELEADER) {
		std::string ip = msg._msg.substr(0, msg._msg.find(":"));
		int port = std::to_integer(msg._msg.substr(msg._msg.find(":") + 1));
		socket->SyncDisconnection();
		socket = _net.Connection(ip, port);
	}

	if (_call_back) {
		_call_back(msg._head._status);
	}
}

void CClient::_WriteCallBack(CMemSharePtr<CSocket>& socket, int err) {
	std::string ip = socket->GetAddress();
	short port = socket->GetPort();
	ip.append(":");
	ip.append(std::to_string(port));

	if (err & EVENT_ERROR_CLOSED) {
		LOG_ERROR("a connect lost msg. err:%d, ip:%s", err, ip.c_str());
		_socket.Reset();
		return;
	}

	LOG_INFO("send a msg to %s", ip.c_str());;
}