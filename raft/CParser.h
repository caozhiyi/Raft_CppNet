#ifndef HEADER_CPARSER
#define HEADER_CPARSER

#include "common.h"
// ����Э��

class CParser
{
public:
	CParser();
	~CParser();

	static std::string Encode(const Msg& msg);
	static Msg Decode(const std::string& msg_str, bool only_header = false);
};

#endif
