#include "Server.h"
//--------------------------------------------------------------------------------
// DB연동 함수 및 ...
#define Default_LEN 10
SQLHENV henv;
SQLHDBC hdbc;
SQLHSTMT hstmt = 0;
SQLRETURN retcode;
SQLWCHAR sz_id[Default_LEN];
int db_x, db_y, db_level, db_hp, db_maxhp, db_exp, db_skill[4], db_connect;
SQLLEN cb_id = 0, cb_x = 0, cb_y = 0, cb_level = 0, cb_hp = 0, cb_maxhp = 0, cb_exp = 0, cb_skill[4]{ 0 }, cb_connect = 0;

char  buf[255];
wchar_t sql_data[255];
SQLWCHAR sqldata[255] = { 0 };
//--------------------------------------------------------------------------------

void init_DB() {
	// Allocate environment handle  
	retcode = SQLAllocHandle( SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv );

	// Set the ODBC version environment attribute  
	if ( retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO ) {
		retcode = SQLSetEnvAttr( henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0 );
		// Allocate connection handle  
		if ( retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO ) {
			retcode = SQLAllocHandle( SQL_HANDLE_DBC, henv, &hdbc );

			// Set login timeout to 5 seconds  
			if ( retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO )
			{
				SQLSetConnectAttr( hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0 );
			}
		}
	}

	std::cout << std::endl << "DB Load Complete!" << std::endl;
}

int get_DB_Info( int ci ) {
	// Connect to data source  
	retcode = SQLConnect( hdbc, (SQLWCHAR*)L"2012180004_game", SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0 );

	// Allocate statement handle  
	if ( retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO ) {
		retcode = SQLAllocHandle( SQL_HANDLE_STMT, hdbc, &hstmt );

		sprintf( buf, "EXEC dbo.Search_id %s", g_clients[ci].game_id );
		MultiByteToWideChar( CP_UTF8, 0, buf, strlen( buf ), sql_data, sizeof sql_data / sizeof *sql_data );
		sql_data[strlen( buf )] = '\0';

		retcode = SQLExecDirect( hstmt, sql_data, SQL_NTS );
		if ( retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO ) {

			// Bind columns 1, 2, and 3  
			retcode = SQLBindCol( hstmt, 1, SQL_WCHAR, sz_id, Default_LEN, &cb_id );
			retcode = SQLBindCol( hstmt, 2, SQL_INTEGER, &db_x, Default_LEN, &cb_x );
			retcode = SQLBindCol( hstmt, 3, SQL_INTEGER, &db_y, Default_LEN, &cb_y );
			retcode = SQLBindCol( hstmt, 4, SQL_INTEGER, &db_hp, Default_LEN, &cb_hp );
			retcode = SQLBindCol( hstmt, 5, SQL_INTEGER, &db_level, Default_LEN, &cb_level );
			retcode = SQLBindCol( hstmt, 6, SQL_INTEGER, &db_exp, Default_LEN, &cb_exp );
			retcode = SQLBindCol( hstmt, 7, SQL_INTEGER, &db_maxhp, Default_LEN, &cb_maxhp );
			retcode = SQLBindCol( hstmt, 8, SQL_INTEGER, &db_skill[0], Default_LEN, &cb_skill[0] );
			retcode = SQLBindCol( hstmt, 9, SQL_INTEGER, &db_skill[1], Default_LEN, &cb_skill[1] );
			retcode = SQLBindCol( hstmt, 10, SQL_INTEGER, &db_skill[2], Default_LEN, &cb_skill[2] );
			retcode = SQLBindCol( hstmt, 11, SQL_INTEGER, &db_skill[3], Default_LEN, &cb_skill[3] );
			retcode = SQLBindCol( hstmt, 12, SQL_INTEGER, &db_connect, Default_LEN, &cb_connect );

			// Fetch and print each row of data. On an error, display a message and exit.  

			retcode = SQLFetch( hstmt );

			if ( retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO ) {
#if (DebugMod == TRUE )
				printf( "ID : %s\tX : %d\tY : %d\n", sz_id, db_x, db_y );
#endif
				SQLDisconnect( hdbc );
				return DB_Success;
			}
		}

		if ( retcode == SQL_NO_DATA ) {
			SQLDisconnect( hdbc );
			return DB_NoData;
		}

		if ( retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO ) {
			SQLDisconnect( hdbc );
			return DB_NoConnect;
		}
	}
	SQLDisconnect( hdbc );
}

void set_DB_Info( int ci ) {
	// Connect to data source  
	retcode = SQLConnect( hdbc, (SQLWCHAR*)L"2012180004_game", SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0 );

	// Allocate statement handle  
	if ( retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO ) {
		retcode = SQLAllocHandle( SQL_HANDLE_STMT, hdbc, &hstmt );

		sprintf( buf, "EXEC dbo.Set_id %s, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d", g_clients[ci].game_id, g_clients[ci].x, g_clients[ci].y, g_clients[ci].level, g_clients[ci].hp, g_clients[ci].Max_hp, g_clients[ci].exp, g_clients[ci].skill_1, g_clients[ci].skill_2, g_clients[ci].skill_3, g_clients[ci].skill_4 );
		MultiByteToWideChar( CP_UTF8, 0, buf, strlen( buf ), sql_data, sizeof sql_data / sizeof *sql_data );
		sql_data[strlen( buf )] = '\0';

		retcode = SQLExecDirect( hstmt, sql_data, SQL_NTS );
		if ( retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO ) {

			// Bind columns 1, 2, and 3  
			retcode = SQLBindCol( hstmt, 1, SQL_WCHAR, sz_id, Default_LEN, &cb_id );
			retcode = SQLBindCol( hstmt, 2, SQL_INTEGER, &db_x, Default_LEN, &cb_x );
			retcode = SQLBindCol( hstmt, 3, SQL_INTEGER, &db_y, Default_LEN, &cb_y );
			retcode = SQLBindCol( hstmt, 4, SQL_INTEGER, &db_hp, Default_LEN, &cb_hp );
			retcode = SQLBindCol( hstmt, 5, SQL_INTEGER, &db_level, Default_LEN, &cb_level );
			retcode = SQLBindCol( hstmt, 6, SQL_INTEGER, &db_exp, Default_LEN, &cb_exp );
			retcode = SQLBindCol( hstmt, 7, SQL_INTEGER, &db_maxhp, Default_LEN, &cb_maxhp );
			retcode = SQLBindCol( hstmt, 8, SQL_INTEGER, &db_skill[0], Default_LEN, &cb_skill[0] );
			retcode = SQLBindCol( hstmt, 9, SQL_INTEGER, &db_skill[1], Default_LEN, &cb_skill[1] );
			retcode = SQLBindCol( hstmt, 10, SQL_INTEGER, &db_skill[2], Default_LEN, &cb_skill[2] );
			retcode = SQLBindCol( hstmt, 11, SQL_INTEGER, &db_skill[3], Default_LEN, &cb_skill[3] );
			retcode = SQLBindCol( hstmt, 12, SQL_INTEGER, &db_connect, Default_LEN, &cb_connect );

			// Fetch and print each row of data. On an error, display a message and exit.  

			retcode = SQLFetch( hstmt );
			if ( retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO ) {
			}

			if ( retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO ) {
#if (DebugMod == TRUE )
				printf( "ID : %s\tX : %d\tY : %d\n", sz_id, db_x, db_y );
#endif
			}

		}
	}
	SQLDisconnect( hdbc );
}

void new_DB_Id( int ci ) {
	// Connect to data source  
	retcode = SQLConnect( hdbc, (SQLWCHAR*)L"2012180004_game", SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0 );

	// Allocate statement handle  
	if ( retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO ) {
		retcode = SQLAllocHandle( SQL_HANDLE_STMT, hdbc, &hstmt );

		sprintf( buf, "EXEC dbo.new_id %s, %d, %d, %d, %d, %d, %d", g_clients[ci].game_id, g_clients[ci].x, g_clients[ci].y, g_clients[ci].level, g_clients[ci].hp, g_clients[ci].Max_hp, g_clients[ci].exp );
		MultiByteToWideChar( CP_UTF8, 0, buf, strlen( buf ), sql_data, sizeof sql_data / sizeof *sql_data );
		sql_data[strlen( buf )] = '\0';

		retcode = SQLExecDirect( hstmt, sql_data, SQL_NTS );
		if ( retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO ) {

			retcode = SQLFetch( hstmt );
			if ( retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO ) {
			}

			if ( retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO ) {
#if (DebugMod == TRUE )
				printf( "ID : %s\tX : %d\tY : %d\n", sz_id, db_x, db_y );
#endif
			}

		}
	}
	SQLDisconnect( hdbc );
}

void set_DB_Shutdown( int ci ) {
	// Connect to data source  
	retcode = SQLConnect( hdbc, (SQLWCHAR*)L"2012180004_game", SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0 );

	// Allocate statement handle  
	if ( retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO ) {
		retcode = SQLAllocHandle( SQL_HANDLE_STMT, hdbc, &hstmt );

		sprintf( buf, "EXEC dbo.shutdown_id %s", g_clients[ci].game_id );
		MultiByteToWideChar( CP_UTF8, 0, buf, strlen( buf ), sql_data, sizeof sql_data / sizeof *sql_data );
		sql_data[strlen( buf )] = '\0';

		retcode = SQLExecDirect( hstmt, sql_data, SQL_NTS );
		if ( retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO ) {
			retcode = SQLFetch( hstmt );
			if ( retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO ) {
			}

			if ( retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO ) {
#if (DebugMod == TRUE )
				printf( "ID : %s\tX : %d\tY : %d\n", sz_id, db_x, db_y );
#endif
			}

		}
	}
	SQLDisconnect( hdbc );
}