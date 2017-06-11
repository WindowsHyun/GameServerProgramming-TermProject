#ifndef __SERVER_H__
#define __SERVER_H__

#pragma comment(lib, "ws2_32")
#pragma comment(lib, "LUA\\lua53.lib")

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <winsock2.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <unordered_set> // ������ �� ��������. [������ ����������]
#include <random>
#include <chrono>

extern "C" {
#include "LUA\lua.h"
#include "LUA\lauxlib.h"
#include "LUA\lualib.h"
}

using namespace std::chrono;

//---------------------------------------------------------------------------------------------
// ���� ����
#define SERVERPORT 9000
#define BUFSIZE    1024
#define MAX_BUFF_SIZE   4000
#define MAX_PACKET_SIZE  255
#define MAX_Client 500
//---------------------------------------------------------------------------------------------
// ���� ����
#define DebugMod FALSE
#define Game_Width 300														// ����
#define Game_Height 300														// ����
#define VIEW_RADIUS   10														// Viwe �Ÿ�
#define NPC_RADIUS   5														// Viwe �Ÿ�
#define NPC_START  1000
#define MAX_NPC 1050															// NPC ����
#define MAX_STR_SIZE  100
//---------------------------------------------------------------------------------------------

void err_quit( char *msg );
void err_display( char *msg, int err_no );
int getRandomNumber( int min, int max );

void init();
void Worker_Thread();
void Accept_Thread();
void Shutdown_Server();
void SendPutPlayerPacket( int client, int object );
void SendPacket( int cl, void *packet );
void DisconnectClient( int ci );
void ProcessPacket( int ci, char *packet );
void SendPositionPacket( int client, int object );
void SendRemovePlayerPacket( int client, int object );
void SendChatPacket( int target_client, int object_id, wchar_t *mess );
bool Distance( int me, int  you, int Radius );
bool IsPlayer( int ci );

void Timer_Thread();
void init_NPC(); // npc �ʱ�ȭ �Լ�
bool Is_NPC( int id );
void WakeUpNPC( int id );
void NPC_Random_Move( int id );
void check_Attack( int ci, char * ptr );
void Move_NPCtoClient( int ci, int npc );
void NPC_Responder( int id );


int API_get_x( lua_State *L );
int API_get_y( lua_State *L );
int API_Send_Message( lua_State *L );
char * ConvertWCtoC( wchar_t* str );
wchar_t* ConverCtoWC( char* str );

//------------------------------------------------------------
// �� �浹üũ�� ���Ͽ� ����
extern int map[Game_Width][Game_Height];
extern int Tile1[Game_Width][Game_Height];
extern int Tile2[Game_Width][Game_Height];
void read_map();
bool CollisionCheck( int ci, int dir );
//------------------------------------------------------------


enum OPTYPE { OP_SEND, OP_RECV, OP_DO_AI, E_PLAYER_MOVE_NOTIFY, OP_Attack_Move, OP_Responder };
enum Event_Type { E_MOVE, E_Attack_Move, E_Responder };

struct OverlappedEx {
	WSAOVERLAPPED over;
	WSABUF wsabuf;
	unsigned char IOCP_buf[MAX_BUFF_SIZE];
	OPTYPE event_type;
	int target_id;
};

struct CLIENT {
	int x;
	int y;
	int hp;
	int direction = 2;
	int movement = 0;
	bool connect;

	SOCKET client_socket;
	OverlappedEx recv_over;
	unsigned char packet_buf[MAX_PACKET_SIZE];
	int prev_packet_data; // ���� ó������ �ʴ� ��Ŷ�� �󸶳�
	int curr_packet_size; // ���� ó���ϰ� �ִ� ��Ŷ�� �󸶳�
	std::unordered_set<int> view_list; //�� set���� �ξ�������!
	std::mutex vl_lock;

	std::chrono::high_resolution_clock::time_point last_move_time; // npc�� �����ӽð� ���� + ���ǵ��� ����
	bool is_active; // npc�� ���� �������� �ȿ������� �Ǵ��ϱ� ����
	int npc_level = 0; // ������ ������ ��Ÿ����.
	int npc_Attack = 0; // NPC�� ���� ������
	int npc_Client = -1; // NPC�� � Ŭ���̾�Ʈ�� ������
	int npc_dir = -1; // NPC�� Ŭ���̾�Ʈ�� ��� ������
	int npc_x, npc_y; // �׾������ �ʱ� ��ġ�� �̵��Ѵ�.

	lua_State *L;
};

extern CLIENT g_clients[MAX_NPC];

struct Timer_Event {
	int object_id;
	high_resolution_clock::time_point exec_time; // �� �̺�Ʈ�� ���� ����Ǿ� �ϴ°�
	Event_Type event; // ������ Ȯ���ϸ� ����ܿ� �ٸ��͵��� �߰��� ��
};

class comparison {
	bool reverse;
public:
	comparison() {}
	bool operator() ( const Timer_Event first, const Timer_Event second ) const {
		return first.exec_time > second.exec_time;
	}
};

extern std::mutex tq_lock; // �켱���� ť ��� ���� ��
extern std::priority_queue <Timer_Event, std::vector<Timer_Event>, comparison> timer_queue; // �켱����ť, �׳� ���� �����ϱ� ���� �ɾ������

#endif