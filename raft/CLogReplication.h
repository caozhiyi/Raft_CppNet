#ifndef HEADER_LOG_REPLICATION
#define HEADER_LOG_REPLICATION

#include <string>
#include <vector>
#include <fstream>

#include "base/RunnableAloneTaskList.h"
#include "base/Single.h"

typedef unsigned long long Time;
typedef std::pair<Time, std::string> BinLog;

class CBinLog: public CRunnableAloneTaskList<BinLog>, public CSingle<BinLog> {
public:
    CBinLog(std::string fine_name = "raft_bin_log.log");
    ~CBinLog();

    //thread
    void Run();
	void Stop();

    //submit log with time
    void PushLog(Time time, std::string log);
    //get log vector after time
    bool GetLog(Time time, std::vector<BinLog>& log_vec);
    //get the newest time
    Time GetNewestTime();

private:
	//get the newer log
	std::vector<std::string> GetTargetLogStr(int count);
	//parser log str to BinLog
	BinLog StrToBinLog(const std::string& log);
	//parser BinLog to log str
	std::string BinLogToStr(const BinLog& log);

private:
    std::string        _file_name;
    std::fstream       _file_stream;
    std::atomic<bool>  _stop;
    Time               _newest_time;
	std::mutex		   _mutex;
};

#endif