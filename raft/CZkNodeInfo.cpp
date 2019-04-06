#include <map>
#include "CZkNodeInfo.h"
#include "CZkClient.h"
#include "common.h"
#include "Log.h"

const char* ZK_NODE = "/node_info";

CZkNodeInfo::CZkNodeInfo() {

}

CZkNodeInfo::~CZkNodeInfo() {

}

bool CZkNodeInfo::Init(const std::string& local_ip_port, const std::string& zk_ip_port) {
	_local_ip_port	= local_ip_port;
	_zk_ip_port = zk_ip_port;

	bool ret = CZkClient::Instance().ConnectZK(_zk_ip_port, 100000);
    if (ret == false) {
        return false;
    }
	_GetNodeList();
	return ret;
}

bool CZkNodeInfo::RegisterNode() {
	std::string path = ZK_NODE;
    if (!CZkClient::Instance().NodeExista(path)) {
        
        if (!CZkClient::Instance().CreateNode(path, "")) {
            return false;
        }
    }
    path.append("/");
	path.append(_local_ip_port);
	return CZkClient::Instance().CreateNode(path, _local_ip_port, true);
}

const std::vector<NodeInfo>& CZkNodeInfo::GetNodeList() {
	return _node_list;
}

int CZkNodeInfo::GetNodeNum() {
	return (int)_node_list.size();
}

void CZkNodeInfo::_GetNodeList() {
	std::map<std::string, std::string> tmp_map;
	CZkClient::Instance().GetAllChildren(ZK_NODE, tmp_map, true);

	std::unique_lock<std::mutex> lock(_mutex);
	_node_list.clear();
	NodeInfo info;
	std::string ip_port;
	int pos = 0;
	for (auto iter = tmp_map.begin(); iter != tmp_map.end(); ++iter) {
		ip_port = iter->second;
		pos = ip_port.find_first_of(":");
		info._ip = ip_port.substr(0, pos);
		info._port = atoi(ip_port.substr(pos + 1).c_str());
		_node_list.push_back(info);
	}
}

static void CZkNodeInfo::_CallBack(int type, std::string path) {
	if (path == ZK_NODE && type == CHILD_EVENT_DEF) {
		CZkNodeInfo::Instance()._GetNodeList();
		LOG_INFO("node info list changed.");
	}
}