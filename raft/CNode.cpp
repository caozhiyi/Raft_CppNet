#include "CNode.h"
#include "CZkNodeInfo.h"
#include "Log.h"

CNode::CNode(const std::string& config_path) :
	_status(Follower), 
	_config_path(config_path) {

}


CNode::~CNode() {

}

bool CNode::Init() {
	LoadConfig();
	_local_port = _config.GetIntValue("local_port");
	_local_ip = _config.GetStringValue("local_ip");
	_zk_ip_port = _config.GetStringValue("zk_ip_port");

	// set call back
	_net.SetAcceptCallback(std::bind(&CNode::_AcceptCallBack, this, std::placeholders::_1, std::placeholders::_2));
	_net.SetReadCallback(std::bind(&CNode::_ReadCallBack, this, std::placeholders::_1, std::placeholders::_2));
	_net.SetWriteCallback(std::bind(&CNode::_WriteCallBack, this, std::placeholders::_1, std::placeholders::_2));

	_heart.SetHeartCallBack(std::bind(&CNode::_HeartCallBack, this));
	_heart.SetTimeOutCallBack(std::bind(&CNode::_TimeOutCallBack, this));

	std::string local_ip_port_str = _local_ip + std::to_string(_local_port);
	bool ret = true;
	// connect zk
	ret = CZkNodeInfo::Instance()->Init(local_ip_port_str, _zk_ip_port);
	if (ret == false) {
		LOG_ERROR("connect zk server failed.");
		return false;
	}

	// get node info list
	const std::vector<NodeInfo> node_info_list = CZkNodeInfo::Instance()->GetNodeList();

	// connect all node
	{
		std::unique_lock<std::mutex> lock(_socket_mutex);
		for (auto iter = node_info_list.begin(); iter != node_info_list.end(); ++iter) {
			auto soket = _net.Connection(iter->_port, iter->_ip);
			if (soket) {
				_socket_list.push_back(soket);

			} else {
				LOG_ERROR("connect node failed. ip:%s, port:%d", iter->_ip.c_str(), iter->_port);
				return false;
			}
		}
	}

	// create node on zk
	CZkNodeInfo::Instance()->RegisterNode();

	// start timer
	int step = _config.GetIntValue("timer_step");
	int heart_time = _config.GetIntValue("heart_time");
	int time_out = _config.GetIntValue("time_out");
	LOG_INFO("Init heart timer. timer_step:%d, heart_time:%d, time_out:%d", step, heart_time, time_out);

	_heart.Init(step, heart_time, time_out);

	return true;
}

void CNode::LoadConfig() {
	_config.LoadFile(_config_path);
}

void CNode::SendAllHreart() {
	Msg msg;
	msg._head._type = Heart;
	
	{
		std::unique_lock<std::mutex> lock(_msg_mutex);
		msg._msg = _cur_msg;
		_cur_msg.clear();
	}

	std::unique_lock<std::mutex> lock(_socket_mutex);
	msg._head._follower_num = _socket_list.size();
	std::string msg_str = CParser::Encode(msg);
	for (auto iter = _socket_list.begin(); iter != _socket_list.end(); ++iter) {
		iter->SyncWrite(msg_str.c_str(), msg_str.length());
	}
}

void CNode::HandleHeart(const Msg& msg) {

}

void CNode::HandleReHeart(const Msg& msg) {

}

void CNode::HandleCampaign(const Msg& msg) {

}

void CNode::HandleVote(const Msg& msg) {

}

void CNode::HandleSync(const Msg& msg) {

}

void CNode::HandleClient(const Msg& msg) {

}

// net io
void CNode::_ReadCallBack(CMemSharePtr<CSocket>& socket, int err) {

}

void CNode::_WriteCallBack(CMemSharePtr<CSocket>& socket, int err) {

}

void CNode::_AcceptCallBack(CMemSharePtr<CSocket>& socket, int err) {

}

// timer
void CNode::_HeartCallBack() {

}

void CNode::_TimeOutCallBack() {

}
