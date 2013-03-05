/**
 * $Id: tunnel-common.h,v 1.6 2008/04/29 08:14:53 pensil Exp $
 * Copyright (c) 2008 Pensil - www.pensil.jp
 * 
 * 共通プログラムヘッダ
 * いろいろ詰め込みすぎて大きくなりすぎて失敗した例
 */

#if !defined(TUNNEL_COMMON_H)
#define TUNNEL_COMMON_H

#include <winsock2.h>
#include <windows>
#include <time.h>

#define maxMacAddr 8

#define MAJOR_VERSION (int)0
#define MINOR_VERSION (int)5

struct MAC_ADDRESS {
	u_char addr[6];
	int used;
};

struct DATA_HEADER {
	u_char dtype;
	u_char doption;
	short dsize;
	long sendCount;
	long recvCount;
};

struct SOCKET_EX {
	SOCKET sd;
	sockaddr_in sinRemote;
	int state;
	int number;
	int user;
	int sendSize;
	int recvSize;
	
	HWND hWnd;

	CRITICAL_SECTION sendSection;
	char * sendBuffer;
	int sendReadPoint;
	int sendWritePoint;
	int sendBufferSize;
	HANDLE sendThreadHandle;
	DWORD sendThreadId;
	clock_t sendLast;
	bool sendWaitData;
	int sendState;

	CRITICAL_SECTION recvSection;
	char * recvBuffer;
	int recvReadPoint;
	int recvWritePoint;
	int recvBufferSize;
	HANDLE recvThreadHandle;
	DWORD recvThreadId;
	clock_t recvLast;
	bool recvWaitData;
	int recvState;

	long sendCount;
	long sendCountBefore;
	long recvCount;

	MAC_ADDRESS mac[maxMacAddr];
};

#define STATE_IDOL 0
#define STATE_MALLOCKED 1
#define STATE_WAITTOCONNECT 2
#define STATE_CONNECTED 3
#define STATE_CLOSED 4
#define STATE_ERROR -1

//HWND hWndMain;

extern const char* WSAGetLastErrorMessage(const char* pcMessagePrefix, int nErrorID = 0);

extern u_long LookupAddress(const char* pcHost);

extern bool SendPackets(SOCKET_EX *sdex, char *data, int size);

extern bool SendCommand(SOCKET_EX *sdex, DATA_HEADER *hd, char *data);

extern int SendPacketSize(SOCKET_EX *sdex);

extern int RecvPacketSize(SOCKET_EX *sdex);

extern bool RecvPackets(SOCKET_EX *sdex, char *data, int size, bool flush);

extern bool RecvCommand(SOCKET_EX *sdex, DATA_HEADER *hd, char *data, int maxSize);

extern int SendFromBuffer(SOCKET_EX * sdex);

extern void DoCommand(SOCKET_EX * sdex, DATA_HEADER * dh, char * data);

extern void DoClose(SOCKET_EX * sdex);

extern bool RecvAndExecCommand(SOCKET_EX *sdex);

extern bool EstablishConnection(SOCKET_EX *sdex, u_long nRemoteAddr, u_short nPort);

extern bool AcceptConnection(SOCKET_EX *sdex, SOCKET ListeningSocket);

extern bool InitConnection(SOCKET_EX *sdex, int bufferSize);

extern bool StartConnection(SOCKET_EX *sdex);

extern bool CloseConnection(SOCKET_EX *sdex);

extern void GetSetting(char* key, char* defValue, char* buffer, int size);

extern void GetMacSetting(char* key, MAC_ADDRESS* mac);

extern void SetSetting(char* key, char* value);

extern void PrintMac(MAC_ADDRESS* mac);

extern int InputFromUser(char* title, char* buffer, int size);

extern DWORD WINAPI DoMain(void *);

// 内容をMessageBoxで表示。
extern BOOL formatmessagebox(HWND hWnd, DWORD dwMessageId);

// 内容をConsoleで表示。
extern BOOL formatconsole(DWORD dwMessageId);

#endif // !defined (TUNNEL_COMMON_H)

