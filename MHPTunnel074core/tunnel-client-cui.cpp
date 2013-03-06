/**
 * $Id: tunnel-client-cui.cpp,v 1.7 2008/05/18 22:51:13 pensil Exp $
 * Copyright (c) 2008 Pensil - www.pensil.jp
 * 
 * Tunnel client for MHP
 */
#include <iostream>
#include <string.h>
#include <conio.h>
#include <windows.h>

#pragma comment (lib, "tunnel.lib")
#include "tunnel-client.h"

using namespace std;

int InputFromUser(char* title, char* buffer, int size)
{
	printf("%s", title);
	fgets(buffer, size, stdin);
	int len = strlen(buffer);
	if (len > 0) {
		buffer[len - 1] = 0;
	}
	return strlen(buffer);
}

/**
 * Log output to the console
 */ 
void TunnelEvent(int eventType, int , const char * logText)
{
	switch (eventType)
	{
		case TUNNEL_EVENT_ERROR:
			printf("%s\n", logText);
			break;
			// 
		case TUNNEL_EVENT_NOTICE:
			printf("%s\n", logText);
			break;
			// 
		case TUNNEL_EVENT_CLIENTLOG:
			printf("%s\n", logText);
			break;
			// 
		case TUNNEL_EVENT_ENTERUSER:
			break;
			// 
		case TUNNEL_EVENT_LEAVEUSER:
			break;
			// 
		case TUNNEL_EVENT_ENTERPSP:
			break;
			// 
		case TUNNEL_EVENT_LEAVEPSP:
			break;
			// 
		default:
			break;
			// 
	}
}

// Thread log monitoring
DWORD WINAPI MonitorLog(void *)
{
	while (1) {
		CONSOLE_LOG * log = GetConsoleLog();
		if (log != NULL) {
			if (log->text != NULL) {
				TunnelEvent(log->type, log->option, log->text);
			}
		}
    };
//    return 0;
}

char* deviceName[20];
int deviceNum = 0;

int main()
{
	UseConsoleEvent(true);
	CreateThread(0, 0, MonitorLog, NULL, 0, NULL);

	TextCommand("/version");
	const char * version = GetCommandResult();
	
    printf("MHP Tunnel CUI Client Ver%s by Pensil\n\n", version);

//	if (!InitClient()) {
//		return 255;
//	}
	//SetHandler(TunnelEvent);

	TextCommand("/get NickName");
	const char * NickName = GetCommandResult();

	while (strlen(NickName) == 0) {
		char szNickName[30];
		InputFromUser("ニックネーム: ", szNickName, sizeof(szNickName));
		char szBuffer[512];
		snprintf(szBuffer, sizeof(szBuffer), "/set NickName %s", szNickName);
		TextCommand(szBuffer);
		TextCommand("/get NickName");
	}

	TextCommand("/get AutoDevice");
	const char * autoDevice = GetCommandResult();
	if (strcmp(autoDevice, "false") != 0) {
		TextCommand("/opendevice");
		while (GetAdapterStatus()==0) {
			
			int inum = 0;
			TextCommand("/listdevice");
			const char * device = GetCommandResult();
	
			printf("0. Do not use the radio interface (chat only)\n");
			while (device != NULL) {
				deviceName[deviceNum] = (char *)malloc(sizeof(device) + 1);
				strcpy(deviceName[deviceNum], device);
				deviceNum++;
				device = GetCommandResult();
			}
			for (int i = 0; i < deviceNum; i++) {
				char command[100];
				snprintf(command, sizeof(command), "/devicedesc %s", deviceName[i]);
				TextCommand(command);
				const char * description = GetCommandResult();
				printf("%d. %s\n", i+1, description);
			};
		
			if(deviceNum > 0)
			{
				printf("Please select the number of the interface you want to use wireless (0-%d): ", deviceNum);
				char strTmp[32];
				InputFromUser("", strTmp, sizeof(strTmp));
				sscanf(strTmp, "%d", &inum);
			
				if(inum < 0 || inum > deviceNum)
				{
					printf("Input value is out of range.\n");
					/* Free the device list */
				} else if (inum == 0) {
					break;
				} else {
					char command[256];
					snprintf(command, sizeof(command), "/opendevice %s", deviceName[inum-1]);
					TextCommand(command);
				}
			} else {
				printf("There is no wireless interface. I will start in chat mode.\n");
				break;
			}
		}
	}
	
//	printf("\n");
//	printf("サーバーに接続するには、/connect [サーバー名[:ポート番号]] と入力してください。\n");
//	printf("自分がサーバーになるには、/openserver [ポート番号] と入力してください。\n");
	
	TextCommand("/get AutoConnect");
	const char * autoConnect = GetCommandResult();
	if (strcmp(autoConnect, "false") != 0) {
		TextCommand("/connect");
		while (GetClientStatus() == 0) {
			char szServer[30];
			InputFromUser("The destination server (address [: port])): ", szServer, sizeof(szServer));
			char szBuffer[512];
			snprintf(szBuffer, sizeof(szBuffer), "/connect %s", szServer);
			TextCommand(szBuffer);
		}
	}

	TextCommand("/users");

	// Start console input
    char acReadBuffer[4096];
    while (1) {
		fgets( acReadBuffer, sizeof(acReadBuffer) , stdin );  /* From standard input */
		acReadBuffer[strlen(acReadBuffer) - 1] = 0;
		if (strcmp(acReadBuffer, "/exit") == 0) {
			break;
		} else if (strcmp(acReadBuffer, "/logout") == 0) {
			break;
		} else {
			TextCommand(acReadBuffer);
			const char * text = GetCommandResult();
			if (text != NULL) {
				printf("%s\n", text);
			}
		}
    };
	return 0;
}

