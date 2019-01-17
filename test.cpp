#include <iostream>
#include "zookeeper.h"
#include "Log.h"
#include "CZkClient.h"
using namespace std;
int main() {
    CLog::Instance().SetLogName("raft.log");
    CLog::Instance().SetLogLevel(LOG_DEBUG_LEVEL);
    CLog::Instance().Start();


    CZkClient::Instance().ConnectZK("127.0.0.1:2181", 2000);
    CZkClient::Instance().CreateNode("/test", "test");

    CLog::Instance().Join();
    CLog::Instance().Stop();

}