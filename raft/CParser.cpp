#include <string.h>
#include "CParser.h"

const char* SPILT = "%R%N";

CParser::CParser() {

}

CParser::~CParser() {

}

std::string CParser::Encode(Msg& msg) {
	char head[header_len + 1] = { 0 };
	memcpy(head, &msg._head, header_len);
	std::string body;
	for (int i = 0; i < msg._msg.size(); i++) {
		body.append(msg._msg[i]);
		body.append(SPILT);
	}
	msg._head._body_len = body.size();
	msg._head._num_msg = msg._msg.size();

	std::string ret(head);
	ret.append(body);
	return std::move(ret);
}

Msg CParser::Decode(const std::string& msg_str, bool only_header) {
	Msg msg;
	memcpy(&msg._head, msg_str.c_str(), header_len);
	msg_str = msg_str.substr(header_len + 1);
	if (!only_header) {
		int pos = 0;
		std::string one_msg;
		for (int i = 0; i < msg._head._num_msg; i++) {
			pos = msg_str.find_first_of(SPILT);
			one_msg = msg_str.substr(0, pos);
			msg_str = msg_str.substr(pos + sizeof(SPILT));
			msg._msg.push_back(one_msg);
		}
	}
	return std::move(msg);
}