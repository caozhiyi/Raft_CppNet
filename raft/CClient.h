#ifndef HEADER_CCLIENT
#define HEADER_CCLIENT

#include <mutex>
#include <queue>
#include <functional>

#include "NetObject.h"

typedef std::function<void(int)> ResponseBack;

class CClient
{
public:
	CClient();
	~CClient();

	bool Init(std::string ip, int port);

	void SendMsg(const std::string& msg_str);
	void SetResponseBack(ResponseBack& call_back);

private:
	// net io
	void _ReadCallBack(CMemSharePtr<CSocket>& socket, int err);
	void _WriteCallBack(CMemSharePtr<CSocket>& socket, int err);

private:
	CNetObject	_net;
	std::string _ip;
	short       _port;
    CMemSharePtr<CSocket>	_socket;
	ResponseBack _call_back;
};

#endif