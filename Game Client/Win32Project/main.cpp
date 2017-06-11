#include "Client.h"
#include "protocol.h"
#include "mdump.h"
//using namespace Gdiplus;

// 다이얼로그 관련
HINSTANCE hInstance;
HWND cpy_hwnd;
HDC cpy_hdc, cpy_memdc;
HWND main_window_handle = NULL; // save the window handle
HINSTANCE main_instance = NULL; // save the instance

// SOCKET 선언
SOCKET g_mysocket;
WSABUF	send_wsabuf;
char 	send_buffer[BUF_SIZE];
WSABUF	recv_wsabuf;
char	recv_buffer[BUF_SIZE];
char	packet_buffer[BUF_SIZE];
DWORD		in_packet_size = 0;
int		saved_packet_size = 0;
int		g_myid;
int		g_left_x = 0, g_top_y = 0;

BOB player;				// 플레이어 Unit
BOB npc[MAX_NPC];      // NPC Unit
BOB skelaton[MAX_Client];     // 상대 플레이어 Unit

void init();


LRESULT CALLBACK WndProc( HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam );

//-------------------------------------------------------------------------
// 이미지 불러오기
//-------------------------------------------------------------------------
// 채팅 관련 함수
bool chat_enabled[MAX_Chat] = { false }; //chat이 들어왔나?
int  chat_ci[MAX_Chat] = { false }; //어떤 NPC가 들어왔나?
int chat_view[MAX_Chat] = { 0 }; // 얼마나 보여줄까?
//-------------------------------------------------------------------------

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpszCmdLine, int nCmdShow ) {
	CMiniDump::Begin();
	DialogBox( hInstance, MAKEINTRESOURCE( IDD_DIALOG1 ), NULL, DlgProc );
	HWND hwnd;
	MSG msg;
	WNDCLASS WndClass;
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = hInstance;
	WndClass.hIcon = LoadIcon( NULL, IDI_APPLICATION );
	WndClass.hCursor = LoadCursor( NULL, IDC_ARROW );
	WndClass.hbrBackground = (HBRUSH)GetStockObject( BLACK_BRUSH );
	//WndClass.hbrBackground = CreateSolidBrush( RGB( 0, 255, 0 ) );
	//WndClass.hbrBackground = CreatePatternBrush( LoadBitmap( hInstance, MAKEINTRESOURCE( IDB_BITMAP7 ) ) );
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = "Window Class Name";

	RegisterClass( &WndClass );
	hwnd = CreateWindow( "Window Class Name", "2012180004 권창현", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 800, NULL, NULL, hInstance, NULL );
	main_window_handle = hwnd;
	main_instance = hInstance;
	init();
	ShowWindow( hwnd, nCmdShow );
	UpdateWindow( hwnd );
	while ( 1 ) {
		while ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
	}
	return msg.wParam;
	CMiniDump::End();
}

LRESULT CALLBACK WndProc( HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM  lParam ) {
	static HDC hdc, mem0dc, mem1dc;
	PAINTSTRUCT ps;
	static HBITMAP hbmOld, hbmMem, hbmMemOld;			// 더블버퍼링을 위하여!
	static char isDebugData[500];
	static int isXdata, isYdata;
	static int isTempHighNum = 2;
	static RECT rt;
	static HBITMAP img_back;
	switch ( iMsg ) {

	case WM_SOCKET:
	{
		if ( WSAGETSELECTERROR( lParam ) ) {
			closesocket( (SOCKET)wParam );
			clienterror();
			break;
		}
		switch ( WSAGETSELECTEVENT( lParam ) ) {
		case FD_READ:
			ReadPacket( (SOCKET)wParam );
			break;
		case FD_CLOSE:
			closesocket( (SOCKET)wParam );
			clienterror();
			break;
		}
	}

	case WM_KEYDOWN:
	{
		if ( wParam == 'a' || wParam == 'A' ) {
			player.skill_num = 0;
			Send_Attack_Packet( player.skill_num, 5 );
			break;
		}
		if ( wParam == 's' || wParam == 'S' ) {
			player.skill_num = 1;
			Send_Attack_Packet( player.skill_num, 10 );
			break;
		}
		if ( wParam == 'd' || wParam == 'D' ) {
			player.skill_num = 2;
			Send_Attack_Packet( player.skill_num, 15 );
			break;
		}
		if ( wParam == 'f' || wParam == 'F' ) {
			player.skill_num = 3;
			Send_Attack_Packet( player.skill_num, 20 );
			break;
		}

		int x = 0, y = 0;
		if ( wParam == VK_RIGHT ) {
			x += 1;
		}
		if ( wParam == VK_LEFT ) {
			x -= 1;
		}
		if ( wParam == VK_UP ) {
			y -= 1;
		}
		if ( wParam == VK_DOWN ) {
			y += 1;
		}
		cs_packet_up *my_packet = reinterpret_cast<cs_packet_up *>(send_buffer);
		my_packet->size = sizeof( my_packet );
		send_wsabuf.len = sizeof( my_packet );
		DWORD iobyte;
		if ( 0 != x ) {
			if ( 1 == x ) my_packet->type = CS_RIGHT;
			else my_packet->type = CS_LEFT;
			int ret = WSASend( g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL );
			if ( ret ) {
				int error_code = WSAGetLastError();
#if (DebugMod == TRUE)
				printf( "Error while sending packet [%d]", error_code );
#endif
			}
		}
		if ( 0 != y ) {
			if ( 1 == y ) my_packet->type = CS_DOWN;
			else my_packet->type = CS_UP;
			WSASend( g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL );
		}

		InvalidateRgn( hwnd, NULL, FALSE );
	}
	break;


	case WM_CREATE:
#if (DebugMod == TRUE)
		AllocConsole();
		freopen( "CONOUT$", "wt", stdout );
#endif
		cpy_hwnd = hwnd;

		srand( (unsigned int)time( NULL ) + GetTickCount() );
		GetClientRect( hwnd, &rt );
		init_Image();
		SetTimer( hwnd, 1, 1, NULL );
		break;

	case WM_PAINT:
		srand( (unsigned int)time( NULL ) );

		hdc = BeginPaint( hwnd, &ps );
		mem0dc = CreateCompatibleDC( hdc );//2
		cpy_hdc = hdc;
		cpy_memdc = mem0dc;
		hbmMem = CreateCompatibleBitmap( hdc, rt.right, rt.bottom );//3
		SelectObject( mem0dc, hbmMem );
		PatBlt( mem0dc, rt.left, rt.top, rt.right, rt.bottom, BLACKNESS );
		hbmMemOld = (HBITMAP)SelectObject( mem0dc, hbmMem );//4
		mem1dc = CreateCompatibleDC( mem0dc );//5

		//printf( "%d,%d\t", g_left_x, g_top_y );

		for ( int i = 0; i < SightSeeWidth; ++i ) {
			for ( int j = 0; j < SightSeeHeight; ++j ) {
				cimage_draw( mem0dc, i, j, i + g_left_x, j + g_top_y );
			}
		}

		sprintf( isDebugData, "Postion : %d, %d [%d]", player.x, player.y, player.movement );



		for ( int i = 0; i < MAX_Client; ++i ) {
			if ( (skelaton[i].attr & BOB_ATTR_VISIBLE) ) {
				Character_You_Draw( mem0dc, skelaton[i].x - g_left_x, skelaton[i].y - g_top_y, skelaton[i].direction, skelaton[i].movement, skelaton[i].hp );
			}
		}

		for ( int i = 0; i < MAX_NPC; ++i ) {
			if ( (npc[i].attr & BOB_ATTR_VISIBLE) && npc[i].hp > 0 ) {
				Monster_Draw( mem0dc, npc[i].x - g_left_x, npc[i].y - g_top_y, 0, npc[i].hp );
			}
		}

		if ( player.skill_num != -1 ) {
			AttackEffect_Draw( mem0dc, player.x - g_left_x, player.y - g_top_y, player.skill_num );
		}

		Character_Draw( mem0dc, 7, 7, player.direction, player.movement, player.hp );

		SetTextColor( mem0dc, RGB( 255, 0, 0 ) );
		SetBkMode( mem0dc, OPAQUE );
		SetTextAlign( hdc, TA_TOP );
		TextOut( mem0dc, 0 + (strlen( isDebugData ) * 3.2), 740, isDebugData, strlen( isDebugData ) );

		for ( int i = 0; i < MAX_Chat; ++i ) {
			if ( chat_enabled[i] == true  ) {
				SetTextColor( mem0dc, RGB( 0, 0, 255 ) );
				SetBkMode( mem0dc, OPAQUE );
				SetTextAlign( hdc, TA_TOP );
				wsprintf( isDebugData, "[NPC : %d] : %ws", chat_ci[i], npc[chat_ci[i]].message );
				TextOut( mem0dc, 0 + (strlen( isDebugData ) * 3.6), 720 - (i * 20), isDebugData, strlen( isDebugData ) );
			}
		}


		BitBlt( hdc, 0, 0, rt.right, rt.bottom, mem0dc, 0, 0, SRCCOPY );

		SelectObject( mem0dc, hbmMemOld ); //-4
		DeleteObject( hbmMem ); //-3
		DeleteObject( hbmMemOld ); //-3
		DeleteDC( mem0dc ); //-2
		DeleteDC( mem1dc ); //-2
		DeleteDC( hdc ); //-2
		EndPaint( hwnd, &ps );
		break;

	case WM_TIMER:
		switch ( wParam ) {
		case 0:
			if ( player.skill_num != -1 ) {
				player.skill_timer += 1;
				if ( player.skill_timer >= MAX_Skill_View ) {
					KillTimer( hwnd, 0 );
					player.skill_num = -1;
				}
			}
			break;
		case 1:
			for ( int i = 0; i < MAX_Chat; ++i ) {
				if ( chat_enabled[i] == true ) {
					chat_view[i]++;
					if ( chat_view[i] >= MAX_Chat_View ) {
						chat_enabled[i] = false;
						chat_view[i] = 0;
						chat_ci[i] = -1;
					}
				}
			}
			break;
		}
		InvalidateRgn( hwnd, NULL, FALSE );
		break;

	case WM_DESTROY:
		PostQuitMessage( 0 );
		exit( -1 );
		break;

	default:break;
	}
	return DefWindowProc( hwnd, iMsg, wParam, lParam );
}

void init() {
	for ( int i = 0; i < Game_Height; ++i ) { // 가로
		for ( int j = 0; j < Game_Width; ++j ) { // 세로
			if ( i % 2 == j % 2 )
				isGameData[i][j] = 1;
		}
	}
	WSADATA	wsadata;
	WSAStartup( MAKEWORD( 2, 2 ), &wsadata );

	g_mysocket = WSASocket( AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0 );

	SOCKADDR_IN ServerAddr;
	ZeroMemory( &ServerAddr, sizeof( SOCKADDR_IN ) );
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons( SERVERPORT );
	ServerAddr.sin_addr.s_addr = inet_addr( ipAddres );

	int Result = WSAConnect( g_mysocket, (sockaddr *)&ServerAddr, sizeof( ServerAddr ), NULL, NULL, NULL, NULL );

	WSAAsyncSelect( g_mysocket, main_window_handle, WM_SOCKET, FD_CLOSE | FD_READ );

	send_wsabuf.buf = send_buffer;
	send_wsabuf.len = BUF_SIZE;
	recv_wsabuf.buf = recv_buffer;
	recv_wsabuf.len = BUF_SIZE;

#if (DebugMod == TRUE)
	err_display( "Connect : ", Result );
#endif
}

void clienterror() {
	exit( -1 );
}

void ReadPacket( SOCKET sock ) {
	DWORD iobyte, ioflag = 0;

	int ret = WSARecv( sock, &recv_wsabuf, 1, &iobyte, &ioflag, NULL, NULL );
	if ( ret ) {
		int err_code = WSAGetLastError();
#if (DebugMod == TRUE)
		printf( "Recv Error [%d]\n", err_code );
#endif
	}

	BYTE *ptr = reinterpret_cast<BYTE *>(recv_buffer);

	while ( 0 != iobyte ) {
		if ( 0 == in_packet_size ) in_packet_size = ptr[0];
		if ( iobyte + saved_packet_size >= in_packet_size ) {
			memcpy( packet_buffer + saved_packet_size, ptr, in_packet_size - saved_packet_size );
			ProcessPacket( packet_buffer );
			ptr += in_packet_size - saved_packet_size;
			iobyte -= in_packet_size - saved_packet_size;
			in_packet_size = 0;
			saved_packet_size = 0;
		}
		else {
			memcpy( packet_buffer + saved_packet_size, ptr, iobyte );
			saved_packet_size += iobyte;
			iobyte = 0;
		}
	}
}

void ProcessPacket( char *ptr )
{
	static bool first_time = true;
	switch ( ptr[1] ) {
	case SC_PUT_PLAYER: {
		sc_packet_put_player *my_packet = reinterpret_cast<sc_packet_put_player *>(ptr);
		int id = my_packet->id;
		if ( first_time ) {
			first_time = false;
			g_myid = id;
		}
		if ( id == g_myid ) {
			player.x = my_packet->x;
			player.y = my_packet->y;
			player.hp = my_packet->hp;
			player.direction = my_packet->direction;
			player.attr |= BOB_ATTR_VISIBLE;
		}
		else if ( id < NPC_START ) {
			skelaton[id].x = my_packet->x;
			skelaton[id].y = my_packet->y;
			skelaton[id].hp = my_packet->hp;
			skelaton[id].attr |= BOB_ATTR_VISIBLE;
		}
		else {
			npc[id - NPC_START].x = my_packet->x;
			npc[id - NPC_START].y = my_packet->y;
			npc[id - NPC_START].hp = my_packet->hp;
			npc[id - NPC_START].attr |= BOB_ATTR_VISIBLE;
#if (DebugMod == TRUE)
			printf( "%d\t", id - NPC_START );
#endif
		}
		InvalidateRgn( cpy_hwnd, NULL, FALSE );
		break;
	}
	case SC_POS:
	{
		sc_packet_pos *my_packet = reinterpret_cast<sc_packet_pos *>(ptr);
		int other_id = my_packet->id;
		if ( other_id == g_myid ) {
			g_left_x = my_packet->x - 7;
			g_top_y = my_packet->y - 7;
			player.x = my_packet->x;
			player.y = my_packet->y;
			player.hp = my_packet->hp;
			player.direction = my_packet->direction;
			player.movement = my_packet->movement;
		}
		else if ( other_id < NPC_START ) {
			skelaton[other_id].x = my_packet->x;
			skelaton[other_id].y = my_packet->y;
			skelaton[other_id].hp = my_packet->hp;
			skelaton[other_id].movement = my_packet->movement;
			skelaton[other_id].direction = my_packet->direction;
		}
		else {
			npc[other_id - NPC_START].x = my_packet->x;
			npc[other_id - NPC_START].y = my_packet->y;
			npc[other_id - NPC_START].hp = my_packet->hp;
		}
		break;
	}

	case SC_REMOVE_PLAYER:
	{
		sc_packet_remove_player *my_packet = reinterpret_cast<sc_packet_remove_player *>(ptr);
		int other_id = my_packet->id;
		if ( other_id == g_myid ) {
			player.attr &= ~BOB_ATTR_VISIBLE;
		}
		else if ( other_id < NPC_START ) {
			skelaton[other_id].attr &= ~BOB_ATTR_VISIBLE;
		}
		else {
			npc[other_id - NPC_START].attr &= ~BOB_ATTR_VISIBLE;
		}
		break;
	}
	case SC_CHAT: {
		sc_packet_chat *my_packet = reinterpret_cast<sc_packet_chat *>(ptr);
		int other_id = my_packet->id;
		if ( other_id == g_myid ) {
			wcsncpy_s( player.message, my_packet->message, 256 );
			player.message_time = GetTickCount();
		}
		else if ( other_id < NPC_START ) {
			wcsncpy_s( skelaton[other_id].message, my_packet->message, 256 );
			skelaton[other_id].message_time = GetTickCount();
		}
		else {
			wcsncpy_s( npc[other_id - NPC_START].message, my_packet->message, 256 );
			npc[other_id - NPC_START].message_time = GetTickCount();
		}

		for ( int i = 0; i < MAX_Chat; ++i ) {
			if ( chat_enabled[i] == true && chat_ci[i] == (other_id - NPC_START) ) {
				break;
			}
			if ( chat_enabled[i] == false ) {
				chat_ci[i] = other_id - NPC_START;
				chat_enabled[i] = true;
				break;
			}
		}

		break;
	}
	default:
#if (DebugMod == TRUE)
		printf( "Unknown PACKET type [%d]\n", ptr[1] );
#endif
		break;
	}
}

void Send_Attack_Packet( int skill_num, int damage ) {
	player.skill_timer = 0; // 스킬 타이머를 구현
	SetTimer( cpy_hwnd, 0, 1, NULL );
	cs_packet_attack *my_packet = reinterpret_cast<cs_packet_attack *>(send_buffer);
	my_packet->size = sizeof( my_packet );
	send_wsabuf.len = sizeof( my_packet );
	DWORD iobyte;
	my_packet->type = CS_Attack;
	my_packet->skill_num = 1;
	my_packet->damage = damage;


	int ret = WSASend( g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL );
	if ( ret ) {
		int error_code = WSAGetLastError();
	}
}