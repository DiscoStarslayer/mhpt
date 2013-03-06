/**
 * $Id: tunnel-server.h,v 1.8 2008/06/06 10:38:13 pensil Exp $
 * Copyright (c) 2008 Pensil - www.pensil.jp
 * 
 * Tunnel server software for MHP
 */
#ifndef TUNNELSERVER_H_
#define TUNNELSERVER_H_

// If there is no declaration DLLAPI, definition (like a promise) to this
#ifndef DLLAPI
#define DLLAPI extern "C" __declspec(dllimport)
#pragma comment (lib, "tunnelsvr.lib")
#endif /* DLLAPI */

struct ARENA
{
	int size;
	int majorVersion;
	int minorVersion;
	long initTime;
	long lastTime;
	long clock;
	int users;
	char globalName[256];
	char hostName[64];
	int port;
	char name[64];
	char description[128];
	char welcomeMessage[128];
	char entryPassword[32];
	char adminPassword[32];
	char adminId[32];
};

// The callback function
typedef void (*server_log_handler)(const char * logText);

DLLAPI void _stdcall SetServerLogHandler(server_log_handler handler);

DLLAPI void _stdcall GetServerSetting(struct ARENA * arena);

DLLAPI bool _stdcall OpenServer(struct ARENA * arena);

DLLAPI bool _stdcall CloseServer();

DLLAPI void _stdcall WaitServer();

//extern bool LocalConnect(struct SOCKET_EX *);

//extern void UseSSIDFilter(bool value);

#endif /*TUNNELSERVER_H_*/
