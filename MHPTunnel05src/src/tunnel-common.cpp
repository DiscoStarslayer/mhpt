/**
 * $Id: tunnel-common.cpp,v 1.9 2008/04/29 08:14:52 pensil Exp $
 * Copyright (c) 2008 Pensil - www.pensil.jp
 * 
 * サーバー/クライアント 共通コンポーネント
 */

#include "tunnel-common.h"

#include <iostream>
#include <algorithm>
#include <strstream>
#include <windows>
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

HWND hMainWnd;


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
    ostrstream outs(acErrorBuffer, sizeof(acErrorBuffer));
    outs << pcMessagePrefix << ": ";

    // Tack appropriate canned message onto end of supplied message 
    // prefix. Note that we do a binary search here: gaErrorList must be
	// sorted by the error constant's value.
	ErrorEntry* pEnd = gaErrorList + kNumMessages;
    ErrorEntry Target(nErrorID ? nErrorID : WSAGetLastError());
    ErrorEntry* it = lower_bound(gaErrorList, pEnd, Target);
    if ((it != pEnd) && (it->nID == Target.nID)) {
        outs << it->pcMessage;
    }
    else {
        // Didn't find error in list, so make up a generic one
        outs << "unknown error";
    }
    outs << " (" << Target.nID << ")";

    // Finish error message off and return it.
    outs << ends;
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

// 残り受信パケットサイズ
int RecvPacketSize(SOCKET_EX *sdex)
{
	//sdex->recvWritePoint : 受信アドレス
	//sdex->recvReadPoint : 取得アドレス
	// 残りサイズ = sdex->recvWritePoint - sdex->recvReadPoint
	int size = sdex->recvWritePoint - sdex->recvReadPoint;
	if (size < 0) {
		// 0以下ということは、バッファ終端を越えているということ
		size += sdex->recvBufferSize;
	}
    return size;
}

// 残り送信パケットサイズ
int SendPacketSize(SOCKET_EX *sdex)
{
	//sdex->recvWritePoint : 受信アドレス
	//sdex->recvReadPoint : 取得アドレス
	// 残りサイズ = sdex->recvWritePoint - sdex->recvReadPoint
	int size = sdex->sendWritePoint - sdex->sendReadPoint;
	if (size < 0) {
		// 0以下ということは、バッファ終端を越えているということ
		size += sdex->sendBufferSize;
	}
    return size;
}

// 残り受信コマンドサイズ
void SendBufferCopy(SOCKET_EX *sdex, char *data, int start_, int size)
{
//	printf("SendBufferCopy きました from %d  size %d\n", start_, size);
	if (size < 1) {
		return;
	}
	int start = start_;
	while (start > sdex->sendBufferSize) {
		start = start - sdex->sendBufferSize;
	}
	// バッファの終端の切れ目にさしかかるかどうか
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

// 残り受信コマンドサイズ
void RecvBufferCopy(SOCKET_EX *sdex, char *data, int start_, int size)
{
	int start = start_;
	while (start > sdex->recvBufferSize) {
		start = start - sdex->recvBufferSize;
	}
	// バッファの終端の切れ目にさしかかるかどうか
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

// 残り送信バッファの移動
void SendBufferMove(SOCKET_EX *sdex, int size)
{
	int newPoint = sdex->sendWritePoint + size;
	if (newPoint > sdex->sendBufferSize) {
		newPoint = newPoint - sdex->sendBufferSize;
	}
//	printf("新しい書き込みポイントは %d (%dプラス)\n", newPoint, size);
	sdex->sendWritePoint = newPoint;
}

// 残り受信バッファの移動
void RecvBufferMove(SOCKET_EX *sdex, int size)
{
	int newPoint = sdex->recvReadPoint + size;
	if (newPoint > sdex->recvBufferSize) {
		newPoint = newPoint - sdex->recvBufferSize;
	}
//	printf("新しい読み込みポイントは %d\n", newPoint);
	sdex->recvReadPoint = newPoint;
}

// 残り送信コマンドサイズ
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

// 残り受信コマンドサイズ
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
			// 0以下ということは、バッファ終端を越えているということ
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
		
		// チェックポイントがバッファの終端にさしかかっているかどうか
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
	
	// バッファサイズを超えるコマンド
	if (size > maxSize) {
		printf("致命的エラー:サイズ上限を超えたコマンドを受信しました。(%d > %d)このコマンドをスルーします。\n", size, maxSize);
		// 読み込みポインタを移動
//		RecvBufferMove(sdex, size + sizeof(eof));
		RecvBufferMove(sdex, size);
		LeaveCriticalSection(&sdex->recvSection);
		return false;
	}
	
	// ヘッダーを取得
	RecvBufferCopy(sdex, (char *)hd, sdex->recvReadPoint, sizeof(DATA_HEADER));

	if (size > sizeof(DATA_HEADER)) {
		// データを取得
		RecvBufferCopy(sdex, data, sdex->recvReadPoint + sizeof(DATA_HEADER), size - sizeof(DATA_HEADER) - sizeof(eof));
		hd->dsize = (short)(size - sizeof(DATA_HEADER) - sizeof(eof));
	}

	// 読み込みポインタを移動
//	RecvBufferMove(sdex, size + sizeof(eof));
	RecvBufferMove(sdex, size);
	//printf("355:LeaveCriticalSection\n");

	if (sdex->sendCountBefore + 1 != hd->sendCount && sdex->sendCountBefore != 0) {
		printf("LostSendCount! %d - %d = %d\n", hd->sendCount, sdex->sendCountBefore + 1, hd->sendCount - sdex->sendCountBefore - 1);
		sdex->sendCountBefore = 0;
		printf("エラーのため一度再接続します\n");
		LeaveCriticalSection(&sdex->recvSection);
		CloseConnection(sdex);
		return false;
//		CloseConnection(sdex);
	}
	if (sdex->sendCountBefore == hd->sendCount && sdex->sendCountBefore != 0) {
		sdex->sendCountBefore = 0;
		printf("エラーのため一度再接続します\n");
		CloseConnection(sdex);
		LeaveCriticalSection(&sdex->recvSection);
		return false;
	}
	sdex->sendCountBefore = hd->sendCount;
	sdex->recvCount++;

	LeaveCriticalSection(&sdex->recvSection);
	return true;
}

// 指定したヘッダーつきのコマンドを送信する
// return 0 : 送信失敗
// return SOCKET_ERROR : ソケットエラー
// return 0以上 : トータル送信パケット数
bool SendCommand(SOCKET_EX *sdex, DATA_HEADER *hd, char *data)
{
	if (sdex->state != STATE_CONNECTED) {
		return false;
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
		// 受信バッファにコマンドがある限り繰り返す
		DoCommand(sdex, &dh, data);
	}
	return false;
}

int RecvToBuffer(SOCKET_EX * sdex)
{
	int last = sdex->recvBufferSize - sdex->recvWritePoint;
	int start = sdex->recvWritePoint;
	if (last == 0) {
		// 読み込みバッファが一巡したら、バッファの最初から読み込む
		start = 0;
		last = sdex->recvBufferSize;
	}
    //printf("受信バッファ残り : %d バイト\n", sdex->recvBufferSize - sdex->recvWritePoint);

	// 受信クリティカルセクションも1つとする
	// と、思ったら受信スレッドは1つしかないから競合するわけないじゃん
	// クリティカルセクション使う意味無し。
	// よってやめる
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
		// TODO: 書き込みポイントが読み込みポイントを超えた場合
		// バッファオーバーフローエラーを出すべきだができてない
		
		// どうも受信して直後にパケットおくるのがまずいのかな?
		//printf("recv(): SUCCESS!\n");
	} else if (nTemp == SOCKET_ERROR) {
		if(WSAGetLastError() != WSAEWOULDBLOCK){
			sdex->recvState = -1;
			sdex->recvLast = clock();
			sdex->state = STATE_ERROR;
			printf("%d:受信時エラー切断しました(SOCKET_ERROR)\n", sdex->number);
			//CloseConnection(sdex);
        } else {
			//printf("recv(): WSAEWOULDBLOCK!\n");
			//return 0;
		}
    } else {
		//sdex->state = STATE_ERROR;
		printf("%d:受信時エラー切断しました(送信バイト0)\n", sdex->number);
		//CloseConnection(sdex);
    	// タイムアウトした可能性有り。
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
	
		// 受信クリティカルセクションも1つとする
		// と、思ったら受信スレッドは1つしかないから競合するわけないじゃん
		// クリティカルセクション使う意味無し。
		// よってやめる
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
			// TODO: 書き込みポイントが読み込みポイントを超えた場合
			// バッファオーバーフローエラーを出すべきだができてない
			
			// どうも受信して直後にパケットおくるのがまずいのかな
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
				printf("%d:送信時エラー切断しました(SOCKET_ERROR)\n", sdex->number);
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
			printf("%d:送信時エラー切断しました(送信バイト0)\n", sdex->number);
			//CloseConnection(sdex);
	    	// タイムアウトした可能性有り。
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
 *  イベントを使用してパケットを送受信するスレッド
 */
DWORD WINAPI RecvPacketThread(void * sdex_) 
{
	SOCKET_EX * sdex = (SOCKET_EX *)sdex_;
	
    int nRet;
    HANDLE  hEvent = WSACreateEvent();
    WSANETWORKEVENTS    events;

    WSAEventSelect(sdex->sd,hEvent,FD_READ|FD_WRITE|FD_CLOSE);

    while(sdex->state == STATE_CONNECTED)
    {
        /*  イベント待ち                */
        nRet = WSAWaitForMultipleEvents(1,&hEvent,FALSE,WSA_INFINITE,FALSE);

        if(nRet == WSA_WAIT_FAILED)
        {
            break;
        }

        /*  イベントの調査              */
        if(WSAEnumNetworkEvents(sdex->sd,hEvent,&events) == SOCKET_ERROR)
        {
        	// なんかエラーでた
			printf("SOCKET_ERROR on RecvEvent\n");
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

    WSACloseEvent(hEvent);                  /*  イベントクローズ            */

    CloseConnection(sdex);
    return 0;
}

// Ping送信スレッド
DWORD WINAPI PingAction(void * sdex_) 
{
	DATA_HEADER dh;
	char data;
	dh.dsize = 0;
	dh.dtype = 'P';
	SOCKET_EX * sdex = (SOCKET_EX *)sdex_;
	while (sdex->state == STATE_CONNECTED) {
		Sleep(3000);
		if (sdex->recvLast + 1000 < clock()) {
			SendCommand(sdex, &dh, &data);
		}
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
	
	if (sdex->hWnd == NULL) {
//		InitTunnelWindow(sdex);
//	    CreateThread(0, 0, TunnelMessageReader, (void*)sdex, 0, NULL);
	}

	for (int i = 0; i < maxMacAddr; i++) {
		sdex->mac[i].used = 0;
	}
	
	return true;
}

bool StartConnection(SOCKET_EX *sdex)
{
	int iOptVal;
	int iOptLen = sizeof(int);
	if (getsockopt(sdex->sd, SOL_SOCKET, SO_KEEPALIVE, (char*)&iOptVal, &iOptLen) != SOCKET_ERROR) {
		//printf("SO_KEEPALIVE Value: %ld\n", iOptVal);
	    if (setsockopt(sdex->sd, IPPROTO_TCP, TCP_NODELAY, (char*)&iOptVal, iOptLen) != 0) {
			printf("TCP_NODELAY失敗\n");
	    }
	}
		
	/* 接続、送信、受信、切断のイベントをウィンドウメッセージで通知されるようにする */
//	if(WSAAsyncSelect(sdex->sd, sdex->hWnd, WSOCK_SELECT, FD_CONNECT | FD_WRITE | FD_READ | FD_CLOSE) == SOCKET_ERROR){
//		printf("ソケットイベントを通知させる設定に失敗しました。\n");
//	}

    sdex->sendCountBefore = 0;
	sdex->state = STATE_CONNECTED;
    sdex->sendThreadHandle = CreateThread(0, 0, PingAction, (void*)sdex, 0, &sdex->sendThreadId);
    sdex->recvThreadHandle = CreateThread(0, 0, RecvPacketThread, (void*)sdex, 0, &sdex->recvThreadId);
    //CreateThread(0, 0, PingAction, (void*)sdex, 0, NULL);
	return true;
}

bool CloseConnection(SOCKET_EX *sdex)
{
	SOCKET_EX closedsdex;
	
	if (sdex->state == STATE_IDOL) {
		return true;
	}

	// 切断処理コールバック関数を起動
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
	//free(sdex->recvBuffer); // 受信バッファを消すと落ちるようだ
	sdex->state = STATE_IDOL;

	DoClose(&closedsdex);
	return true;
}

// dwMessageIdのメッセージを取得し、hMessageに内容を返す。
BOOL formatmessage(DWORD dwMessageId, HLOCAL* hMessage){
    BOOL bRet=FALSE;
    //DWORD dwLen=0UL;
    DWORD dwLen;
    //DWORD cb=0UL;
    //DWORD cbNeeded=0UL;
    //DWORD dwEntries=0UL;
    HANDLE hProcess=NULL;
    HMODULE hModules[4096];

    try{
        // まずシステムから検索
        dwLen=::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
                            FORMAT_MESSAGE_ALLOCATE_BUFFER,
                            NULL,
                            dwMessageId,
                            MAKELANGID(LANG_NEUTRAL, 
                            SUBLANG_SYS_DEFAULT),
                            (LPTSTR)hMessage,
                            0,
                            NULL);
        if (dwLen)
            return TRUE;

    }catch(BOOL b){
        bRet=b;
    }

    // 開放
    if (hProcess) ::CloseHandle(hProcess); 

    return bRet;
}

// 内容をMessageBoxで表示。
BOOL formatmessagebox(HWND hWnd, DWORD dwMessageId){
    BOOL bRet=FALSE;
    HLOCAL hMessage=NULL;

    // メッセージを取得
    if (formatmessage(dwMessageId, &hMessage)){
        // MessageBox表示
        ::MessageBox(hWnd, (LPTSTR)hMessage, _T("ERROR"), MB_OK|MB_ICONSTOP);
        hMessage=::LocalFree(hMessage);
        if (!hMessage) bRet=TRUE;
    }

    return bRet;
}

// 内容をConsoleに表示。
BOOL formatconsole(DWORD dwMessageId){
    BOOL bRet=FALSE;
    HLOCAL hMessage=NULL;

    // メッセージを取得
    if (formatmessage(dwMessageId, &hMessage)){
        // Console表示
        printf("ERROR: %s\n", (char *)hMessage);
        hMessage=::LocalFree(hMessage);
        if (!hMessage) bRet=TRUE;
    }

    return bRet;
}

// CUIで作る場合の処理ルーチン
#if !defined(USE_WINMAIN) 
int main()
{
    // Start Winsock up
    WSAData wsaData;
	int nCode;
    if ((nCode = WSAStartup(MAKEWORD(2, 0), &wsaData)) != 0) {
		cerr << "WSAStartup()関数がエラーを返しました。エラーコードは " << nCode << " です。Windowsのバージョンが古い?" <<
				endl;
        return 255;
    }

	int result = DoMain(NULL);

    // Shut Winsock back down and take off.
//	printf("WSACleanup\n");
    WSACleanup();

	return result;
}
#endif // #if !defined(USE_WINMAIN) 

// WinMainで作る場合のメイン処理ルーチン
#if defined(USE_WINMAIN) 

// ウインドウメッセージでソケットの送受信を行う処理だったが、
// Eventを使う方式にしたので使うのをやめた。
LRESULT CALLBACK WndProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int nSize;

	switch (uMsg)
	{
		case WM_CREATE:
			printf("WM_CREATE \n");
			return 0;
		
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;	

	    default:
	        return false;
	}

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int PASCAL _WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) 
{
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONIN$", "r", stdin);

	//static TCHAR      szDefaultClassName[] = TEXT("Default");
	static TCHAR      szClassName[] = TEXT("MHP2GTunnel");
	MSG        msg;
	//WNDCLASSEX wc = {0};

    // ウィンドウクラスの登録
    WNDCLASSEX wc;
    wc.cbClsExtra = 0;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.cbWndExtra = 0;
    wc.hbrBackground = (HBRUSH)::GetStockObject(BLACK_BRUSH);
    wc.hCursor = ::LoadCursor(NULL,MAKEINTRESOURCE(IDC_ARROW));
    wc.hIcon = NULL;
    wc.hIconSm = NULL;
    wc.hInstance = hInstance;
    //wc.lpfnWndProc = TunnelWndProc;
    wc.lpfnWndProc = DefWindowProc;
    wc.lpszClassName = szClassName;
    wc.lpszMenuName = NULL;
    wc.style = CS_HREDRAW|CS_VREDRAW;

	// ウインドウクラスの登録
    if (RegisterClassEx(&wc) == NULL) {
		MessageBox(NULL, "RegisterClassEx でエラーが発生しました", _T("ERROR"), MB_OK|MB_ICONSTOP);
		formatmessagebox(NULL, GetLastError());
    	return 0;
    }

    // ウィンドウの表示
    hMainWnd = ::CreateWindow(szClassName,szClassName,
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, HWND_MESSAGE, NULL, hInstance, NULL);
    if ( hMainWnd==NULL ) {
		MessageBox(NULL, "CreateWindowでエラーが発生しました", _T("ERROR"), MB_OK|MB_ICONSTOP);
		formatmessagebox(NULL, GetLastError());
        return 0;
    }

	// メッセージ処理を行うプロシージャを定義する
	if (SetWindowLong(hMainWnd, GWL_WNDPROC, (long)WndProc) == 0) {
		MessageBox(NULL, "SetWindowLongでエラーが発生しました", _T("ERROR"), MB_OK|MB_ICONSTOP);
		formatmessagebox(NULL, GetLastError());
        return 0;
	}
	// ユーザーデータに sdex を入れる
	if (SetWindowLong(hMainWnd, GWL_USERDATA, -1) == 0) {
		int err = GetLastError();
		if (err != 0) {
			MessageBox(NULL, "SetWindowLong(GWL_USERDATA)でエラーが発生しました", _T("ERROR"), MB_OK|MB_ICONSTOP);
			formatmessagebox(NULL, err);
	        return false;
		}
	}

    // Start Winsock up
	printf("Start Winsock\n");
    WSAData wsaData;
	int nCode;
    if ((nCode = WSAStartup(MAKEWORD(2, 0), &wsaData)) != 0) {
		cerr << "WSAStartup()関数がエラーを返しました。エラーコードは " << nCode << " です。Windowsのバージョンが古い?" <<
				endl;
        return 255;
    }

	//printf("CreateThread\n");
    CreateThread(0, 0, DoMain, NULL, 0, NULL);
	
	//printf("Message Loop\n");
	// メッセージループ
	while (GetMessage(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
    // Shut Winsock back down and take off.
	//printf("WSACleanup\n");
    WSACleanup();

	return (int)msg.wParam;
}
#endif // #if defined(USE_WINMAIN) 

/**
 * 設定ファイルを読み込む
 */ 
void GetSetting(char* key, char* defValue, char* buffer, int size)
{
	char	cur[512];
	TCHAR	* fileName = "ini";
	if ( GetModuleFileName(NULL, cur, sizeof(cur)) == 0 ) {
		// エラー
	}
	strcpy(&cur[strlen(cur) - 3], fileName);
	
	GetPrivateProfileString( 
		_T("Setting") , key , defValue ,
		buffer , size , cur );
}

void SetSetting(char* key, char* value)
{
	char	cur[512];
	TCHAR	* fileName = "ini";
	if ( GetModuleFileName(NULL, cur, sizeof(cur)) == 0 ) {
		// エラー
	}
	strcpy(&cur[strlen(cur) - 3], fileName);
	
	WritePrivateProfileString( _T("Setting") , key , value ,cur );
}

void GetMacSetting(char* key, MAC_ADDRESS* mac)
{
	TCHAR szMac[20];
	int im[6];

	for (int i = 0; i < 6; i++) im[i] = 0;

	GetSetting( key , _T("00:00:00:00:00:00") , szMac , 20);

	sscanf(szMac, "%X:%X:%X:%X:%X:%X", &im[0], &im[1], &im[2], &im[3], &im[4], &im[5]);
	for (int i = 0; i < 6; i++) {
		mac->addr[i] = (u_char)im[i];
		mac->used += im[i];
	}
}

void PrintMac(MAC_ADDRESS* mac)
{
	printf("%0X:%0X:%0X:%0X:%0X:%0X", mac->addr[0], mac->addr[1], mac->addr[2], mac->addr[3], mac->addr[4], mac->addr[5]);
}

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
