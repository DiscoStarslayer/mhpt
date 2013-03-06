/**
 * $Id: CUITunnel.cpp,v 1.2 2008/06/08 09:55:59 pensil Exp $
 * Copyright (c) 2008 Pensil - www.pensil.jp
 * 
 * Tunnel client for MHP
 */
#include <iostream>
#include <string.h>
#include <conio.h>
#include <windows.h>

#include "tunnel-client.h"
#include "cTextCommand.h"

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
	SetClientLogHandler(TunnelEvent);
//	CreateThread(0, 0, MonitorLog, NULL, 0, NULL);

	cTextCommand cmd;
	cmd.execute("/version");
	const char * version = cmd.nextResult();

	printf("MHP Tunnel CUI Client Ver%s by Pensil\n\n", version);

//	if (!InitClient()) {
//		return 255;
//	}
	//SetHandler(TunnelEvent);

//	TextCommand("/get NickName");
	const char * NickName = cmd.getSetting("NickName");

	while (strlen(NickName) == 0) {
		char szNickName[30];
		InputFromUser("Your nickname: ", szNickName, sizeof(szNickName));
		char szBuffer[512];
		cmd.setSetting("NickName", szNickName);
		NickName = cmd.getSetting("NickName");
	}

	const char * autoDevice = cmd.getSetting("AutoDevice");
	if (std::strcmp(autoDevice, "false") != 0) {
		cmd.execute("/opendevice");
		while (GetAdapterStatus()==0) {

			int inum = 0;
			cmd.execute("/listdevice");
			const char * device = cmd.nextResult();

			printf("0. Do not use the radio interface (chat only)\n");
			while (device != NULL) {
				deviceName[deviceNum] = (char *)malloc(sizeof(device) + 1);
				std::strcpy(deviceName[deviceNum], device);
				deviceNum++;
				device = cmd.nextResult();
			}
			for (int i = 0; i < deviceNum; i++) {
				char command[100];
				snprintf(command, sizeof(command), "/devicedesc %s", deviceName[i]);
				cmd.execute(command);
				const char * description = cmd.nextResult();
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
					cmd.execute(command);
				}
			} else {
				printf("There is no wireless interface. I will start in chat mode.\n");
				break;
			}
		}
	}
	Sleep(100);
	
//	printf("\n");
//	printf("サーバーに接続するには、/connect [サーバー名[:ポート番号]] と入力してください。\n");
//	printf("自分がサーバーになるには、/openserver [ポート番号] と入力してください。\n");
	
	const char * autoConnect = cmd.getSetting("AutoConnect");
	if (std::strcmp(autoConnect, "false") != 0) {
		if (std::strcmp(cmd.getSetting("Server"), "")!=0) {
			cmd.execute("/connect");
		}
		while (GetClientStatus() == 0) {
			char szServer[30];
			InputFromUser("The destination server (address [: port]): ", szServer, sizeof(szServer));
			char szBuffer[512];
			snprintf(szBuffer, sizeof(szBuffer), "/connect %s", szServer);
			cmd.execute(szBuffer);
		}
	}

	cmd.execute("/users");

	// Start console input
	char acReadBuffer[4096];
	while (1) {
		fgets( acReadBuffer, sizeof(acReadBuffer) , stdin );  /* From standard input */
		acReadBuffer[strlen(acReadBuffer) - 1] = 0;
		if (std::strcmp(acReadBuffer, "/exit") == 0) {
			break;
		} else if (std::strcmp(acReadBuffer, "/logout") == 0) {
			break;
		} else {
			cmd.execute(acReadBuffer);
			const char * text;
			while ((text = cmd.nextResult()) != NULL) {
				printf("%s\n", text);
			}
		}
	};
	return 0;
}

