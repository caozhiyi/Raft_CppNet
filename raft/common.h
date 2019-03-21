#ifndef HEADER_COMMON
#define HEADER_COMMON

#include <string>

//节点状态
enum Status {
	Follower  = 1,	
	Candidate = 2,
	Leader    = 3
};

//消息类型
enum MsgType {
	Heart	 = 1,
	Client	 = 2,
	Campaign = 3,	// 竞争leader
	Vote	 = 4	// 投票
};

struct MsgHead {
	int	type;
	int follower_num;	// 跟随者数量
	int length;
};

struct Msg {
	MsgHead head;
	std::string msg;
};

#endif