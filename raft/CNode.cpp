#include "Log.h"
#include "CNode.h"
#include "CZkNodeInfo.h"
#include "CParser.h"

#include <iostream>

CNode::CNode(const std::string& config_path) :
    _done_msg(false),
	_role(Follower),
	_client_listener(nullptr),
	_config_path(config_path) {

}


CNode::~CNode() {
    _bin_log.Stop();
    StopNet();
}

bool CNode::Init() {
	if (_client_listener) {
		LOG_ERROR("Can't mulit init.");
		return false;
	}
    
	// begin log thread
	_bin_log.Start();
	if (!LoadConfig()) {
        LOG_ERROR("load config file failed.");
        return false;
	}
	
    int listen_port = _config.GetIntValue("listen_port");
    _local_port = _config.GetIntValue("local_port");
    _local_ip = _config.GetStringValue("local_ip");
    _zk_ip_port = _config.GetStringValue("zk_ip_port");

	// set call back
	_net.SetAcceptCallback(std::bind(&CNode::_AcceptCallBack, this, std::placeholders::_1, std::placeholders::_2));
	_net.SetReadCallback(std::bind(&CNode::_ReadCallBack, this, std::placeholders::_1, std::placeholders::_2));
	_net.SetWriteCallback(std::bind(&CNode::_WriteCallBack, this, std::placeholders::_1, std::placeholders::_2));
    _net.SetConnectionCallback(std::bind(&CNode::_ConnectCallBack, this, std::placeholders::_1, std::placeholders::_2));

	// start net with 2 thread
	_net.Init(2);
	_net.ListenAndAccept(_local_port, _local_ip);

	_heart.SetHeartCallBack(std::bind(&CNode::_HeartCallBack, this));
	_heart.SetTimeOutCallBack(std::bind(&CNode::_TimeOutCallBack, this));

	std::string local_ip_port_str = _local_ip + ":" + std::to_string(_local_port);
	bool ret = true;
	// connect zk
	ret = CZkNodeInfo::Instance().Init(local_ip_port_str, _zk_ip_port);
	if (ret == false) {
		LOG_ERROR("Init the zk connect failed.");
		return false;
	}

	// get node info list
	const std::vector<NodeInfo> node_info_list = CZkNodeInfo::Instance().GetNodeList();

	// connect all node
	{
		std::string ip_port;
		std::unique_lock<std::mutex> lock(_socket_mutex);
		for (auto iter = node_info_list.begin(); iter != node_info_list.end(); ++iter) {
			auto soket = _net.Connection(iter->_port, iter->_ip);
			ip_port = iter->_ip + ":" + std::to_string(iter->_port);
			if (soket) {
				_socket_map[ip_port] = soket;

			} else {
				LOG_ERROR("connect node failed. ip:%s, port:%d", iter->_ip.c_str(), iter->_port);
				return false;
			}
		}
	}

	// create node on zk
	CZkNodeInfo::Instance().RegisterNode();

	// start timer
	int step = _config.GetIntValue("timer_step");
	int heart_time = _config.GetIntValue("heart_time");
	int time_out = _config.GetIntValue("time_out");
	LOG_INFO("Init heart timer. timer_step:%d, heart_time:%d, time_out:%d", step, heart_time, time_out);

	_heart.Init(step, heart_time, time_out);

	// start client listen
	_client_listener = new CListener(this);
	_client_listener->Init(_local_ip, listen_port);

	return true;
}

void CNode::Join() {
	_net.Join();
}

void CNode::StopNet() {
    _net.Dealloc();
    _net.Join();
}

bool CNode::LoadConfig() {
	return _config.LoadFile(_config_path);
}

void CNode::SendAllHeart() {
	Msg msg;
	msg._head._type = Heart;
	msg._head._newest_version = _bin_log.GetNewestTime();
	
    if (_done_msg) {
        msg._head._type |= DoneMsg;
        _done_msg = false;
    }

	{
		std::unique_lock<std::mutex> lock(_msg_mutex);
		msg._msg = std::move(_cur_msg);
		_cur_msg.clear();
	}

	std::unique_lock<std::mutex> lock(_socket_mutex);
	msg._head._follower_num = _socket_map.size();
	// make msg string
	std::string msg_str = CParser::Encode(msg);
	for (auto iter = _socket_map.begin(); iter != _socket_map.end(); ++iter) {
		iter->second->SyncWrite(msg_str.c_str(), msg_str.length());
	}
}

void CNode::SendAllVote() {
	Msg msg;
	msg._head._type = Campaign;
	std::unique_lock<std::mutex> lock(_socket_mutex);
	// make msg string
	std::string msg_str = CParser::Encode(msg);
	for (auto iter = _socket_map.begin(); iter != _socket_map.end(); ++iter) {
		iter->second->SyncWrite(msg_str.c_str(), msg_str.length());
	}
}

bool CNode::IsLeader() {
	return _role == Leader;
}

std::string CNode::GetLeaderInfo() {
	return _leader_ip_port;
}

void CNode::SendMsg(const std::string& ip_port, Msg& msg) {
	// make msg string
	std::string msg_str = CParser::Encode(msg);

	std::unique_lock<std::mutex> lock(_socket_mutex);
	auto iter = _socket_map.find(ip_port);
	if (iter != _socket_map.end()) {
		iter->second->SyncWrite(msg_str.c_str(), msg_str.length());

    } else {
        LOG_ERROR("can't find the socket.");
    }
}

void CNode::HandleMsg(const std::string ip_port, const Msg* msg) {
	if (!msg){
		LOG_ERROR("get a invalid msg.");
		return;
	}
    if (msg->_head._type & DoneMsg) {
        HandleDoneMsg(ip_port, *msg);
    }

    if (msg->_head._type & Heart) {
        HandleHeart(ip_port, *msg);
    
    } else if (msg->_head._type & ReHeart) {
        HandleReHeart(ip_port, *msg);

    } else if (msg->_head._type & Campaign) {
        HandleCampaign(ip_port, *msg);

    } else if (msg->_head._type & Vote) {
        HandleVote(ip_port, *msg);

    } else if (msg->_head._type & Sync) {
        HandleSync(ip_port, *msg);

    } else if (msg->_head._type & ToSync) {
        HandleToSync(ip_port, *msg);

    } else {
        LOG_ERROR("get a unknow type msg.");
    }

	delete msg;
}

void CNode::HandleHeart(const std::string& ip_port, const Msg& msg) {
	if (_role != Follower) {
		_role = Follower;
		_leader_ip_port = ip_port;
		LOG_INFO("Leader change to %s", ip_port.c_str());
	}
	// reset vote num
	_vote_count = 0;
	_heart.ResetTimer();

	// cur node lost message. sync from loeader
	if (msg._head._newest_version != _bin_log.GetNewestTime() && _bin_log.GetNewestTime() != 0) {
		Time version = 0;
		if (msg._head._num_msg > 0) {
			BinLog bin_log =_bin_log.StrToBinLog(msg._msg[msg._msg.size()-1]);	
			version = bin_log.first;
		
		} else {
			version = msg._head._newest_version;
		}
		Msg re_msg;
		re_msg._head._type = Sync;
		re_msg._head._newest_version = version;
		SendMsg(ip_port, re_msg);
		LOG_INFO("send a sync msg to leader : %s", ip_port.c_str());
		return;
	}

	// get messsage
	if (msg._head._num_msg > 0) {
		std::unique_lock<std::mutex> lock(_vec_mutex);
		for (size_t i = 0; i < msg._msg.size(); i++) {
			LOG_INFO("recv a msg from client : %s", msg._msg[i].c_str());
			_msg_vec.push_back(std::move(msg._msg[i]));
		}
	}

	// send response
	BinLog bin_log = _bin_log.StrToBinLog(_msg_vec[_msg_vec.size() - 1]);
	Time version = bin_log.first;
	Msg re_msg;
	re_msg._head._type = ReHeart;
	re_msg._head._newest_version = version;
	SendMsg(ip_port, re_msg);
}

void CNode::HandleReHeart(const std::string& ip_port, const Msg& msg) {
	if (_role == Leader) {
		_msg_re_count++;
		std::unique_lock<std::mutex> lock(_socket_mutex);
		// get more than half response
		if (_msg_re_count > _socket_map.size() / 2) {
            _done_msg = true;
			_msg_re_count = 0;

			_client_listener->SendMsg(msg._head._newest_version, RAFT_OK);
		}
	}
	//not a leader do nothing
}

void CNode::HandleCampaign(const std::string& ip_port, const Msg& msg) {
	Msg re_msg;
	if (_role == Leader) {
		re_msg._head._type = ToSync;
	
	} else if (_role == Follower) {
		re_msg._head._type = Vote;
		_heart.ResetTimer();

	} else if (_role == Candidate) {
		// do nothing
	}
	LOG_INFO("recv a Campaign msg from ip : %s, role : %d", ip_port.c_str(), _role);
	SendMsg(ip_port, re_msg);
}

void CNode::HandleVote(const std::string& ip_port, const Msg& msg) {
	if (_role == Candidate) {
		_vote_count++;
		std::unique_lock<std::mutex> lock(_socket_mutex);
		// can be a leader
		if (_vote_count >= _socket_map.size()) {
			_role = Leader;
			SendAllHeart();
		}
	}
	// not a Candidater do nothing
    LOG_INFO("recv a vote msg from ip : %s, size : %d", ip_port.c_str(), _vote_count.load());
}

void CNode::HandleSync(const std::string& ip_port, const Msg& msg) {
	if (_role == Leader) {
		Msg msg;
        msg._head._type = Heart;
		msg._head._newest_version = msg._head._newest_version;

		std::vector<BinLog> log_vec;
		_bin_log.GetLog(msg._head._newest_version, log_vec);
		for (size_t i = 0; i < log_vec.size(); i++) {
			msg._msg.push_back(std::move(_bin_log.BinLogToStr(log_vec[i])));
		}
		{
			std::unique_lock<std::mutex> lock(_socket_mutex);
			msg._head._follower_num = _socket_map.size();
		}
		SendMsg(ip_port, msg);
	}
	LOG_INFO("recv a SYNC msg from ip : %s, role : %d", ip_port.c_str(), _role);
	//not a leader do nothing
}

void CNode::HandleToSync(const std::string& ip_port, const Msg& msg) {
	if (_role != Follower) {
		_role = Follower;
	}

	// reset vote num
	_vote_count = 0;
	_heart.ResetTimer();

	Msg re_msg;
	re_msg._head._type = Sync;
	re_msg._head._newest_version = _bin_log.GetNewestTime();
	SendMsg(ip_port, re_msg);
}

void CNode::HandleDoneMsg(const std::string& ip_port, const Msg& msg) {
	if (_role == Follower) {
		std::vector<std::string> vec;
		{
			std::unique_lock<std::mutex> lock(_vec_mutex);
			vec = std::move(_msg_vec);
		}
		BinLog log;
		for (size_t i = 0; i < vec.size(); i++) {
			log = _bin_log.StrToBinLog(vec[i]);
			_bin_log.PushLog(log.first, std::move(log.second));
		}
	}
}

void CNode::HandleClient(const BinLog& log) {
	if (_role == Leader) {
        std::string log_str = _bin_log.BinLogToStr(log);
		std::unique_lock<std::mutex> lock(_msg_mutex);
		_cur_msg.push_back(std::move(log_str));
	}
}

// net io
void CNode::_ReadCallBack(CMemSharePtr<CSocket>& socket, int err) {
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
	if (len < header_len) {
		socket->SyncRead();
		return;
	}

	char buf[4096] = {0};
	// read header
	len = socket->_read_event->_buffer->ReadNotClear(buf, header_len);
	if (len < header_len) {
		socket->SyncRead();
		return;
	}
	// can make a msg pool.
	Msg *msg = new Msg;
	*msg = CParser::Decode(std::string(buf, len));
	int body_len = msg->_head._body_len;
	if (body_len > 0) {
		len = socket->_read_event->_buffer->GetCanReadSize();
		// not recv a complete msg
		if (len < body_len + header_len) {
			socket->SyncRead();
			delete msg;
			return;

		} else {
			socket->_read_event->_buffer->Read(buf, body_len + header_len);
			*msg = CParser::Decode(buf);
		}

	} else {
		// clean the buffer
		socket->_read_event->_buffer->Read(buf, header_len);
	}
	
	LOG_INFO("get a msg from %s", ip.c_str());
	HandleMsg(ip, msg);
    socket->SyncRead();
}

void CNode::_WriteCallBack(CMemSharePtr<CSocket>& socket, int err) {
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

void CNode::_AcceptCallBack(CMemSharePtr<CSocket>& socket, int err) {
	std::string ip = socket->GetAddress();
	short port = socket->GetPort();
	ip.append(":");
	ip.append(std::to_string(port));

	_socket_map[ip] = socket;
	socket->SyncRead();
	LOG_INFO("recv a connect from ip  %s", ip.c_str());
}

void CNode::_ConnectCallBack(CMemSharePtr<CSocket>& socket, int err) {
    std::string ip = socket->GetAddress();
    short port = socket->GetPort();
    ip.append(":");
    ip.append(std::to_string(port));

    LOG_INFO("connect to ip  %s", ip.c_str());
}

// timer
void CNode::_HeartCallBack() {
	if (_role == Leader) {
		_heart.ResetTimer();
		SendAllHeart();
		LoadConfig();
		LOG_INFO("heart call back.");
	}
}

void CNode::_TimeOutCallBack() {
	if (_role != Leader) {
        _role = Candidate;
        
        // vote self
        _vote_count = 1;
        SendAllVote();
		LOG_INFO("to be a candidate.");
	} 
    LOG_INFO("time out call back.");
}