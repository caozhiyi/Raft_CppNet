#ifndef HEADER_COMMON
#define HEADER_COMMON

#include <string>
#include <vector>

//节点状态
enum NodeStatus {
	Follower  = 1,	
	Candidate = 2,
	Leader    = 3
};

//消息类型
enum MsgType {
	Heart	 = 1,	// 心跳
	ReHeart  = 2,	// 心跳回复
	Campaign = 3,	// 竞争leader
	Vote	 = 4,	// 投票
	Sync	 = 5	// 同步
};

//节点信息
struct NodeInfo {
	std::string _ip;
	int			_port;
};

const int header_len = 16;

struct MsgHead {
	int	_type;
	int _follower_num;	// 跟随者数量
	int _body_len;		// 消息体长度
	int _num_msg;		// 携带消息数量
};

struct Msg {
	MsgHead _head;
	std::vector<std::string> _msg;
};

struct NodeInfo {
	std::string _ip;
};

#endif