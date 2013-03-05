/**
 * $Id: tunnel-client.h,v 1.13 2008/06/08 09:53:17 pensil Exp $
 * Copyright (c) 2008 Pensil - www.pensil.jp
 * 
 * MHP用 トンネルクライアント
 */

#ifndef TUNNELCLIENT_H_
#define TUNNELCLIENT_H_

// DLLAPI宣言がなければ、これを定義 (お約束のようです)
#ifndef DLLAPI
#define DLLAPI extern "C" __declspec(dllimport)
#pragma comment (lib, "tunnel.lib")
#endif /* DLLAPI */

struct WIRELESS_LAN_DEVICE;

struct WIRELESS_LAN_DEVICE
{
	WIRELESS_LAN_DEVICE * nextDevice;
	char * name;
	char * description;
};

struct CONSOLE_LOG
{
	int type;
	int option;
	char * text;
};

struct COMMAND_RESULT
{
	COMMAND_RESULT * next;
	char * text;
};

#define TUNNEL_EVENT_NOTICE    (int) 0
#define TUNNEL_EVENT_CLIENTLOG (int) 1
#define TUNNEL_EVENT_SERVERLOG (int) 2
#define TUNNEL_EVENT_ENTERUSER (int) 3
#define TUNNEL_EVENT_LEAVEUSER (int) 4
#define TUNNEL_EVENT_ENTERPSP  (int) 5
#define TUNNEL_EVENT_LEAVEPSP  (int) 6
#define TUNNEL_EVENT_RESULT    (int) 7
#define TUNNEL_EVENT_ERROR     (int)-1

// コールバック関数
typedef void (*tunnel_handler)(int eventType, int option, const char * logText);

DLLAPI void _stdcall SetClientLogHandler(tunnel_handler handler);

DLLAPI COMMAND_RESULT * _stdcall TextCommand(const char * szTextCommand);

DLLAPI void _stdcall FreeCommandResult(COMMAND_RESULT *);

DLLAPI CONSOLE_LOG * _stdcall GetConsoleLog();

DLLAPI const char * _stdcall GetConsoleLogText();

DLLAPI int _stdcall GetAdapterStatus();

DLLAPI int _stdcall GetClientStatus();

DLLAPI void _stdcall UseConsoleEvent(bool);

#endif /*TUNNELCLIENT_H_*/
