#include <iostream>
#include <functional>
#include "CClient.h"
#include "config.h"
#include "Log.h"
#include "LinuxFunc.h"
using namespace std;
void ResponseFunc(int status) {
	cout << "status" << status << endl;
}

int main() {
    SetCoreFileUnlimit();

	CLog::Instance().SetLogName("server.log");
	CLog::Instance().SetLogLevel(LOG_DEBUG_LEVEL);
	CLog::Instance().Start();

	CConfig fig;
	fig.LoadFile("conf/client.conf");

	string ser_ip = fig.GetStringValue("ip");
	int port = fig.GetIntValue("port");

	CClient client;
	client.SetResponseBack(ResponseFunc);
	client.Init(ser_ip, port);

	while (1) {
		string msg;
		cin >> msg;
		client.SendMsg(msg);
	}
}