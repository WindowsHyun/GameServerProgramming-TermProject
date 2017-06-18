#include "Server.h"
#include "protocol.h"

void err_quit( char *msg ) {
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
		(LPTSTR)&lpMsgBuf, 0, NULL );
	printf( "Error : %s\n", msg );
	LocalFree( lpMsgBuf );
	exit( 1 );
}

void err_display( char *msg, int err_no ) {
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
		(LPTSTR)&lpMsgBuf, 0, NULL );
	printf( "[%s] %s\n", msg, (char *)lpMsgBuf );
	LocalFree( lpMsgBuf );
}

void SendPacket( int cl, void *packet ) {
	if ( g_clients[cl].connect == true ) {
		int psize = reinterpret_cast<unsigned char *>(packet)[0];
		int ptype = reinterpret_cast<unsigned char *>(packet)[1];
		OverlappedEx *over = new OverlappedEx;
		ZeroMemory( &over->over, sizeof( over->over ) );
		over->event_type = OP_SEND;
		memcpy( over->IOCP_buf, packet, psize );
		over->wsabuf.buf = reinterpret_cast<CHAR *>(over->IOCP_buf);
		over->wsabuf.len = psize;
		int res = WSASend( g_clients[cl].client_socket, &over->wsabuf, 1, NULL, 0, &over->over, NULL );
		if ( 0 != res ) {
			int error_no = WSAGetLastError();
			if ( WSA_IO_PENDING != error_no ) {
				err_display( "SendPacket:WSASend", error_no );
				DisconnectClient( cl );
			}
		}
	}
}

void SendChatPacket( int target_client, int object_id, wchar_t *mess ) {
	sc_packet_chat packet;
	packet.id = object_id;
	packet.size = sizeof( packet );
	packet.type = SC_CHAT;
	wcscpy( packet.message, mess );
	if ( strcmp( ConvertWCtoC( packet.message ), "Hello" ) == 0 ) {
		NPC_Random_Move( object_id );
	}
	else {
		// Hello 로 안보냈을경우
	}
	SendPacket( target_client, reinterpret_cast<unsigned char *>(&packet) );
}

void SendInfoPacket( int client, int object ) {
	sc_packet_info packet;
	packet.id = object;
	packet.size = sizeof( packet );
	packet.type = SC_INFO;
	packet.hp = g_clients[object].hp;
	packet.MaxHp = g_clients[object].Max_hp;
	packet.level = g_clients[object].level;
	packet.exp = g_clients[object].exp;
	packet.skill_1 = g_clients[object].skill_1;
	packet.skill_2 = g_clients[object].skill_2;
	packet.skill_3 = g_clients[object].skill_3;
	packet.skill_4 = g_clients[object].skill_4;
	SendPacket( client, &packet );
}

void SendPutPlayerPacket( int client, int object ) {
	sc_packet_put_player packet;
	packet.id = object;
	packet.size = sizeof( packet );
	packet.type = SC_PUT_PLAYER;
	packet.x = g_clients[object].x;
	packet.y = g_clients[object].y;
	packet.direction = g_clients[object].direction;
	packet.movement = g_clients[object].movement;

	SendInfoPacket( client, object );
	SendPacket( client, &packet );
}

void SendPositionPacket( int client, int object ) {
	sc_packet_pos packet;
	packet.id = object;
	packet.size = sizeof( packet );
	packet.type = SC_POS;
	packet.x = g_clients[object].x;
	packet.y = g_clients[object].y;
	packet.direction = g_clients[object].direction;
	packet.movement = g_clients[object].movement;
	SendInfoPacket( client, object );
	SendPacket( client, &packet );
}

void SendRemovePlayerPacket( int client, int object ) {
	sc_packet_remove_player packet;
	packet.id = object;
	packet.size = sizeof( packet );
	packet.type = SC_REMOVE_PLAYER;

	SendPacket( client, &packet );
}