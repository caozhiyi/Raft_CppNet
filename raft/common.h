#ifndef HEADER_COMMON
#define HEADER_COMMON

#include <string>
#include <vector>

//节点状态
enum NodeRole {
	Follower  = 1,	
	Candidate = 2,
	Leader    = 3
};

//消息类型
enum MsgType {
	Heart	 = 1,	// 心跳
	ReHeart  = 2,	// 心跳回复
	DoneMsg  = 3,	// 确定将消息落库
	Campaign = 4,	// 竞争leader
	Vote	 = 5,	// 投票
	Sync	 = 6	// 同步

};

//节点信息
struct NodeInfo {
	std::string _ip;
	int			_port;
};

struct MsgHead {
	int	_type;
	int _follower_num = 0;	// 跟随者数量
	int _body_len = 0;		// 消息体长度
	int _num_msg = 0;		// 携带消息数量
	long long _msg_version = 0;	// 消息版本 递增
};

const int header_len = sizeof(MsgHead);

struct Msg {
	MsgHead _head;
	std::vector<std::string> _msg;
};

struct NodeInfo {
	std::string _ip;
};

#endif