#ifndef HEADER_COMMON
#define HEADER_COMMON

#include <string>
#include <vector>

//�ڵ�״̬
enum NodeRole {
	Follower  = 1,	
	Candidate = 2,
	Leader    = 3
};

//��Ϣ����
enum MsgType {
	Heart	 = 1,	// ����
	ReHeart  = 2,	// �����ظ�
	DoneMsg  = 3,	// ȷ������Ϣ���
	Campaign = 4,	// ����leader
	Vote	 = 5,	// ͶƱ
	Sync	 = 6	// ͬ��

};

//�ڵ���Ϣ
struct NodeInfo {
	std::string _ip;
	int			_port;
};

struct MsgHead {
	int	_type;
	int _follower_num = 0;	// ����������
	int _body_len = 0;		// ��Ϣ�峤��
	int _num_msg = 0;		// Я����Ϣ����
	long long _msg_version = 0;	// ��Ϣ�汾 ����
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