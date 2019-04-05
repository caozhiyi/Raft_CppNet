#include <iostream>
#include "CNode.h"
#include "Log.h"
#include "LinuxFunc.h"

using namespace std;

int main() {

    SetCoreFileUnlimit();

	CLog::Instance().SetLogName("server.log");
	CLog::Instance().SetLogLevel(LOG_DEBUG_LEVEL);
	CLog::Instance().Start();

	CNode node("conf/server.conf");
	if (!node.Init()) {
        node.StopNet();
        return -1;
	}
	node.Join();
    node.StopNet();

    return 0;
}