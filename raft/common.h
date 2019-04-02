#ifndef HEADER_COMMON
#define HEADER_COMMON

#include <string>
#include <vector>

// node role
enum NodeRole {
	Follower  = 1,	
	Candidate = 2,
	Leader    = 3
};

// message type
enum MsgType {
	Heart	 = 1,   // heart 
	ReHeart  = 2,	// heart response
	DoneMsg  = 3,	// client receive and push the message to file
	Campaign = 4,	// campaign to be leader
	Vote	 = 5,	// vote 
	Sync	 = 6,	// sync the message from  leader
	ToSync   = 7    // follower get it to sync from leader
};

// node info
struct NodeInfo {
	std::string _ip;
	int			_port;
};

struct MsgHead {
	int	_type;
	int _follower_num;	// the leader's num of follower
	int _body_len;		// message body len
	int _num_msg;		// the body's num of message
	long long _newest_version;

	MsgHead() {
		_follower_num = 0;
		_body_len = 0;
		_num_msg = 0;
		_newest_version = 0;
	}

	~MsgHead() {}
};

const int header_len = sizeof(MsgHead);

struct Msg {
	MsgHead _head;
	std::vector<std::string> _msg;

	Msg() {}
	~Msg() {}
};

enum STATUS {
	RAFT_OK = 0,
    RAFT_FAILED = 1,
	RAFT_RELEADER = 2
};

struct ClientMsgHead {
	int _status;
	int _body_len;
};

const int client_header_len = sizeof(ClientMsgHead);

struct ClientMsg {
	ClientMsgHead _head;
	std::string   _msg;
};

#endif