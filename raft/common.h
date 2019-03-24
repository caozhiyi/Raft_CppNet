#ifndef HEADER_COMMON
#define HEADER_COMMON

#include <string>

//�ڵ�״̬
enum Status {
	Follower  = 1,	
	Candidate = 2,
	Leader    = 3
};

//��Ϣ����
enum MsgType {
	Heart	 = 1,
	Campaign = 2,	// ����leader
	Vote	 = 3	// ͶƱ
};

//�ڵ���Ϣ
struct NodeInfo {
	std::string _ip;
};

struct MsgHead {
	int	_type;
	int _follower_num;	// ����������
	int _length;
};

struct Msg {
	MsgHead _head;
	std::string _msg;
};

struct NodeInfo {
	std::string _ip;
};

#endif