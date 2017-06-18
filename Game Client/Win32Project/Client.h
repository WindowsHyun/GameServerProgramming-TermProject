#pragma once
#include <WinSock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <atlimage.h> // CImage
#include "resource.h"

#pragma comment(lib, "ws2_32")
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define DebugMod FALSE													// 개발 할경우 True
#define SERVERPORT 9000
#define BUF_SIZE				1024
#define MAX_BUFF_SIZE   4000
#define MAX_PACKET_SIZE  255
#define MAX_Client 10000
#define Game_Width 300														// 가로
#define Game_Height 300														// 세로
#define SightSeeWidth 20
#define SightSeeHeight 20

#define NPC_START  1000
#define MAX_NPC 2000															// NPC 갯수
#define MAX_Chat 10
#define MAX_Chat_View 100
#define MAX_Skill_View 10

#define	WM_SOCKET				WM_USER + 1
#define BOB_ATTR_VISIBLE        16  // bob is visible

extern char ipAddres[MAX_PATH];
extern char game_id[MAX_PATH];
static int isGameData[Game_Width][Game_Height];						// 게임판 설정하기


//--------------------------------------------------------------------------------------------------------------
extern CImage m_image; // Map 이미지
extern CImage m_Status; // 상태창
extern CImage Ch_image[5]; // 본인 캐릭터
extern CImage You_image[5]; // 상대방 캐릭터
extern CImage Monster_image[10]; // 몬스터 캐릭터
extern CImage Effect_image[5]; // 공격 이펙트

void init_Image(); // 이미지 함수 로드
void Status_Draw( HDC hdc);
void cimage_draw( HDC hdc, int x, int y, int r_x, int r_y );
void Character_Draw( HDC hdc, int x, int y, int direction, int movement, int hp, int exp, int level );
void Character_You_Draw( HDC hdc, int x, int y, int direction, int movement, int hp );
void Monster_Draw( HDC hdc, int x, int y, int Kind, int hp );
void AttackEffect_Draw( HDC hdc, int x, int y, int Kind );
//--------------------------------------------------------------------------------------------------------------

SOCKET init_sock();

void err_quit( char *msg );
void err_display( char *msg, int err_no );

void clienterror();
void ReadPacket( SOCKET sock );
void ProcessPacket( char *ptr );
void Send_Attack_Packet( int skill_num, int damage );
void Send_Move_Packet( int x, int y );

BOOL CALLBACK DlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );

typedef struct BOB_TYP {
	int x, y;          // position bitmap will be displayed at
	int attr;           // attributes pertaining to the object (general)
	int hp;
	int MaxHp;
	int exp;
	int level;
	int direction = 2;		// 캐릭터 방향
	int movement = 0;	// 캐릭터 움직임
	int skill_num = -1;
	int skillTimer_1 = 0;
	int skillTimer_2 = 0;
	int skillTimer_3 = 0;
	int skillTimer_4 = 0;
	WCHAR message[256];
	DWORD message_time;

} BOB, *BOB_PTR;

extern BOB player;				// 플레이어 Unit