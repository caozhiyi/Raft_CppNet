#include <exception>
#include <string.h>         //for memset
#include "CLogReplication.h"

#define FLAG_STR "%R%N"
#define FLAG_LEN 4
#define ONCE_LEN 1024

CBinLog::CBinLog(std::string fine_name) : _file_name(fine_name), _stop(false) {
	_file_stream.open(_file_name, std::ios::in|std::ios::app| std::ios::out);
	if (!_file_stream.good()) {
		throw std::exception(std::logic_error("open log file failed"));
	}
}

CBinLog::~CBinLog() {
	_file_stream.close();
}

void CBinLog::Run() {
    if (!_stop) {
        BinLog log = std::move(_Pop());
		std::string log_str = BinLogToStr(log);

		std::unique_lock<std::mutex> lock(_mutex);
		_file_stream << log_str;
    }
}

void CBinLog::Stop() {
	_stop = true;
}

void CBinLog::PushLog(Time time, std::string log) {
    BinLog bin_log;
	bin_log.first  = time;
	bin_log.second = log;
    Push(std::move(bin_log));
}

bool CBinLog::GetLog(Time time, std::vector<BinLog>& log_vec) {
	std::unique_lock<std::mutex> lock(_mutex);
	
	std::vector<std::string> vec = GetTargetLogStr(20);
	if (vec.empty()) {
		return false;
	}
	for (size_t i = 0; i < vec.size(); i++) {
		BinLog bin_log = StrToBinLog(vec[i]);
		if (bin_log.first < time) {
			break;
		}
		log_vec.push_back(bin_log);
	}
	return true;
}

Time CBinLog::GetNewestTime() {
	std::vector<std::string> vec = GetTargetLogStr(1);
	if (vec.empty()) {
		return 0;
	}
	BinLog bin_log = StrToBinLog(vec[0]);
	return bin_log.first;
}

std::vector<std::string> CBinLog::GetTargetLogStr(int count) {
	size_t file_size = _file_stream.tellg();
	_file_stream.clear();
	if (file_size <= ONCE_LEN){
		_file_stream.seekg(0, std::ios::beg);
	} else {
		_file_stream.seekg(-ONCE_LEN, std::ios::end);
	}
	
	std::vector<std::string> res_vec;
	bool ret = false;
	int pri_pos = 0;
	int find_rand = 1;
	char buf[ONCE_LEN * 2 + 1] = { 0 };
	char pri_buf[ONCE_LEN + 1] = { 0 };
	while (true) {
		find_rand ++;
		memset(buf, 0, ONCE_LEN * 2 + 1);
		_file_stream.read(buf, ONCE_LEN);
		int len = _file_stream.gcount();
		if (len == 0) {
			break;
		}
		if (pri_pos > 0) {
			strncpy(buf + len, pri_buf, pri_pos);
			memset(pri_buf, 0, ONCE_LEN + 1);
			pri_pos = len + pri_pos;
		} else {
			pri_pos = len;
		}
		
		char cur[FLAG_LEN+1] = { 0 };
		for (int i = len - FLAG_LEN; i >= 0; i--) {
			strncpy(cur, buf+i, FLAG_LEN);
			if (strcmp(cur, FLAG_STR) == 0) {
				res_vec.push_back(std::string(buf + i + FLAG_LEN, pri_pos - i - FLAG_LEN));
				pri_pos = i;
				count--;
				if (count == 0) {
					ret = true;
					break;
				}
			}
		}
		if (ret) {
			break;
		}
		if (pri_pos > 0) {
			strncpy(pri_buf, buf, pri_pos);
		}
		if (_file_stream.eof()) {
			_file_stream.clear();
		}
		int pos = -find_rand*ONCE_LEN;
		_file_stream.seekg(pos, std::ios::end);
	}
	return res_vec;
}

BinLog CBinLog::StrToBinLog(const std::string& log) {
	int pos = log.find(':');
	BinLog res;
	if (pos == std::string::npos) {
		return res;
	}
	std::string time_srt = log.substr(0, pos);
	std::string content = log.substr(pos + 1, log.length());

	res.first  = atoll(time_srt.c_str());
	res.second = std::move(content);
	return std::move(res);
}

std::string CBinLog::BinLogToStr(const BinLog& log) {
	std::string res;
	res.append(std::to_string(log.first));
	res.append(":");
	res.append(log.second);
	return std::move(res);
}