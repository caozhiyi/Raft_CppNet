#ifndef HEADER_CLISTENER
#define HEADER_CLISTENER

#include <mutex>
#include <queue>
#include <functional>

#include "NetObject.h"
#include "common.h"

typedef std::function<void(int)> SendCallBack;
typedef unsigned long long Time;

class CListener
{
public:
    friend class CNode;
	CListener(CNode* cur_node);
	~CListener();

	bool Init(std::string ip, int port);
	void SendMsg(Time version, int status);
	void SendMsg(const std::string& ip_port, ClientMsg& msg);

private:
	// net io
	void _ReadCallBack(CMemSharePtr<CSocket>& socket, int err);
	void _WriteCallBack(CMemSharePtr<CSocket>& socket, int err);
	void _AcceptCallBack(CMemSharePtr<CSocket>& socket, int err);


private:
	CNetObject	_net;
	std::string _ip;
	short       _port;
	CNode*      _cur_node;

	std::mutex _send_mutex;
	std::queue<std::pair<Time, SendCallBack>>    _send_queue;

	std::mutex _socket_mutex;
	std::map<std::string, CMemSharePtr<CSocket>> _socket_map;
};

#endif