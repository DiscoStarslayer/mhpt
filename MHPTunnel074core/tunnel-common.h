/**
 * $Id: tunnel-common.h,v 1.19 2008/05/30 09:14:27 pensil Exp $
 * Copyright (c) 2008 Pensil - www.pensil.jp
 * 
* Common header program
? * The example I failed too many big stuff too
 */

#if !defined(TUNNEL_COMMON_H)
#define TUNNEL_COMMON_H

#include <winsock2.h>
#include <windows.h>
#include <time.h>

#define maxMacAddr 8

#define MAJOR_VERSION (int)0
#define MINOR_VERSION (int)7

struct USER_INFO {
	clock_t last;
	char szNickName[30];
	int majorVersion;
	int minorVersion;
	char szSSID[33];
	char szUID[33];
	DWORD pid;
	int pspCount;
	clock_t ping;
//	LONG rssi;
};

struct MAC_ADDRESS {
	u_char addr[6];
	clock_t last_clock;
	int findCount;
	boolean active;
	boolean doSend;
};

struct DATA_HEADER {
	u_char dtype;
	u_char doption;
	short dsize;
	long sendCount;
	long recvCount;
};

struct SOCKET_EX;

// Callback function to retrieve the user name
typedef void (*DoConnect)(SOCKET_EX * sdex);
typedef void (*DoCommand)(SOCKET_EX * sdex, const DATA_HEADER * dh, const char * data);
typedef void (*DoClose)(SOCKET_EX * sdex);

struct SOCKET_EX {
	SOCKET sd;
	sockaddr_in sinRemote;
	int state;
	int number;
	int sendSize;
	int recvSize;
	
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

	USER_INFO info;
	
	DoConnect doConnect;
	DoCommand doCommand;
	DoClose doClose;
	
	SOCKET_EX * localConnect;
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

extern bool SendCommand(SOCKET_EX *sdex, DATA_HEADER *hd, const char *data);

extern int SendPacketSize(SOCKET_EX *sdex);

extern int RecvPacketSize(SOCKET_EX *sdex);

extern bool EstablishConnection(SOCKET_EX *sdex, u_long nRemoteAddr, u_short nPort);

extern bool AcceptConnection(SOCKET_EX *sdex, SOCKET ListeningSocket);

extern bool InitConnection(SOCKET_EX *sdex, int bufferSize);

extern bool StartConnection(SOCKET_EX *sdex);

extern bool CloseConnection(SOCKET_EX *sdex);

extern void GetSetting(const char* key, const char* defValue, char* buffer, int size);

extern void SetSetting(const char* key, const char* value);

extern bool InitTunnel();

extern bool CloseTunnel();

extern void BufferDump(SOCKET_EX * sdex, DATA_HEADER * hd, char * data);

extern void LogOut(const char *, const char *);

extern void ServerLogOut(const char *);

#endif // !defined (TUNNEL_COMMON_H)

