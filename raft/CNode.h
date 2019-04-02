#ifndef HEADER_CNODE
#define HEADER_CNODE

#include <mutex>

#include "CHeart.h"
#include "CLogReplication.h"
#include "common.h"
#include "NetObject.h"
#include "config.h"
#include "CListener.h"

class CNode
{
public:
	CNode(const std::string& config_path);
	~CNode();

	bool Init();
	void Join();

	void LoadConfig();

	// only leader send heart
	void SendAllHeart();
	// time out. to Candidate
	void SendAllVote();

	bool IsLeader();
	std::string GetLeaderInfo();

	void SendMsg(const std::string& ip_port, Msg& msg);

	void HandleMsg(const std::string ip_port, const Msg* msg);
	void HandleHeart(const std::string& ip_port, const Msg& msg);
	void HandleReHeart(const std::string& ip_port, const Msg& msg);
	void HandleCampaign(const std::string& ip_port, const Msg& msg);
	void HandleVote(const std::string& ip_port, const Msg& msg);
	void HandleSync(const std::string& ip_port, const Msg& msg);
	void HandleToSync(const std::string& ip_port, const Msg& msg);
	void HandleDoneMsg(const std::string& ip_port, const Msg& msg);

	void HandleClient(const BinLog& log);

private:
	// net io
	void _ReadCallBack(CMemSharePtr<CSocket>& socket, int err);
	void _WriteCallBack(CMemSharePtr<CSocket>& socket, int err);
	void _AcceptCallBack(CMemSharePtr<CSocket>& socket, int err);


	// timer
	void _HeartCallBack();
	void _TimeOutCallBack();

private:
	NodeRole	_role;
	CNetObject	_net;
	NodeInfo	_leader_info;
	std::string _config_path;
	CConfig		_config;
	CHeart		_heart;
	std::atomic_int _msg_re_count;	// reheart' num
	std::atomic_int _vote_count;	// vote's num
	CBinLog     _bin_log;
	std::string _leader_ip_port;

	std::string _zk_ip_port;
	std::string _local_ip;
	int			_local_port;

	CListener*  _client_listener;

	std::mutex _msg_mutex;
	std::vector<std::string>	_cur_msg;

	std::mutex _socket_mutex;
	std::map<std::string, CMemSharePtr<CSocket>>	_socket_map;

	std::mutex _vec_mutex;
	std::vector<std::string>	_msg_vec;	// all recv client msg
};

#endif