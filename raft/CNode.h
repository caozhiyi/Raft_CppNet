#ifndef HEADER_CNODE
#define HEADER_CNODE

#include <mutex>

#include "CHeart.h"
#include "CLogReplication.h"
#include "command.h"
#include "NetObject.h"
#include "config.h"

class CNode
{
public:
	CNode(const std::string& config_path);
	~CNode();

	bool Init();

	void LoadConfig();

	void SendAllHreart();

	void HandleHeart(const Msg& msg);
	void HandleReHeart(const Msg& msg);
	void HandleCampaign(const Msg& msg);
	void HandleVote(const Msg& msg);
	void HandleSync(const Msg& msg);

	void HandleClient(const Msg& msg);

private:
	// net io
	void _ReadCallBack(CMemSharePtr<CSocket>& socket, int err);
	void _WriteCallBack(CMemSharePtr<CSocket>& socket, int err);
	void _AcceptCallBack(CMemSharePtr<CSocket>& socket, int err);

	// timer
	void _HeartCallBack();
	void _TimeOutCallBack();

private:
	NodeStatus	_status;
	CNetObject	_net;
	NodeInfo	_leader_info;
	std::string _config_path;
	CConfig		_config;
	CHeart		_heart;

	std::string _zk_ip_port;
	std::string _local_ip;
	int			_local_port;

	std::mutex _msg_mutex;
	std::vector<std::string>	_cur_msg;

	std::mutex _socket_mutex;
	std::vector<CMemSharePtr<CSocket>>	_socket_list;

	std::vector<std::string>	_msg_vec;	// all client msg
};

#endif