#include <iostream>
#include "CNode.h"
#include "Log.h"
using namespace std;

int main() {

	CLog::Instance().SetLogName("server.log");
	CLog::Instance().SetLogLevel(LOG_DEBUG_LEVEL);
	CLog::Instance().Start();

	CNode node("server.conf");
	node.Init();
	node.Join();
}