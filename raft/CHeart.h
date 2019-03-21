#ifndef HEADER_CPARSER
#define HEADER_CPARSER

#include "PoolSharedPtr.h"
#include "Socket.h"
#include "NetObject.h"

class CHeart
{
public:
	CHeart();
	~CHeart();

private:
	static void TimerFunc(void* param);

private:
	CNetObject* _net;
	std::vector<std::string> _msg_vec;
	std::vector<CMemSharePtr<CSocket>> _socket_vec;
};

#endif