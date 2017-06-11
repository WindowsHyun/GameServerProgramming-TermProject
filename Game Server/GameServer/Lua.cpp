#include "Server.h"

extern "C" {
#include "LUA\lua.h"
#include "LUA\lauxlib.h"
#include "LUA\lualib.h"
}

int API_get_x( lua_State *L ) {
	int oid = (int)lua_tonumber( L, -1 );
	lua_pop( L, 2 );
	lua_pushnumber( L, g_clients[oid].x );
	return 1;
}

int API_get_y( lua_State *L ) {
	int oid = (int)lua_tonumber( L, -1 );
	lua_pop( L, 2 );
	lua_pushnumber( L, g_clients[oid].y );
	return 1;
}

int API_Send_Message( lua_State *L ) {
	int target_client = (int)lua_tonumber( L, -3 );
	int my_id = (int)lua_tonumber( L, -2 );
	char *mess = (char *)lua_tostring( L, -1 );
	lua_pop( L, 4 );

	size_t wlen, len = strlen( mess ) + 1;
	wchar_t wmess[MAX_STR_SIZE];
	len = (MAX_STR_SIZE - 1 < len) ? MAX_STR_SIZE - 1 : len;
	mbstowcs_s( &wlen, wmess, len, mess, _TRUNCATE );
	wmess[MAX_STR_SIZE - 1] = (wchar_t)0;
	SendChatPacket( target_client, my_id, wmess );
	return 0;
}

char * ConvertWCtoC( wchar_t* str ) {
	//반환할 char* 변수 선언
	char* pStr;
	//입력받은 wchar_t 변수의 길이를 구함
	int strSize = WideCharToMultiByte( CP_ACP, 0, str, -1, NULL, 0, NULL, NULL );
	//char* 메모리 할당
	pStr = new char[strSize];
	//형 변환 
	WideCharToMultiByte( CP_ACP, 0, str, -1, pStr, strSize, 0, 0 );
	return pStr;
}

wchar_t* ConverCtoWC( char* str ) {
	//wchar_t형 변수 선언
	wchar_t* pStr;
	//멀티 바이트 크기 계산 길이 반환
	int strSize = MultiByteToWideChar( CP_ACP, 0, str, -1, NULL, NULL );
	//wchar_t 메모리 할당
	pStr = new WCHAR[strSize];
	//형 변환
	MultiByteToWideChar( CP_ACP, 0, str, strlen( str ) + 1, pStr, strSize );
	return pStr;
}