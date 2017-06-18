#ifndef __SERVER_H__
#define __SERVER_H__

#pragma comment(lib, "ws2_32")
#pragma comment(lib, "LUA\\lua53.lib")

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#define MoveHACK TRUE

#include <winsock2.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <unordered_set> // 성능이 더 좋아진다. [순서가 상관없을경우]
#include <random>
#include <chrono>
#include <windows.h>  
#include <sqlext.h>  

extern "C" {
#include "LUA\lua.h"
#include "LUA\lauxlib.h"
#include "LUA\lualib.h"
}

using namespace std::chrono;

//---------------------------------------------------------------------------------------------
// 소켓 설정
#define SERVERPORT 9000
#define BUFSIZE    1024
#define MAX_BUFF_SIZE   4000
#define MAX_PACKET_SIZE  255
#define MAX_Client 999
//---------------------------------------------------------------------------------------------
// 게임 설정
#define DebugMod FALSE
#define Game_Width 300														// 가로
#define Game_Height 300														// 세로
#define VIEW_RADIUS   15														// Viwe 거리
#define NPC_RADIUS   10														// Viwe 거리
#define NPC_START  1000
#define MAX_NPC 1790
#define MAX_STR_SIZE  100
//---------------------------------------------------------------------------------------------

void err_quit( char *msg );
void err_display( char *msg, int err_no );
int getRandomNumber( int min, int max );

void init();
void Worker_Thread();
void Accept_Thread();
void Shutdown_Server();
void SendInfoPacket( int client, int object );
void SendPutPlayerPacket( int client, int object );
void SendPacket( int cl, void *packet );
void DisconnectClient( int ci );
void ProcessPacket( int ci, char *packet );
void SendPositionPacket( int client, int object );
void SendRemovePlayerPacket( int client, int object );
void SendChatPacket( int target_client, int object_id, wchar_t *mess );
bool Distance( int me, int  you, int Radius );
bool IsPlayer( int ci );
void check_Player_HP();
void check_Player_Level( int ci );
void Teleport_Move( int ci, char * ptr );

void Timer_Thread();
void init_NPC(); // npc 초기화 함수
bool Is_NPC( int id );
void WakeUpNPC( int id );
void NPC_Random_Move( int id );
void check_Attack( int ci, char * ptr );
void Move_NPCtoClient( int ci, int npc );
void NPC_Responder( int id );
void check_Monster_HP( int id );

int API_get_x( lua_State *L );
int API_get_y( lua_State *L );
int print_LUA( lua_State *L );
int API_get_hp( lua_State *L );
int API_Send_Message( lua_State *L );
char * ConvertWCtoC( wchar_t* str );
wchar_t* ConverCtoWC( char* str );

//------------------------------------------------------------
// DB_init
void init_DB();
extern int db_x, db_y, db_level, db_hp, db_maxhp, db_exp, db_skill[4], db_connect;
int get_DB_Info( int ci );
void set_DB_Info( int ci );
void set_DB_Shutdown( int ci );
void new_DB_Id( int ci );
//------------------------------------------------------------
// 맵 충돌체크를 위하여 보관
extern int map[Game_Width][Game_Height];
extern int Tile1[Game_Width][Game_Height];
extern int Tile2[Game_Width][Game_Height];
void read_map();
bool CollisionCheck( int ci, int dir );
//------------------------------------------------------------


enum OPTYPE { OP_SEND, OP_RECV, OP_DO_AI, E_PLAYER_MOVE_NOTIFY, OP_Attack_Move, OP_Responder };
enum Event_Type { E_MOVE, E_Attack_Move, E_Responder };
enum NPC_Type { N_Peace, N_War };
enum NPC_EXP { No_Send_EXP, Yes_Send_EXP };
enum DB_Info { DB_Success, DB_NoData, DB_NoConnect, DB_Overlap };

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
	char game_id[10]; // 클라에서 받아온 게임아이디를 저장
	int hp;
	int Max_hp;
	int hp_timer = 0;
	int level;
	int exp;
	int skill_1 = 0;
	int skill_2 = 0;
	int skill_3 = 0;
	int skill_4 = 0;
	int direction = 2;
	int movement = 0;
	bool connect;

	SOCKET client_socket;
	OverlappedEx recv_over;
	unsigned char packet_buf[MAX_PACKET_SIZE];
	int prev_packet_data; // 이전 처리되지 않는 패킷이 얼마냐
	int curr_packet_size; // 지금 처리하고 있는 패킷이 얼마냐
	std::unordered_set<int> view_list; //걍 set보다 훨씬빠르다!
	std::mutex vl_lock;

	std::chrono::high_resolution_clock::time_point last_move_time; // npc의 움직임시간 제어 + 스피드핵 방지
	bool is_active; // npc가 현재 움직였나 안움직였나 판단하기 위함
	int npc_Attack = 0; // NPC의 공격 데미지
	int npc_Client = -1; // NPC가 어떤 클라이언트를 따라갈지
	int npc_recent_Client = -1; // 가장 최근에 공격을 하려한 Client가 누구인지 확인한다.
	int npc_dir = -1; // NPC가 클라이언트의 어디에 있을지
	int npc_type = 0;
	int npc_x, npc_y; // 죽었을경우 초기 위치로 이동한다.

	lua_State *L;
};

extern CLIENT g_clients[MAX_NPC];

struct Timer_Event {
	int object_id;
	high_resolution_clock::time_point exec_time; // 이 이벤트가 언제 실행되야 하는가
	Event_Type event; // 게임을 확장하며 무브외에 다른것들이 추가될 것
};

class comparison {
	bool reverse;
public:
	comparison() {}
	bool operator() ( const Timer_Event first, const Timer_Event second ) const {
		return first.exec_time > second.exec_time;
	}
};

extern std::mutex tq_lock; // 우선순위 큐 제어를 위한 락
extern std::priority_queue <Timer_Event, std::vector<Timer_Event>, comparison> timer_queue; // 우선순위큐, 그냥 쓰면 죽으니까 락을 걸어줘야함
extern std::chrono::high_resolution_clock::time_point serverTimer;

#endif