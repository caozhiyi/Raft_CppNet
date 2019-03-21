#ifndef HEADER_CTIME
#define HEADER_CTIME

#include <string>
#include <map>
#include <mutex>

const int NOT_FIND = -1;

class CConfig
{
public:
	bool SetFilePath(const std::string& path);
	bool ReLoadFile();

	bool LoadFile(const std::string& path);
	
	int GetIntValue(const std::string& key);
	int GetStringValue(const std::string& key);
	int GetDoubleValue(const std::string& key);

private:
	void _Trim(std::string line);

private:
	std::string _file;
	std::mutex	_mutex;
	std::map<std::string, std::string> _config_map;
};

#endif