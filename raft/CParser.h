#ifndef HEADER_CPARSER
#define HEADER_CPARSER

#include "common.h"
// Ω‚Œˆ–≠“È

class CParser{
public:
	CParser();
	~CParser();

	static std::string Encode(Msg& msg);
	static Msg Decode(const std::string& msg_str, bool only_header = true);

	static std::string Encode(ClientMsg& msg);
	static ClientMsg DecodeClient(const std::string& msg_str, bool only_header = true);

private:
    static std::string EncodeHead(const MsgHead& head);
    static std::string EncodeHead(const ClientMsgHead& head);
};

#endif
