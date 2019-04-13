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

	std::string ret(head, header_len);
	ret.append(body);
	return std::move(ret);
}

Msg CParser::Decode(const std::string& msg_str, bool only_header) {
	Msg msg;
	memcpy(&msg._head, msg_str.c_str(), header_len);
	if (!only_header) {
        std::string str = msg_str.substr(header_len);
		int pos = 0;
		std::string one_msg;
		for (int i = 0; i < msg._head._num_msg; i++) {
			pos = str.find_first_of(SPILT);
			one_msg = str.substr(0, pos);
            str = str.substr(pos + sizeof(SPILT));
			msg._msg.push_back(one_msg);
		}
	}
	return std::move(msg);
}

std::string CParser::Encode(ClientMsg& msg) {
	char head[client_header_len + 1] = { 0 };
	memcpy(head, &msg._head, client_header_len);

	msg._head._body_len = msg._msg.length();
	std::string ret(head, client_header_len);
	ret.append(msg._msg);
	return std::move(ret);
}

ClientMsg CParser::DecodeClient(const std::string& msg_str, bool only_header) {
	ClientMsg msg;
	memcpy(&msg._head, msg_str.c_str(), client_header_len);
    if (!only_header) {
        msg_str = msg_str.substr(client_header_len + 1);
        msg._msg = msg_str;
    }
	return std::move(msg);
}