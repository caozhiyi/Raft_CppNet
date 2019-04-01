#ifndef HEADER_CNODE
#define HEADER_CNODE

#include <mutex>
#include <queue>
#include <functional>

#include "common.h"
#include "NetObject.h"
#include "config.h"

typedef long long Time;
typedef std::function<void(int)> _send_call_back;

class CListener
{
public:
	CListener(CNode* cur_node);
	~CListener();

	bool Init(std::string ip, int port);

	void SendMsg(Time version, int status);

private:
	// net io
	void _ReadCallBack(CMemSharePtr<CSocket>& socket, int err);
	void _WriteCallBack(CMemSharePtr<CSocket>& socket, int err);
	void _AcceptCallBack(CMemSharePtr<CSocket>& socket, int err);


private:
	std::string _ip;
	short       _port;
	CNode*      _cur_node;

	std::mutex _send_mutex;
	std::queue<std::pair<Time, _send_call_back>>    _send_queue;

	std::mutex _socket_mutex;
	std::map<std::string, CMemSharePtr<CSocket>>	_socket_map;
};

#endif