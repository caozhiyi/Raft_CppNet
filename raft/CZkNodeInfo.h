#ifndef HEADER_ZK_NODE_INFO
#define HEADER_ZK_NODE_INFO

#include <vector>
#include <mutex>
#include "common.h"
#include "CZkClient.h"
#include "Single.h"

class CZkNodeInfo : public CSingle<CZkNodeInfo> {
public:
	CZkNodeInfo();
	~CZkNodeInfo();

	// connect to zk server
	bool Init(const std::string& local_ip_port, const std::string& zk_ip_port);

	// register local info to zk
	bool RegisterNode();

	// get all node info
	const std::vector<NodeInfo>& GetNodeList();
	int GetNodeNum();

private:
	void _GetNodeList();
	static void _CallBack(int type, std::string path);

private:
	std::string _local_ip_port;
	std::string	_zk_ip_port;
	std::mutex	_mutex;
	std::vector<NodeInfo> _node_list;
};

#endif