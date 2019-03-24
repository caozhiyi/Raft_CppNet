#include <iostream>
//#include "zookeeper.h"
//#include "Log.h"
//#include "CZkClient.h"
#include "config.h"
using namespace std;
void TestCConfig() {
	CConfig fig;
	fig.SetFilePath("./raft.conf");
	if (!fig.ReLoadFile()) {
		cout << "load file faild" << endl;
	}
	cout << fig.GetDoubleValue("d") << endl;
	cout << fig.GetIntValue("i") << endl;
	cout << fig.GetStringValue("s") << endl;
	cout << fig.GetBoolValue("b") << endl;
}
int main() {
	TestCConfig();
	/*CLog::Instance().SetLogName("raft.log");
	CLog::Instance().SetLogLevel(LOG_DEBUG_LEVEL);
	CLog::Instance().Start();


	CZkClient::Instance().ConnectZK("127.0.0.1:2181", 2000);
	CZkClient::Instance().CreateNode("/test", "test");

	CLog::Instance().Join();
	CLog::Instance().Stop();*/
}