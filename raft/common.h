#ifndef HEADER_COMMON
#define HEADER_COMMON

#include <string>
#include <vector>

//�ڵ�״̬
enum NodeStatus {
	Follower  = 1,	
	Candidate = 2,
	Leader    = 3
};

//��Ϣ����
enum MsgType {
	Heart	 = 1,	// ����
	ReHeart  = 2,	// �����ظ�
	Campaign = 3,	// ����leader
	Vote	 = 4,	// ͶƱ
	Sync	 = 5	// ͬ��
};

//�ڵ���Ϣ
struct NodeInfo {
	std::string _ip;
	int			_port;
};

const int header_len = 16;

struct MsgHead {
	int	_type;
	int _follower_num;	// ����������
	int _body_len;		// ��Ϣ�峤��
	int _num_msg;		// Я����Ϣ����
};

struct Msg {
	MsgHead _head;
	std::vector<std::string> _msg;
};

struct NodeInfo {
	std::string _ip;
};

#endif