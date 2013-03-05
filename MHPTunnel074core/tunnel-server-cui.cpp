/**
 * $Id: tunnel-server-cui.cpp,v 1.4 2008/06/06 11:38:37 pensil Exp $
 * Copyright (c) 2008 Pensil - www.pensil.jp
 * 
 * MHP用 トンネルクライアント
 */
#include <iostream>

#include <winsock2.h>
#include <windows.h>
#include <time.h>
#include "tunnel-server.h"
//#include "tunnel-common.h"

using namespace std;

void ConsoleServerLogHandler(const char * log)
{
	SYSTEMTIME syst;
	GetLocalTime(&syst);

	printf("%04d/%02d/%02d %02d:%02d:%02d %s\n", syst.wYear, syst.wMonth, syst.wDay, syst.wHour, syst.wMinute, syst.wSecond, log);
}

// CUIで作る場合の処理ルーチン
int main(int argc, char *argv[])
{
	SetServerLogHandler(ConsoleServerLogHandler);

	ARENA arena;

    GetServerSetting(&arena);

    if ( argc > 1 ) {
		int port = 0;
		sscanf(argv[1], "%d", &port);
		
		if (port > 0) {
			arena.port = port;
		}
    }

    printf("MHP Tunnel Server Ver%d.%d by Pensil\n\n", arena.majorVersion, arena.minorVersion);

//	GetSetting(_T("Server"), _T(""), szServer , 256);

//	char szPort[10];
//	GetSetting("Port" , "443", szPort , 10);

	OpenServer(&arena);
	
	WaitServer();

	CloseServer();
	
	return 0;
}
