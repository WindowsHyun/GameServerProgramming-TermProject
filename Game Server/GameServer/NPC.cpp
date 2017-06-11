#include "Server.h"
#include "protocol.h"

std::mutex tq_lock;
std::priority_queue <Timer_Event, std::vector<Timer_Event>, comparison> timer_queue;


void init_NPC() {
#if (DebugMod == TRUE )
	int npc_num = 0;
#endif

	for ( int i = NPC_START; i < MAX_NPC; ++i ) {
		g_clients[i].connect = false;
		g_clients[i].x = getRandomNumber( 4, 45 ); // 모든맵에 뿌려줘야지 안그러면 모든 npc가 겹쳐버림
		g_clients[i].y = getRandomNumber( 50, 100 ); // 마찬가지
		while ( CollisionCheck( i, CS_NPC ) == false ) {
			g_clients[i].x = getRandomNumber( 4, 45 ); // 모든맵에 뿌려줘야지 안그러면 모든 npc가 겹쳐버림
			g_clients[i].y = getRandomNumber( 50, 100 ); // 마찬가지
		}

		g_clients[i].last_move_time = std::chrono::high_resolution_clock::now(); // 라스트 무브 타임 초기화
		g_clients[i].is_active = false;
		g_clients[i].npc_level = 0;
		g_clients[i].hp = 50;
		g_clients[i].npc_Attack = g_clients[i].hp / 10;
		g_clients[i].npc_x = g_clients[i].x;
		g_clients[i].npc_y = g_clients[i].y;


		Timer_Event event = { i, high_resolution_clock::now() + 60s ,E_Responder };  // 몬스터 리스폰을 60초만다 한번씩 확인한다.
		tq_lock.lock();
		timer_queue.push( event );
		tq_lock.unlock();


		lua_State *L = luaL_newstate();
		luaL_openlibs( L );
		luaL_loadfile( L, "AI_Script\\monster_AI.lua" );
		if ( 0 != lua_pcall( L, 0, 0, 0 ) ) {
			std::cout << "LUA error loading [AI_Script\\monster_AI.lua] : " << (char *)lua_tostring( L, -1 ) << std::endl;
		}
		lua_getglobal( L, "set_uid" );
		lua_pushnumber( L, i );
		if ( 0 != lua_pcall( L, 1, 1, 0 ) ) {
			std::cout << "LUA error calling [set_uid] : " << (char *)lua_tostring( L, -1 ) << std::endl;
		}

		lua_pop( L, 1 );

		lua_register( L, "API_get_x", API_get_x );
		lua_register( L, "API_get_y", API_get_y );
		lua_register( L, "API_Send_Message", API_Send_Message );
		g_clients[i].L = L;

#if (DebugMod == TRUE )
		++npc_num;
#endif
	}
	std::cout << std::endl << "Lua Load Complete!" << std::endl;
#if (DebugMod == TRUE )
	printf( "init NPC : %d개\n", npc_num );
#endif

	//----------------------------------------------------------------------------------------
	//for ( int i = NPC_START; i <MAX_NPC; ++i )
	//	printf( "NPC[%d] = ( %d, %d )\n", i, g_clients[i].x, g_clients[i].y );
	//----------------------------------------------------------------------------------------
}

void NPC_Random_Move( int id ) {
	int x = g_clients[id].x;
	int y = g_clients[id].y;


	switch ( getRandomNumber( 0, 3 ) ) {
	case 0: if ( x > 0 )x--; break;
	case 1: if ( y > 0 ) y--; break;
	case 2: if ( x < Game_Width - 2 ) x++; break;
	case 3: if ( y > -(Game_Height - 2) ) y++; break;
	}

	g_clients[id].x = x;
	g_clients[id].y = y;

	while ( CollisionCheck( id, CS_NPC ) == false ) {
		switch ( getRandomNumber( 0, 3 ) ) {
		case 0: if ( x > 0 )x--; break;
		case 1: if ( y > 0 ) y--; break;
		case 2: if ( x < Game_Width - 2 ) x++; break;
		case 3: if ( y > -(Game_Height - 2) ) y++; break;
		}
		g_clients[id].x = x;
		g_clients[id].y = y;
	}

	//std::cout << id << " : " << g_clients[id].x << ", " << g_clients[id].y << std::endl;
	// 주위에 있는 플레이어들에게 npc가 움직였다고 보내줌
	for ( int i = 0; i < MAX_Client; ++i ) {
		if ( true == g_clients[i].connect ) {
			g_clients[i].vl_lock.lock();
			if ( 0 != g_clients[i].view_list.count( id ) ) {
				if ( true == Distance( i, id, VIEW_RADIUS ) ) {
					g_clients[i].vl_lock.unlock();
					SendPositionPacket( i, id );
				}
				else {
					g_clients[i].view_list.erase( id );
					g_clients[i].vl_lock.unlock();
					SendRemovePlayerPacket( i, id ); // npc가 시야에서 멀어지면 지워줌
				}
			}
			else {
				if ( true == Distance( i, id, VIEW_RADIUS ) ) {
					g_clients[i].view_list.insert( id );
					g_clients[i].vl_lock.unlock();
					SendPutPlayerPacket( i, id );
				}
				else {
					g_clients[i].vl_lock.unlock();
				}
			}
		}
	}

	for ( int i = 0; i < MAX_Client; ++i ) {
		if ( (true == g_clients[i].connect) && (Distance( i, id, VIEW_RADIUS )) ) {
			Timer_Event t = { id, high_resolution_clock::now() + 1s, E_MOVE };
			tq_lock.lock();
			timer_queue.push( t );
			tq_lock.unlock(); // 락을 걸어주고 큐에 넣어줘야 된다
			return;
		}
	}
	g_clients[id].is_active = false;
}

void NPC_Responder( int id ) {
	if ( g_clients[id].hp <= 0 && g_clients[id].npc_level == 0 ) {
		g_clients[id].hp = 50;
		g_clients[id].npc_Attack = g_clients[id].hp / 10;
		g_clients[id].x = g_clients[id].npc_x;
		g_clients[id].y = g_clients[id].npc_y;
	}

	Timer_Event event = { id, high_resolution_clock::now() + 60s ,E_Responder };  // 몬스터 리스폰을 60초만다 한번씩 확인한다.
	tq_lock.lock();
	timer_queue.push( event );
	tq_lock.unlock();
}

void Send_Monster_Attack_Chat( int ci, int npc ) {
	char chatMSG[MAX_STR_SIZE];
	g_clients[ci].hp -= g_clients[npc].npc_Attack;
	sprintf( chatMSG, "몬스터가 용사에게 %d의 데미지를 입혔습니다.", g_clients[npc].npc_Attack );
	SendChatPacket( ci, npc, ConverCtoWC( chatMSG ) );
}

void Check_Attack_Client( int ci, int npc ) {


	switch ( g_clients[npc].npc_dir ) {
	case 0:
		if ( (g_clients[ci].x - 1 == g_clients[npc].x) && (g_clients[ci].y == g_clients[npc].y) ) {
			Send_Monster_Attack_Chat( ci, npc );
		}
		break;
	case 1:
		if ( (g_clients[ci].x + 1 == g_clients[npc].x) && (g_clients[ci].y == g_clients[npc].y) ) {
			Send_Monster_Attack_Chat( ci, npc );
		}
		break;
	case 2:
		if ( (g_clients[ci].x == g_clients[npc].x) && (g_clients[ci].y - 1 == g_clients[npc].y) ) {
			Send_Monster_Attack_Chat( ci, npc );
		}
		break;
	case 3:
		if ( (g_clients[ci].x == g_clients[npc].x) && (g_clients[ci].y + 1 == g_clients[npc].y) ) {
			Send_Monster_Attack_Chat( ci, npc );
		}
		break;
	}


}

void Move_NPCtoClient( int ci, int npc ) {
	int Cx = g_clients[ci].x, Cy = g_clients[ci].y;
	int Nx = g_clients[npc].x, Ny = g_clients[npc].y;

	switch ( g_clients[npc].npc_dir ) {
	case 0:
		Cx -= 1;
		break;
	case 1:
		Cx += 1;
		break;
	case 2:
		Cy -= 1;
		break;
	case 3:
		Cy += 1;
		break;
	}

	if ( Nx > Cx && CollisionCheck( npc, CS_LEFT ) )
		g_clients[npc].x -= 1;
	if ( Ny > Cy  && CollisionCheck( npc, CS_UP ) )
		g_clients[npc].y -= 1;
	if ( Nx < Cx  && CollisionCheck( npc, CS_RIGHT ) )
		g_clients[npc].x += 1;
	if ( Ny < Cy && CollisionCheck( npc, CS_DOWN ) )
		g_clients[npc].y += 1;

	Check_Attack_Client( ci, npc );

	for ( int i = 0; i < MAX_Client; ++i ) {
		if ( true == g_clients[i].connect ) {
			g_clients[i].vl_lock.lock();
			if ( 0 != g_clients[i].view_list.count( npc ) ) {
				if ( true == Distance( i, npc, VIEW_RADIUS ) ) {
					g_clients[i].vl_lock.unlock();
					SendPositionPacket( i, npc );
				}
				else {
					g_clients[i].view_list.erase( npc );
					g_clients[i].vl_lock.unlock();
					SendRemovePlayerPacket( i, npc ); // npc가 시야에서 멀어지면 지워줌
				}
			}
			else {
				if ( true == Distance( i, npc, VIEW_RADIUS ) ) {
					g_clients[i].view_list.insert( npc );
					g_clients[i].vl_lock.unlock();
					SendPutPlayerPacket( i, npc );
				}
				else {
					g_clients[i].vl_lock.unlock();
				}
			}
		}
	}


	if ( Distance( npc, ci, NPC_RADIUS ) && g_clients[npc].hp > 0 ) {
		Timer_Event event = { npc, high_resolution_clock::now() + 1s ,E_Attack_Move };
		tq_lock.lock();
		timer_queue.push( event );
		tq_lock.unlock(); // 락을 걸어주고 큐에 넣어줘야 된다
	}
	else {
		// NPC가 보는 시점에서 클라가 안보이면 움직임을 멈춘다.
		g_clients[npc].npc_Client = -1;
	}
}

void WakeUpNPC( int id ) {
	// 이미 움직인 애를 또 움직이면 안되므로 상태가 필요함
	if ( g_clients[id].is_active == true ) return;
	g_clients[id].is_active = true;
	Timer_Event event = { id, high_resolution_clock::now() + 1s ,E_MOVE };
	tq_lock.lock();
	timer_queue.push( event );
	tq_lock.unlock();
}

void Attack_Move( int npc, int ci, cs_packet_attack * my_packet ) {
	char chatMSG[MAX_STR_SIZE];
	if ( g_clients[npc].npc_Client == -1 ) {
		Timer_Event event = { npc, high_resolution_clock::now() + 1s ,E_Attack_Move };
		tq_lock.lock();
		timer_queue.push( event );
		tq_lock.unlock(); // 락을 걸어주고 큐에 넣어줘야 된다
	}
	g_clients[npc].npc_Client = ci;
	g_clients[npc].hp -= my_packet->damage;
	g_clients[npc].npc_dir = getRandomNumber( 0, 3 );
	sprintf( chatMSG, "용사가 몬스터 %d의 데미지를 입혔습니다.", my_packet->damage );
	SendChatPacket( ci, npc, ConverCtoWC( chatMSG ) );
}

void check_Attack( int ci, char * ptr ) {
	// 플레이어가 NPC를 공격했는지 확인하기
	int x = g_clients[ci].x, y = g_clients[ci].y;
	cs_packet_attack *my_packet = reinterpret_cast<cs_packet_attack *>(ptr);

	for ( int i = NPC_START; i < MAX_NPC; ++i ) {
		if ( x - 1 == g_clients[i].x && y == g_clients[i].y ) {
			// 왼쪽
			Attack_Move( i, ci, my_packet );
		}
		if ( x + 1 == g_clients[i].x && y == g_clients[i].y ) {
			// 오른쪽
			Attack_Move( i, ci, my_packet );
		}
		if ( x == g_clients[i].x && y - 1 == g_clients[i].y ) {
			// 위
			Attack_Move( i, ci, my_packet );
		}
		if ( x == g_clients[i].x && y + 1 == g_clients[i].y ) {
			// 아래
			Attack_Move( i, ci, my_packet );
		}
	}
}

