// 패킷 정보
#define SC_POS           1
#define SC_PUT_PLAYER    2
#define SC_REMOVE_PLAYER 3
#define SC_CHAT		4
#define SC_INFO		5

#define MAX_STR_SIZE  100

// 패킷 정보 2
#define CS_UP    1
#define CS_DOWN  2
#define CS_LEFT  3
#define CS_RIGHT    4
#define CS_CHAT		5
#define CS_Attack		6
#define CS_Move		7

struct cs_packet_up {
	BYTE size;
	BYTE type;
};

// BYTE를 255까지만 인식하여 int형인 BOOL로 수정

struct sc_packet_put_player {
	BYTE size;
	BYTE type;
	WORD id;
	BOOL x;
	BOOL y;
	BOOL direction;
	BOOL movement;
};

struct sc_packet_pos {
	BYTE size;
	BYTE type;
	WORD id;
	BOOL x;
	BOOL y;
	BOOL direction;
	BOOL movement;
};

struct sc_packet_info {
	BYTE size;
	BYTE type;
	WORD id;
	BOOL hp;
	BOOL MaxHp;
	BOOL level;
	BOOL exp;
	BYTE skill_1;
	BYTE skill_2;
	BYTE skill_3;
	BYTE skill_4;
};

struct sc_packet_remove_player {
	BYTE size;
	BYTE type;
	WORD id;
};

struct sc_packet_chat {
	BYTE size;
	BYTE type;
	WORD id;
	WCHAR message[MAX_STR_SIZE];
};

struct cs_packet_attack {
	BYTE size;
	BYTE type;
	BYTE skill_num;
	BYTE damage;
};

struct cs_packet_Move {
	BYTE size;
	BYTE type;
	BYTE x;
	BYTE y;
};