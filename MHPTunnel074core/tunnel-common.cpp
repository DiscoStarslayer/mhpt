/**
 * $Id: tunnel-common.cpp,v 1.29 2008/06/08 09:53:17 pensil Exp $
 * Copyright (c) 2008 Pensil - www.pensil.jp
 * 
 * According to the WinSock, to support communication between the server / client
 */

#include "tunnel-common.h"

#include <iostream>
#include <algorithm>
#include <strstream>
//#include <psapi.h>

using namespace std;

#if !defined(_WINSOCK2API_) 
// Winsock 2 header defines this, but Winsock 1.1 header doesn't.  In
// the interest of not requiring the Winsock 2 SDK which we don't really
// need, we'll just define this one constant ourselves.
#define SD_SEND 1
#endif

//// Constants /////////////////////////////////////////////////////////

const int kBufferSize = 32768;
static char * eof = "<E>";

//char eof[2];

// List of Winsock error constants mapped to an interpretation string.
// Note that this list must remain sorted by the error constants'
// values, because we do a binary search on the list when looking up
// items.
static struct ErrorEntry {
    int nID;
    const char* pcMessage;

    ErrorEntry(int id, const char* pc = 0) : 
    nID(id), 
    pcMessage(pc) 
    { 
    }

    bool operator<(const ErrorEntry& rhs) 
    {
        return nID < rhs.nID;
    }
} gaErrorList[] = {
    ErrorEntry(0,                  "No error"),
    ErrorEntry(WSAEINTR,           "Interrupted system call"),
    ErrorEntry(WSAEBADF,           "Bad file number"),
    ErrorEntry(WSAEACCES,          "Permission denied"),
    ErrorEntry(WSAEFAULT,          "Bad address"),
    ErrorEntry(WSAEINVAL,          "Invalid argument"),
    ErrorEntry(WSAEMFILE,          "Too many open sockets"),
    ErrorEntry(WSAEWOULDBLOCK,     "Operation would block"),
    ErrorEntry(WSAEINPROGRESS,     "Operation now in progress"),
    ErrorEntry(WSAEALREADY,        "Operation already in progress"),
    ErrorEntry(WSAENOTSOCK,        "Socket operation on non-socket"),
    ErrorEntry(WSAEDESTADDRREQ,    "Destination address required"),
    ErrorEntry(WSAEMSGSIZE,        "Message too long"),
    ErrorEntry(WSAEPROTOTYPE,      "Protocol wrong type for socket"),
    ErrorEntry(WSAENOPROTOOPT,     "Bad protocol option"),
    ErrorEntry(WSAEPROTONOSUPPORT, "Protocol not supported"),
    ErrorEntry(WSAESOCKTNOSUPPORT, "Socket type not supported"),
    ErrorEntry(WSAEOPNOTSUPP,      "Operation not supported on socket"),
    ErrorEntry(WSAEPFNOSUPPORT,    "Protocol family not supported"),
    ErrorEntry(WSAEAFNOSUPPORT,    "Address family not supported"),
    ErrorEntry(WSAEADDRINUSE,      "Address already in use"),
    ErrorEntry(WSAEADDRNOTAVAIL,   "Can't assign requested address"),
    ErrorEntry(WSAENETDOWN,        "Network is down"),
    ErrorEntry(WSAENETUNREACH,     "Network is unreachable"),
    ErrorEntry(WSAENETRESET,       "Net connection reset"),
    ErrorEntry(WSAECONNABORTED,    "Software caused connection abort"),
    ErrorEntry(WSAECONNRESET,      "Connection reset by peer"),
    ErrorEntry(WSAENOBUFS,         "No buffer space available"),
    ErrorEntry(WSAEISCONN,         "Socket is already connected"),
    ErrorEntry(WSAENOTCONN,        "Socket is not connected"),
    ErrorEntry(WSAESHUTDOWN,       "Can't send after socket shutdown"),
    ErrorEntry(WSAETOOMANYREFS,    "Too many references, can't splice"),
    ErrorEntry(WSAETIMEDOUT,       "Connection timed out"),
    ErrorEntry(WSAECONNREFUSED,    "Connection refused"),
    ErrorEntry(WSAELOOP,           "Too many levels of symbolic links"),
    ErrorEntry(WSAENAMETOOLONG,    "File name too long"),
    ErrorEntry(WSAEHOSTDOWN,       "Host is down"),
    ErrorEntry(WSAEHOSTUNREACH,    "No route to host"),
    ErrorEntry(WSAENOTEMPTY,       "Directory not empty"),
    ErrorEntry(WSAEPROCLIM,        "Too many processes"),
    ErrorEntry(WSAEUSERS,          "Too many users"),
    ErrorEntry(WSAEDQUOT,          "Disc quota exceeded"),
    ErrorEntry(WSAESTALE,          "Stale NFS file handle"),
    ErrorEntry(WSAEREMOTE,         "Too many levels of remote in path"),
    ErrorEntry(WSASYSNOTREADY,     "Network system is unavailable"),
    ErrorEntry(WSAVERNOTSUPPORTED, "Winsock version out of range"),
    ErrorEntry(WSANOTINITIALISED,  "WSAStartup not yet called"),
    ErrorEntry(WSAEDISCON,         "Graceful shutdown in progress"),
    ErrorEntry(WSAHOST_NOT_FOUND,  "Host not found"),
    ErrorEntry(WSANO_DATA,         "No host data of that type was found")
};
const int kNumMessages = sizeof(gaErrorList) / sizeof(ErrorEntry);


//// WSAGetLastErrorMessage ////////////////////////////////////////////
// A function similar in spirit to Unix's perror() that tacks a canned 
// interpretation of the value of WSAGetLastError() onto the end of a
// passed string, separated by a ": ".  Generally, you should implement
// smarter error handling than this, but for default cases and simple
// programs, this function is sufficient.
//
// This function returns a pointer to an internal static buffer, so you
// must copy the data from this function before you call it again.  It
// follows that this function is also not thread-safe.

const char* WSAGetLastErrorMessage(const char* pcMessagePrefix, 
    int nErrorID /* = 0 */)
{
    // Build basic error string
    static char acErrorBuffer[256];
    std::ostrstream outs(acErrorBuffer, sizeof(acErrorBuffer));
    outs << pcMessagePrefix << ": ";

    // Tack appropriate canned message onto end of supplied message 
    // prefix. Note that we do a binary search here: gaErrorList must be
	// sorted by the error constant's value.
	ErrorEntry* pEnd = gaErrorList + kNumMessages;
    ErrorEntry Target(nErrorID ? nErrorID : WSAGetLastError());
    ErrorEntry* it = std::lower_bound(gaErrorList, pEnd, Target);
    if ((it != pEnd) && (it->nID == Target.nID)) {
        outs << it->pcMessage;
    }
    else {
        // Didn't find error in list, so make up a generic one
        outs << "unknown error";
    }
    outs << " (" << Target.nID << ")";

    // Finish error message off and return it.
    outs << std::ends;
    acErrorBuffer[sizeof(acErrorBuffer) - 1] = '\0';
    return acErrorBuffer;
}


//// LookupAddress /////////////////////////////////////////////////////
// Given an address string, determine if it's a dotted-quad IP address
// or a domain address.  If the latter, ask DNS to resolve it.  In
// either case, return resolved IP address.  If we fail, we return
// INADDR_NONE.

u_long LookupAddress(const char* pcHost)
{
    u_long nRemoteAddr = inet_addr(pcHost);
    if (nRemoteAddr == INADDR_NONE) {
        // pcHost isn't a dotted IP, so resolve it through DNS
        hostent* pHE = gethostbyname(pcHost);
        if (pHE == 0) {
            return INADDR_NONE;
        }
        nRemoteAddr = *((u_long*)pHE->h_addr_list[0]);
    }

    return nRemoteAddr;
}

// Receive Packet Size rest
int RecvPacketSize(SOCKET_EX *sdex)
{
	//sdex->recvWritePoint : 受信アドレス
	//sdex->recvReadPoint : 取得アドレス
	// 残りサイズ = sdex->recvWritePoint - sdex->recvReadPoint
	int size = sdex->recvWritePoint - sdex->recvReadPoint;
	if (size < 0) {
		// That is less than or equal to zero, that is beyond the end buffer
		size += sdex->recvBufferSize;
	}
    return size;
}

// Remaining transmission packet size
int SendPacketSize(SOCKET_EX *sdex)
{
	//sdex->recvWritePoint : 受信アドレス
	//sdex->recvReadPoint : 取得アドレス
	// 残りサイズ = sdex->recvWritePoint - sdex->recvReadPoint
	int size = sdex->sendWritePoint - sdex->sendReadPoint;
	if (size < 0) {
		// That is less than or equal to zero, that is beyond the end buffer
		size += sdex->sendBufferSize;
	}
    return size;
}

// Remaining size of the receive command
void SendBufferCopy(SOCKET_EX *sdex, const char *data, int start_, int size)
{
//	printf("SendBufferCopy Came from %d  size %d\n", start_, size);
	if (size < 1) {
		return;
	}
	int start = start_;
	while (start > sdex->sendBufferSize) {
		start = start - sdex->sendBufferSize;
	}
	// Whether approaching the break at the end of the buffer
	if ( start + size < sdex->sendBufferSize ) {
		//printf("230:memcopy\n");
		memcpy(&sdex->sendBuffer[start], data, size);
	} else {
		//printf("233:memcopy\n");
		int last = sdex->sendBufferSize - start;
		memcpy(&sdex->sendBuffer[start], data, last);
		memcpy(sdex->sendBuffer, data + last, size - last);
	}
}

// Remaining size of the receive command
void RecvBufferCopy(SOCKET_EX *sdex, char *data, int start_, int size)
{
	int start = start_;
	while (start > sdex->recvBufferSize) {
		start = start - sdex->recvBufferSize;
	}
	// Whether approaching the break at the end of the buffer
	if ( start + size < sdex->recvBufferSize ) {
		//printf("230:memcopy\n");
		memcpy(data, &sdex->recvBuffer[start], size);
	} else {
		//printf("233:memcopy\n");
		int last = sdex->recvBufferSize - start;
		memcpy(data, &sdex->recvBuffer[start], last);
		memcpy(data + last, sdex->recvBuffer, size - last);
	}
}

// Move the rest of the send buffer
void SendBufferMove(SOCKET_EX *sdex, int size)
{
	int newPoint = sdex->sendWritePoint + size;
	if (newPoint > sdex->sendBufferSize) {
		newPoint = newPoint - sdex->sendBufferSize;
	}
//	printf("新しい書き込みポイントは %d (%dプラス)\n", newPoint, size);
	sdex->sendWritePoint = newPoint;
}

// Move the rest of the receive buffer
void RecvBufferMove(SOCKET_EX *sdex, int size)
{
	int newPoint = sdex->recvReadPoint + size;
	if (newPoint > sdex->recvBufferSize) {
		newPoint = newPoint - sdex->recvBufferSize;
	}
//	printf("新しい読み込みポイントは %d\n", newPoint);
	sdex->recvReadPoint = newPoint;
}

// Remaining size of the send command
int SendCommandSize(SOCKET_EX *sdex)
{
	//sdex->sendWritePoint : 送信バッファアドレス
	//sdex->sendReadPoint : 送信アドレス
	// 残りサイズ = sdex->sendWritePoint - sdex->sendReadPoint
	int size = sdex->sendWritePoint - sdex->sendReadPoint;
	if (size < 0) {
		size += sdex->sendBufferSize;
	}
    return size;
}

// Remaining size of the receive command
int RecvCommandSize(SOCKET_EX *sdex)
{
	//sdex->recvWritePoint : 受信アドレス
	//sdex->recvReadPoint : 取得アドレス
	// 残りサイズ = sdex->recvWritePoint - sdex->recvReadPoint
	int searchSize = RecvPacketSize(sdex);
	if (searchSize < sizeof(DATA_HEADER) + sizeof(eof)) {
		return -1;
	}
	char *addr;
	int checkPoint = sdex->recvReadPoint;
	do {
		int size = sdex->recvWritePoint - checkPoint;
		if (size < 0) {
			// That is less than or equal to zero, that is beyond the end buffer
			int last = sdex->recvBufferSize - checkPoint;
			addr = (char*)memchr( &sdex->recvBuffer[checkPoint], eof[0], last );
			if (addr == NULL) {
				checkPoint = 0;
				addr = (char*)memchr( &sdex->recvBuffer[checkPoint], eof[0], size - last );
				if (addr == NULL) {
//					printf("273: < は、ねーよ!\n");
					return -1;
				}
			}
		} else {
			addr = (char*)memchr( &sdex->recvBuffer[checkPoint], eof[0], size );
			if (addr == NULL) {
//				printf("282: < は、ねーよ!\n");
				return -1;
			}
		}
		checkPoint = (int)(addr - sdex->recvBuffer);
//		printf("282: < は、%d にありました!\n", checkPoint);
		
		// Whether or not the check point at the end of the buffer Sashikaka~tsu
		bool checkOk = false;
		if (checkPoint + (int)sizeof(eof) < sdex->recvBufferSize) {
//			printf("295: バッファの終端以内\n");
			if (memcmp(&sdex->recvBuffer[checkPoint], eof, sizeof(eof))==0) {
				checkOk = true;
			}
		} else {
//			printf("300: バッファの終端\n");
			int last = sdex->recvBufferSize - checkPoint;
			if (memcmp(&sdex->recvBuffer[checkPoint], eof, last)==0) {
//				printf("300: チェック1スルー\n");
				if (memcmp(sdex->recvBuffer, eof + last, sizeof(eof) - last)==0) {
					checkOk = true;
				}
			}
		}
		if (checkOk) {
//			printf("290:  > もあったよー!\n");
			int resultPoint = (int)(addr - sdex->recvBuffer) - sdex->recvReadPoint;
			if (resultPoint < 0) {
				resultPoint += sdex->recvBufferSize;
			}
			return resultPoint+sizeof(eof);
		}
		checkPoint++;
	} while (addr != NULL);
    return -1;
}

bool RecvCommand(SOCKET_EX *sdex, DATA_HEADER *hd, char *data, int maxSize)
{
//	printf("RecvCommand\n");
	if (sdex->state != STATE_CONNECTED) {
		return false;
	}
//	printf("Recv:EnterCriticalSection\n");
	EnterCriticalSection(&sdex->recvSection);

	int size = RecvCommandSize(sdex);
	if (size < sizeof(DATA_HEADER) + sizeof(eof)) {
		//printf("340:LeaveCriticalSection\n");
		LeaveCriticalSection(&sdex->recvSection);
		return false;
	}
//	printf("recv %d から %d バイト受信\n", sdex->recvReadPoint, size);
	
	// Command exceeds the buffer size
	if (size > maxSize) {
		printf("I received a command exceeds the maximum size: Fatal Error.(%d > %d)I will through this command.\n", size, maxSize);
		// 読み込みポインタを移動
//		RecvBufferMove(sdex, size + sizeof(eof));
		RecvBufferMove(sdex, size);
		LeaveCriticalSection(&sdex->recvSection);
		return false;
	}
	
	// If the "HEAD" or "POST" or, "GET" and then check the data format, it is treated as a HTTP
//	RecvBufferCopy(sdex, (char *)hd, sdex->recvReadPoint, sizeof(DATA_HEADER));

	// Gets the header
	RecvBufferCopy(sdex, (char *)hd, sdex->recvReadPoint, sizeof(DATA_HEADER));

	if (size > sizeof(DATA_HEADER)) {
		// Get data
		RecvBufferCopy(sdex, data, sdex->recvReadPoint + sizeof(DATA_HEADER), size - sizeof(DATA_HEADER) - sizeof(eof));
		hd->dsize = (short)(size - sizeof(DATA_HEADER) - sizeof(eof));
	}

	// Move the read pointer
//	RecvBufferMove(sdex, size + sizeof(eof));
	RecvBufferMove(sdex, size);
	//printf("355:LeaveCriticalSection\n");

	if (sdex->sendCountBefore + 1 != hd->sendCount && sdex->sendCountBefore != 0)
	{
		// Data acquisition failure
		char debugini[40];
		GetSetting("debug", "false", debugini, sizeof(debugini));
		if (strcmp(debugini, "true") == 0) {
		
		    BufferDump(sdex, hd, data);
			printf("I reconnect once due to an internal error. I output to a file error.\n");
		}
		
		//printf("LostSendCount! %d - %d = %d\n", hd->sendCount, sdex->sendCountBefore + 1, hd->sendCount - sdex->sendCountBefore - 1);
		
		sdex->sendCountBefore = 0;
		//printf("エラーのため一度再接続します\n");
		LeaveCriticalSection(&sdex->recvSection);
		CloseConnection(sdex);
		return false;
//		CloseConnection(sdex);
	}
	if (sdex->sendCountBefore == hd->sendCount && sdex->sendCountBefore != 0) {
		sdex->sendCountBefore = 0;
//		printf("エラーのため一度再接続します\n");
		CloseConnection(sdex);
		LeaveCriticalSection(&sdex->recvSection);
		return false;
	}
	sdex->sendCountBefore = hd->sendCount;
	sdex->recvCount++;

	LeaveCriticalSection(&sdex->recvSection);
	return true;
}

int SendFromBuffer(SOCKET_EX * sdex);

// Send the command with the specified header
// 0 return: transmission failure
// Return SOCKET_ERROR: socket error
// Greater than return 0: total number of packets sent
bool SendCommand(SOCKET_EX *sdex, DATA_HEADER *hd, const char *data)
{
//	char pb[200];
//	ServerLogOut(pb);
	if (sdex->state != STATE_CONNECTED) {
		return false;
	}
	if (sdex->localConnect != NULL) {
//		ServerLogOut("sdex->localConnect != NULL");
		if (sdex->localConnect->doCommand != NULL) {
//			ServerLogOut("sdex->localConnect->doCommand != NULL");
			sdex->localConnect->doCommand(sdex->localConnect, hd, data);
		}
		return true;
	}
//	printf("SendCommand きました \n");
	EnterCriticalSection(&sdex->sendSection);

	sdex->sendCount++;
	hd->sendCount = sdex->sendCount;
	hd->recvCount = sdex->recvCount;

	int sendSize = sizeof(DATA_HEADER) + (int)hd->dsize + sizeof(eof);
	int start = sdex->sendWritePoint;
	SendBufferCopy(sdex, (char *)hd, start, sizeof(DATA_HEADER));
//	printf("SendCommand 送信コマンドは[%c]\n", sdex->sendBuffer[start]);
	SendBufferCopy(sdex, data, start+sizeof(DATA_HEADER), (int)hd->dsize);
	SendBufferCopy(sdex, eof, start+sizeof(DATA_HEADER)+ (int)hd->dsize, sizeof(eof));
	SendBufferMove(sdex, sendSize);

	int status = sdex->sendState;
	LeaveCriticalSection(&sdex->sendSection);
	if (status == 0 && sdex->sendState == 0) {
		SendFromBuffer(sdex);
		if (sdex->state == STATE_ERROR) {
			Sleep(10);
			CloseConnection(sdex);
		}
	}
	return true;
}
////    return nSentBytes;
//}

bool RecvAndExecCommand(SOCKET_EX * sdex)
{
//	printf("RecvAndExecCommand起動\n");
	DATA_HEADER dh;
	char data[4096];
	while (RecvCommand(sdex, &dh, data, sizeof(data)))
	{
		// Repeated as long as there is a command in the receive buffer
		sdex->doCommand(sdex, &dh, data);
	}
	return false;
}

int RecvToBuffer(SOCKET_EX * sdex)
{
	int last = sdex->recvBufferSize - sdex->recvWritePoint;
	int start = sdex->recvWritePoint;
	if (last == 0) {
		// Read buffer has run its course, read from the beginning of the buffer
		start = 0;
		last = sdex->recvBufferSize;
	}
    //printf("受信バッファ残り : %d バイト\n", sdex->recvBufferSize - sdex->recvWritePoint);

// Critical section as well as one receiver
// There 's no reason to compete, because there is only one receiver thread, and I thought
// No sense to use a critical section.
// So quit
	EnterCriticalSection(&sdex->recvSection);
    //printf("RECV起動\n");
	sdex->recvState = 1;
	sdex->recvLast = clock();
	int nTemp = recv(sdex->sd, &sdex->recvBuffer[start], last, 0);
    if (nTemp > 0) {
//    	printf("%d バイト読み込み\n", nTemp);
		sdex->recvWritePoint = start + nTemp;
		sdex->recvState = 0;
		sdex->recvLast = clock();
// TODO: If you write is beyond the point read point
// But I have not been able to make a buffer overflow error

// Send the packet immediately after receiving Maybe somehow wrong?
		//printf("recv(): SUCCESS!\n");
	} else if (nTemp == SOCKET_ERROR) {
		if(WSAGetLastError() != WSAEWOULDBLOCK){
			sdex->recvState = -1;
			sdex->recvLast = clock();
			sdex->state = STATE_ERROR;
//			printf("%d:受信時エラー切断しました(SOCKET_ERROR)\n", sdex->number);
			//CloseConnection(sdex);
        } else {
			//printf("recv(): WSAEWOULDBLOCK!\n");
			//return 0;
		}
    } else {
		//sdex->state = STATE_ERROR;
//		printf("%d:受信時エラー切断しました(送信バイト0)\n", sdex->number);
		//CloseConnection(sdex);
    	// There is a possibility that has timed out.
    	//Sleep(100);
		sdex->recvState = -1;
		sdex->recvLast = clock();
		sdex->state = STATE_ERROR;
    }
	LeaveCriticalSection(&sdex->recvSection);
	if (nTemp > 0) {
		RecvAndExecCommand(sdex);
	}
	return nTemp;
}

int SendFromBuffer(SOCKET_EX * sdex)
{	
//	printf("SendFromBufferロック\n");
	EnterCriticalSection(&sdex->sendSection);
	int last = SendCommandSize(sdex);
	int start = sdex->sendReadPoint;
	int sendSize = 0;
	if (last == 0) {
		LeaveCriticalSection(&sdex->sendSection);
		return 0;
	}
	while (last > 0) {
		if (start == sdex->sendBufferSize) {
			start = 0;
		}
		if (start + last > sdex->sendBufferSize) {
			last = sdex->sendBufferSize - start;
		}
	    //printf("受信バッファ残り : %d バイト\n", sdex->recvBufferSize - sdex->recvWritePoint);
	
// Critical section as well as one receiver
// There 's no reason to compete, because there is only one receiver thread, and I thought
// No sense to use a critical section.
// So quit
		//EnterCriticalSection(&sdex->recvSection);
	    //printf("RECV起動\n");
		sdex->sendState = 1;
		sdex->sendLast = clock();
//	    printf("send起動 %d から %d バイト送信\n", start, last);
		int nTemp = send(sdex->sd, &sdex->sendBuffer[start], last, 0);
	    if (nTemp > 0) {
	    	//printf("%d バイト読み込み\n", nTemp);
			//printf("送信コマンド '%c'\n", sdex->recvBuffer[sdex->recvWritePoint]);
			sdex->sendReadPoint = start + nTemp;
			sdex->sendState = 0;
			sdex->sendLast = clock();
// TODO: If you write is beyond the point read point
// But I have not been able to make a buffer overflow error

// Send the packet immediately after receiving Maybe somehow bad
			//printf("recv(): SUCCESS!\n");
			//RecvAndExecCommand(sdex);
			sendSize += nTemp;
			start += nTemp;
			last -= nTemp;
		} else if (nTemp == SOCKET_ERROR) {
			if(WSAGetLastError() != WSAEWOULDBLOCK){
				sdex->sendState = -1;
				sdex->sendLast = clock();
				sdex->state = STATE_ERROR;
//				printf("%d:送信時エラー切断しました(SOCKET_ERROR)\n", sdex->number);
				//CloseConnection(sdex);
				LeaveCriticalSection(&sdex->sendSection);
				return -1;
	        } else {
				//printf("recv(): WSAEWOULDBLOCK!\n");
				LeaveCriticalSection(&sdex->sendSection);
				return sendSize;
			}
	    } else {
			//sdex->state = STATE_ERROR;
//			printf("%d:送信時エラー切断しました(送信バイト0)\n", sdex->number);
			//CloseConnection(sdex);
	    	// There is a possibility that has timed out.
	    	//Sleep(100);
			sdex->sendState = -1;
			sdex->sendLast = clock();
			sdex->state = STATE_ERROR;
			LeaveCriticalSection(&sdex->sendSection);
			return -1;
	    }
	}
	LeaveCriticalSection(&sdex->sendSection);
	return sendSize;
}

/**
 *  Thread to send and receive packets using the event
 */
DWORD WINAPI SocketEventHandler(void * sdex_) 
{
	SOCKET_EX * sdex = (SOCKET_EX *)sdex_;
	
    int nRet;
    HANDLE  hEvent = WSACreateEvent();
    WSANETWORKEVENTS    events;

    WSAEventSelect(sdex->sd,hEvent,FD_READ|FD_WRITE|FD_CLOSE);

    while(sdex->state == STATE_CONNECTED)
    {
        /*  Wait for Event               */
        nRet = WSAWaitForMultipleEvents(1,&hEvent,FALSE,WSA_INFINITE,FALSE);

        if(nRet == WSA_WAIT_FAILED)
        {
            break;
        }

        /*  Investigation of the event             */
        if(WSAEnumNetworkEvents(sdex->sd,hEvent,&events) == SOCKET_ERROR)
        {
        	// Something was in error
			//printf("SOCKET_ERROR on RecvEvent\n");
            break;
        }
        else
        {
            /*  CLOSE                       */
            if(events.lNetworkEvents & FD_CLOSE)
            {
//				printf("FD_CLOSE on RecvEvent\n");
                //CloseConnection(sdex);
                break;
            }
                                            /*  READ                        */
            if(events.lNetworkEvents & FD_READ)
            {
//				printf("FD_READ on RecvEvent\n");
				RecvToBuffer(sdex);
				SendFromBuffer(sdex);
            }
                                            /*  FD_WRITE                        */
            if(events.lNetworkEvents & FD_WRITE)
            {
//				printf("FD_WRITE on RecvEvent\n");
				SendFromBuffer(sdex);
				RecvToBuffer(sdex);
            }
        }
    }

    WSACloseEvent(hEvent);                  /*  Events close            */

    CloseConnection(sdex);
    return 0;
}

// Submit Thread Ping
DWORD WINAPI PingAction(void * sdex_) 
{
	DATA_HEADER dh;
//	clock_t data = clock();
	dh.dsize = sizeof(int);
	dh.dtype = 'P';
	SOCKET_EX * sdex = (SOCKET_EX *)sdex_;
	while (sdex->state == STATE_CONNECTED) {
		Sleep(3000);
		clock_t pingClock = clock();
		char sendData[sizeof(int)];
		memcpy(sendData, (char *)&pingClock, sizeof(int));
		SendCommand(sdex, &dh, sendData);
    };
    return 0;
}

//// EstablishConnection ///////////////////////////////////////////////
// Connects to a given address, on a given port, both of which must be
// in network byte order.  Returns newly-connected socket if we succeed,
// or INVALID_SOCKET if we fail.

bool EstablishConnection(SOCKET_EX *sdex, u_long nRemoteAddr, u_short nPort)
{
//	printf("EstablishConnection\n");
    InitConnection(sdex, 65536);
    // Create a stream socket
    sdex->sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sdex->sd == INVALID_SOCKET) {
//        printf("766 connect 失敗(INVALID_SOCKET)\n");
    	return false;
    }
    //sockaddr_in sinRemote;
    sdex->sinRemote.sin_family = AF_INET;
    sdex->sinRemote.sin_addr.s_addr = nRemoteAddr;
    sdex->sinRemote.sin_port = nPort;
    sdex->sendCountBefore = 0;
//	printf("connect\n");
	sdex->state = STATE_WAITTOCONNECT;
    if (connect(sdex->sd, (sockaddr*)&sdex->sinRemote, sizeof(sockaddr_in)) == SOCKET_ERROR) {
        sdex->sd = INVALID_SOCKET;
//        printf("777 connect 失敗(INVALID_SOCKET)\n");
        return false;
    }
//	printf("StartConnection\n");
//	printf("781 connect 成功\n");
    StartConnection(sdex);
    return true;
}

bool AcceptConnection(SOCKET_EX *sdex, SOCKET ListeningSocket)
{
	//int bOptLen = sizeof(BOOL);
    InitConnection(sdex, 65536);
    int nAddrSize = sizeof(sockaddr_in);
    sdex->state = STATE_WAITTOCONNECT;
	sdex->sd = accept(ListeningSocket, (sockaddr*)&sdex->sinRemote, &nAddrSize);
    if (sdex->sd != INVALID_SOCKET) {
	    sdex->sendCountBefore = 0;
	    StartConnection(sdex);
	    return true;
    }
    return false;
}

bool InitConnection(SOCKET_EX *sdex, int bufferSize)
{
	if (sdex->state != STATE_IDOL) {
//		CloseConnection(sdex);
	}
	InitializeCriticalSection(&sdex->sendSection);
	InitializeCriticalSection(&sdex->recvSection);

	if (sdex->sendBuffer == NULL) {
		sdex->sendBuffer = (char *)malloc(bufferSize);
	}
	if (sdex->recvBuffer == NULL) {
		sdex->recvBuffer = (char *)malloc(bufferSize);
	}

	sdex->sendWritePoint = 0;
	sdex->recvWritePoint = 0;
	sdex->sendReadPoint = 0;
	sdex->recvReadPoint = 0;
	sdex->sendBufferSize = bufferSize;
	sdex->recvBufferSize = bufferSize;
	sdex->sendLast = clock();
	sdex->recvLast = clock();
	sdex->state = STATE_MALLOCKED;
	sdex->sendCount = 0;
	sdex->recvCount = 0;
    sdex->sendCountBefore = 0;
    sdex->localConnect = NULL;
	
//	if (sdex->hWnd == NULL) {
//		InitTunnelWindow(sdex);
//	    CreateThread(0, 0, TunnelMessageReader, (void*)sdex, 0, NULL);
//	}

	for (int i = 0; i < maxMacAddr; i++) {
		sdex->mac[i].active = false;
	}
	
	return true;
}

bool StartConnection(SOCKET_EX *sdex)
{
	int iOptVal;
	int iOptLen = sizeof(int);

    sdex->sendCountBefore = 0;
	sdex->state = STATE_CONNECTED;
	sdex->info.szSSID[0] = 0;
	sdex->info.szNickName[0] = 0;
	sdex->info.majorVersion = 0;
	sdex->info.minorVersion = 0;

	if (sdex->localConnect == NULL) {
		// SO_KEEPALIVE :
		//    Remain connected without disconnecting the communication state. We just make sure to Get
		if (getsockopt(sdex->sd, SOL_SOCKET, SO_KEEPALIVE, (char*)&iOptVal, &iOptLen) != SOCKET_ERROR) {
			//printf("SO_KEEPALIVE Value: %ld\n", iOptVal);
	
		}
		// TCP_NODELAY :
		//    Call now configured to send packets without storing
	    if (setsockopt(sdex->sd, IPPROTO_TCP, TCP_NODELAY, (char*)&iOptVal, iOptLen) != 0) {
	
	    }
	
	    sdex->recvThreadHandle = CreateThread(0, 0, SocketEventHandler, (void*)sdex, 0, &sdex->recvThreadId);
	}
    sdex->sendThreadHandle = CreateThread(0, 0, PingAction, (void*)sdex, 0, &sdex->sendThreadId);
    
    if (sdex->doConnect != NULL) {
    	sdex->doConnect(sdex);
    }
	return true;
}

bool CloseConnection(SOCKET_EX *sdex)
{
	SOCKET_EX closedsdex;
	
	if (sdex->state == STATE_IDOL) {
		return true;
	}

	// Disconnection process invokes the callback functions
	memcpy((char*)&closedsdex, (char*)sdex, sizeof(SOCKET_EX));

	if (sdex->state == STATE_CONNECTED || sdex->state == STATE_ERROR) {
		//TerminateThread(sdex->sendThreadHandle, 3);
		//TerminateThread(sdex->recvThreadHandle, 3);
	}
    if (shutdown(sdex->sd, SD_SEND) == SOCKET_ERROR) {
        sdex->state == STATE_ERROR;
    }
    // Close the socket.
    if (closesocket(sdex->sd) == SOCKET_ERROR) {
        sdex->state == STATE_ERROR;
    }
	//DeleteCriticalSection(&sdex->sendSection);
	//DeleteCriticalSection(&sdex->recvSection);
	//free(sdex->sendBuffer);
	//free(sdex->recvBuffer); // And you seem to fall off the receive buffer
	sdex->state = STATE_IDOL;

	sdex->doClose(&closedsdex);
	
	if (closedsdex.localConnect != NULL) {
		CloseConnection(closedsdex.localConnect);
	}
	
	return true;
}

bool tunnelInited = false;

bool InitTunnel()
{
//	eof[0] = '\n';
//	eof[1] = '\n';
    // Start Winsock up
    if (tunnelInited) {
    	return true;
    }
    WSAData wsaData;
	int nCode;
    if ((nCode = WSAStartup(MAKEWORD(2, 0), &wsaData)) != 0) {
		cerr << "WSAStartup()Function returns an error. The error code is " << nCode << "Is. The old version of Windows?" <<
				endl;
        return false;
    }
    tunnelInited = true;
    return true;
}

bool CloseTunnel()
{
    WSACleanup();
    tunnelInited = false;
    return true;
}

/**
 * Reads the configuration file
 */ 
void GetSetting(const char* key, const char* defValue, char* buffer, int size)
{
	char	cur[512];
	TCHAR	* fileName = "ini";
	if ( GetModuleFileName(NULL, cur, sizeof(cur)) == 0 ) {
		// Error
	}
	strcpy(&cur[strlen(cur) - 3], fileName);
	
	GetPrivateProfileString( 
		_T("Setting") , key , defValue ,
		buffer , size , cur );
}

void SetSetting(const char* key, const char* value)
{
	if (key == NULL) {
		return;
	}
	char	cur[512];
	TCHAR	* fileName = "ini";
	if ( GetModuleFileName(NULL, cur, sizeof(cur)) == 0 ) {
		// Error
	}
	strcpy(&cur[strlen(cur) - 3], fileName);
	
	if (value != NULL) {
		WritePrivateProfileString( _T("Setting") , key , value ,cur );
	} else {
		WritePrivateProfileString( _T("Setting") , key , "" ,cur );
	}
}

void BufferDump(SOCKET_EX * sdex, DATA_HEADER * hd, char * data)
{
	char logdir[256];
	GetSetting("logdir", "logs", logdir, sizeof(logdir));
	if (strlen(logdir) == 0) {
		return;
	}

	SYSTEMTIME syst;
	GetLocalTime(&syst);

	char szPath[_MAX_PATH];
	char szDrive[_MAX_DRIVE];
	char szDir[_MAX_DIR];
	char szFileName[_MAX_FNAME];
	char szExt[_MAX_EXT];
	char szOutput[_MAX_PATH * 5 + 1024];//(^^;
	DWORD dwRet;
	
	//Initialization
	memset(szPath, 0x00, sizeof(szPath));
	memset(szDrive, 0x00, sizeof(szDrive));
	memset(szDir, 0x00, sizeof(szDir));
	memset(szExt, 0x00, sizeof(szExt));
	memset(szOutput, 0x00, sizeof(szOutput));
		
	//Gets the full path name of a running process
	dwRet = GetModuleFileName(NULL, szPath, sizeof(szPath));
	if(dwRet == 0) {
		
	}
		
	//Split the full path name
	_splitpath(szPath, szDrive, szDir, szFileName, szExt); 
	
	//出力文字列を作成
	//wsprintf(szOutput,"実行しているプログラムのフルパス名%s\r\nドライブ%s\r\n"
	//	"ディレクトリ%s\r\nファイル名%s\r\n拡張子%s",
	//	szPath, szDrive, szDir, szFileName, szExt);

	TCHAR	fileName[512];

	wsprintf(fileName,"%s%s%s\\err%04d%02d%02d_%02d%02d%02d_buffer.dmp", szDrive, szDir, logdir, syst.wYear, syst.wMonth, syst.wDay, syst.wHour, syst.wMinute, syst.wSecond);
	printf("%s\n", fileName);

	FILE *errout = fopen(fileName, "wb");
    if( errout != NULL ) {
    	fwrite(sdex->recvBuffer, 1, sdex->recvBufferSize, errout);
	    fclose(errout);
    }

	wsprintf(fileName,"%s%s%s\\err%04d%02d%02d_%02d%02d%02d_sdex.txt", szDrive, szDir, logdir, syst.wYear, syst.wMonth, syst.wDay, syst.wHour, syst.wMinute, syst.wSecond);
	printf("%s\n", fileName);

	FILE *errout2 = fopen(fileName, "w");
    if( errout2 != NULL ) {
        fprintf(errout2, "sdex->state : %d\n", sdex->state);
        fprintf(errout2, "sdex->number : %d\n", sdex->number);
        fprintf(errout2, "sdex->sendSize : %d\n", sdex->sendSize);
        fprintf(errout2, "sdex->recvSize : %d\n", sdex->recvSize);
        fprintf(errout2, "\n");
        fprintf(errout2, "sdex->sendReadPoint : %d\n", sdex->sendReadPoint);
        fprintf(errout2, "sdex->sendWritePoint : %d\n", sdex->sendWritePoint);
        fprintf(errout2, "sdex->sendBufferSize : %d\n", sdex->sendBufferSize);
        fprintf(errout2, "\n");
        fprintf(errout2, "sdex->recvReadPoint : %d\n", sdex->recvReadPoint);
        fprintf(errout2, "sdex->recvWritePoint : %d\n", sdex->recvWritePoint);
        fprintf(errout2, "sdex->recvBufferSize : %d\n", sdex->recvBufferSize);
        fprintf(errout2, "\n");
        fprintf(errout2, "sdex->sendCount : %d\n", (int)sdex->sendCount);
        fprintf(errout2, "sdex->sendCountBefore : %d\n", (int)sdex->sendCountBefore);
        fprintf(errout2, "sdex->recvCount : %d\n", (int)sdex->recvCount);
        fprintf(errout2, "\n");
        fprintf(errout2, "hd->dtype : %d\n", (int)hd->dtype);
        fprintf(errout2, "hd->doption : %d\n", (int)hd->doption);
        fprintf(errout2, "hd->dsize : %d\n", (int)hd->dsize);
	    fclose(errout2);
    }

	wsprintf(fileName,"%s%s%s\\err%04d%02d%02d_%02d%02d%02d_data.dmp", szDrive, szDir, logdir, syst.wYear, syst.wMonth, syst.wDay, syst.wHour, syst.wMinute, syst.wSecond);
	printf("%s\n", fileName);

	FILE *errout3 = fopen(fileName, "wb");
    if( errout3 != NULL ) {
    	fwrite(data, 1, (int)hd->dsize, errout3);
	    fclose(errout3);
    }
}

void LogOut(const char * type, const char * text)
{
	char logdir[256];
	GetSetting("logdir", "logs", logdir, sizeof(logdir));
	if (strlen(logdir) == 0) {
		return;
	}

	SYSTEMTIME syst;
	GetLocalTime(&syst);

	char szPath[_MAX_PATH];
	char szDrive[_MAX_DRIVE];
	char szDir[_MAX_DIR];
	char szFileName[_MAX_FNAME];
	char szExt[_MAX_EXT];
	char szOutput[_MAX_PATH * 5 + 1024];
	DWORD dwRet;
	
	//Initialization
	memset(szPath, 0x00, sizeof(szPath));
	memset(szDrive, 0x00, sizeof(szDrive));
	memset(szDir, 0x00, sizeof(szDir));
	memset(szExt, 0x00, sizeof(szExt));
	memset(szOutput, 0x00, sizeof(szOutput));
		
	//Gets the full path name of a running process
	dwRet = GetModuleFileName(NULL, szPath, sizeof(szPath));
	if(dwRet == 0) {
		
	}
		
	//Split the full path name
	_splitpath(szPath, szDrive, szDir, szFileName, szExt); 
	
	TCHAR	fileName[512];
	SECURITY_ATTRIBUTES secAttr;

	wsprintf(fileName,"%s%s%s", szDrive, szDir, logdir);
	
	if (-1 == (int)GetFileAttributes(fileName)) {
		secAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
		secAttr.lpSecurityDescriptor = NULL;
		secAttr.bInheritHandle = true;
		CreateDirectory(fileName, &secAttr);
	}

	wsprintf(fileName,"%s%s%s\\tunnel%04d%02d%02d.log", szDrive, szDir, logdir, syst.wYear, syst.wMonth, syst.wDay);
	
	FILE *fp;
	if ((fp = fopen(fileName, "a")) != NULL) {
		fprintf(fp, "%04d/%02d/%02d %02d:%02d:%02d (%s) %s\n", syst.wYear, syst.wMonth, syst.wDay, syst.wHour, syst.wMinute, syst.wSecond, type, text);
		fclose(fp);
	}
}

void ServerLogOut(const char * text)
{
	char logdir[256];
	GetSetting("logdir", "logs", logdir, sizeof(logdir));
	if (strlen(logdir) == 0) {
		return;
	}

	SYSTEMTIME syst;
	GetLocalTime(&syst);

	char szPath[_MAX_PATH];
	char szDrive[_MAX_DRIVE];
	char szDir[_MAX_DIR];
	char szFileName[_MAX_FNAME];
	char szExt[_MAX_EXT];
	char szOutput[_MAX_PATH * 5 + 1024];
	DWORD dwRet;
	
	//Initialization
	memset(szPath, 0x00, sizeof(szPath));
	memset(szDrive, 0x00, sizeof(szDrive));
	memset(szDir, 0x00, sizeof(szDir));
	memset(szExt, 0x00, sizeof(szExt));
	memset(szOutput, 0x00, sizeof(szOutput));
		
	//Gets the full path name of a running process
	dwRet = GetModuleFileName(NULL, szPath, sizeof(szPath));
	if(dwRet == 0) {
		
	}
		
	//Split the full path name
	_splitpath(szPath, szDrive, szDir, szFileName, szExt); 
	
	TCHAR	fileName[512];
	SECURITY_ATTRIBUTES secAttr;

	wsprintf(fileName,"%s%s%s", szDrive, szDir, logdir);
	
	if (-1 == (int)GetFileAttributes(fileName)) {
		secAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
		secAttr.lpSecurityDescriptor = NULL;
		secAttr.bInheritHandle = true;
		CreateDirectory(fileName, &secAttr);
	}

	wsprintf(fileName,"%s%s%s\\server%04d%02d%02d.log", szDrive, szDir, logdir, syst.wYear, syst.wMonth, syst.wDay);
	
	FILE *fp;
	if ((fp = fopen(fileName, "a")) != NULL) {
		fprintf(fp, "%04d/%02d/%02d %02d:%02d:%02d %s\n", syst.wYear, syst.wMonth, syst.wDay, syst.wHour, syst.wMinute, syst.wSecond, text);
		fclose(fp);
	}
}
