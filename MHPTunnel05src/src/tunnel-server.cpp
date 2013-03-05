/**
 * $Id: tunnel-server.cpp,v 1.7 2008/04/29 08:14:56 pensil Exp $
 * Copyright (c) 2008 Pensil - www.pensil.jp
 * 
 * MHP用 トンネルサーバーソフト
 */
#include "tunnel-common.h"

#include <iostream>

#include <tlhelp32.h>
#include <conio.h>

using namespace std;


const int kBufferSize = 32767;

TCHAR szServer[256];
TCHAR szPort[10];

const int maxOfSessions = 64;
const int maxOfUsers = 12;

struct USER_EX {
	int no;
	char userName[32];
	int session;
	int status;
	int room;
};

SOCKET_EX sessions[maxOfSessions];
USER_EX users[maxOfUsers];

SOCKET SetUpListener(const char* pcAddress, int nPort);
void AcceptConnections(SOCKET ListeningSocket);
bool EchoIncomingPackets(SOCKET_EX* sdex);
int GetIdolSession(void);

/**
 * 空きセッションを取得する
 */
int GetIdolSession(void) {
	for (int i = 0; i < maxOfSessions; i++) {
		if (sessions[i].state == STATE_IDOL) {
			return i;
		}
	}
	return -1;
}

/**
 * リスナーの初期化
 */
SOCKET SetUpListener(const char* pcAddress, int nPort)
{
    u_long nInterfaceAddr = inet_addr(pcAddress);
    if (nInterfaceAddr != INADDR_NONE) {
        SOCKET sd = socket(AF_INET, SOCK_STREAM, 0);
        if (sd != INVALID_SOCKET) {
            sockaddr_in sinInterface;
            sinInterface.sin_family = AF_INET;
            sinInterface.sin_addr.s_addr = nInterfaceAddr;
            sinInterface.sin_port = nPort;
            if (bind(sd, (sockaddr*)&sinInterface, 
                    sizeof(sockaddr_in)) != SOCKET_ERROR) {
                listen(sd, SOMAXCONN);
                return sd;
            }
            else {
                cerr << WSAGetLastErrorMessage("bind() failed") <<
                        endl;
            }
        }
    }

    return INVALID_SOCKET;
}


/**
 * 受信コマンドに対する処理
 */
void DoCommand(SOCKET_EX* sdex, DATA_HEADER * dh, char * data)
{
	static const u_char bc[6] = {0xff,0xff,0xff,0xff,0xff,0xff};

	int findMac;

	//printf("%d から '%c'コマンド (%d バイト) 受信。\n", sdex->number, dh->dtype, (int)dh->dsize);
    if (dh->dtype == 'C') {
		// コマンド C:チャット T:トンネルパケット
		// 受信パケットをそのままリダイレクトする
		dh->doption = (char)sdex->number;
		dh->dtype = 'c';
    
    	// 有効なセッション全部を対象にする
    	for (int i = 0; i < maxOfSessions; i++) {

			// ここの送信ロジックはスレッド化するべきだと思うが・・・
			// してみた
			if (sessions[i].state == STATE_CONNECTED) {

				// コマンド送信
                //if (SendCommandByThread(sessions[i].sd, &dh, acReadBuffer)) {
	            //int nTemp = SendCommand(sessions[i].sd, &dh, data);
	            if (SendCommand(&sessions[i], dh, data)) {
					//printf("%d → %d に転送 (%dバイト)\n", sdex->number, i, (int)dh->dsize);
					//sessions[i].sendSize += nReadBytes;
                } else {
					printf("%d → %d に転送失敗。切断されたかも。 (%dバイト)\n", sdex->number, i, (int)dh->dsize);
					//sessions[i].type = 0;
                    //return false;
                }
	        }
        }

		data[(int)dh->dsize] = 0;
		printf("%d: %s (S%d->R%d) (Last: S%d R%d)\n", (int)dh->doption , data, sdex->sendCount, dh->recvCount, SendPacketSize(sdex), RecvPacketSize(sdex));

    } else if (dh->dtype == 'T') {
		// コマンド T:トンネルパケット
		// 受信パケットをそのままリダイレクトする
		dh->doption = (char)sdex->number;
		dh->dtype = 't';
    
		// 送信元のMACアドレスをリストに追加する
		findMac = -1;
		for (int i = 0; i < maxMacAddr; i++) {
			if (sdex->mac[i].used > 0) {
				if (memcmp((char *)&sdex->mac[i], &data[6], 6) == 0) {
					findMac = i;
					break;
				}
			}
		}
		if (findMac == -1) {
			for (int i = 0; i < maxMacAddr; i++) {
				if (sdex->mac[i].used == 0) {
					memcpy((char *)&sdex->mac[i], &data[6], 6);
					sdex->mac[i].used = 1;
					printf("PSP認識(%d %d): MACアドレス = ", sdex->number, i);
					PrintMac(&sdex->mac[i]);
					printf("\n");
					break;
				}
			}
		}

    	// 有効なセッション全部を対象にする
    	for (int i = 0; i < maxOfSessions; i++) {

			// ここの送信ロジックはスレッド化するべきだと思うが・・・
			// してみた
			if (sessions[i].state == STATE_CONNECTED && sessions[i].number != sdex->number) {

				bool yesSend = false;

				// まず、ブロードキャスト送信をリダイレクトする
				if (memcmp(&bc, data, 6) == 0) {
					yesSend = true;
				} else {
					
					// 送信先のMACアドレスをリストに存在するか確認する
					for (int m = 0; m < maxMacAddr; m++) {
						if (sessions[i].mac[m].used > 0) {
							if (memcmp((char *)&sessions[i].mac[m], data, 6) == 0) {
								yesSend = true;
								break;
							}
						}
					}
				}
				
				// コマンド送信
                //if (SendCommandByThread(sessions[i].sd, &dh, acReadBuffer)) {
	            //int nTemp = SendCommand(sessions[i].sd, &dh, data);
	            if (yesSend) {
		            if (SendCommand(&sessions[i], dh, data)) {
//						printf("%d → %d に転送 (%dバイト)\n", sdex->number, i, (int)dh->dsize);
						//sessions[i].sendSize += nReadBytes;
	                } else {
						printf("%d → %d に転送失敗。切断されたかも。 (%dバイト)\n", sdex->number, i, (int)dh->dsize);
						//sessions[i].type = 0;
	                    //return false;
	                }
	            }
	        }
        }

    } else if (dh->dtype == 'U') {

		// コマンド U:ユーザー一覧確認コマンド
		// 受信パケットをそのままリダイレクトする
		dh->doption = (char)sdex->number;
		dh->dtype = 'u';
    
    	// 有効なセッション全部を対象にする
    	for (int i = 0; i < maxOfSessions; i++) {

			// ここの送信ロジックはスレッド化するべきだと思うが・・・
			// してみた
			if (sessions[i].state == STATE_CONNECTED) {

	            SendCommand(&sessions[i], dh, data);
	        }
        }

    } else if (dh->dtype == 'I') {

		// コマンド U:ユーザー一覧確認コマンド
		// 受信パケットをそのままリダイレクトする
		int i = (int)dh->doption;
		dh->dtype = 'i';
		dh->doption = (char)sdex->number;
		
		if (sessions[i].state == STATE_CONNECTED) {
			SendCommand(&sessions[i], dh, data);
        }

    } else if (dh->dtype == 'P') {
    	dh->dtype = 'p';
        if (SendCommand(sdex, dh, data)) {
//			printf("ping応答(%d/%d)\n", sdex->sendCount, sdex->recvCount);
        } else {
//			printf("ping応答失敗\n");
        }
	} else if (dh->dtype == 'p') {
		data[(int)dh->dsize] = 0;
		printf("%d : ping(S%d->R%d S%d->R%d)\n", sdex->number , sdex->sendCount, dh->recvCount, dh->sendCount ,sdex->recvCount);
    } else {
		printf(" → サポート外のコマンド [%c]\n", dh->dtype);
        //return false;
    }
}

void DoClose(SOCKET_EX * sdex)
{
	DATA_HEADER dh;
	char data[256];
	
	snprintf(data, sizeof(data), "%d が切断しました", sdex->number);
	dh.dtype = 'c';
	dh.dsize = (short)strlen(data);

	// 有効なセッション全部を対象にする
	for (int i = 0; i < maxOfSessions; i++) {

		if (sessions[i].state == STATE_CONNECTED) {

			// コマンド送信
            //if (SendCommand(&sessions[i], &dh, data)) {
            //}
        }
	}
}

DWORD WINAPI DoMain(void *)  
{
    printf("MHP Tunnel Server Ver%d.%d by Pensil\n\n", MAJOR_VERSION, MINOR_VERSION);

	GetSetting(_T("Server"), _T(""), szServer , 256);
	GetSetting(_T("Port") , _T("443"), szPort , 10);

	// ポート番号
    int nPort = atoi(szPort);

    // セッションリストの初期化
	for (int i = 0; i < maxOfSessions; i++) {
		sessions[i].sd = INVALID_SOCKET;
		sessions[i].state = STATE_IDOL;
		sessions[i].number = i;
	}

    // リスナーの初期化
    printf("リスナーを初期化しています...\n");
    SOCKET ListeningSocket = SetUpListener(szServer, htons(nPort));
    if (ListeningSocket == INVALID_SOCKET) {
        printf("\n%s\n", WSAGetLastErrorMessage("リスナー初期化時エラー"));
        return 3;
    }

	// ひたすら接続待ち
    while (1) {
	    printf("接続を待っています...\n");
		int i = GetIdolSession();
		while(i == -1) {
			// アイドルセッションがないので 5秒待ってみる
			Sleep(5000);
			i = GetIdolSession();
		};
		if (AcceptConnection(&sessions[i], ListeningSocket)) {
			printf("接続セッション番号 : %d\n", i);
            cout << "クライアント " <<
                    inet_ntoa(sessions[i].sinRemote.sin_addr) << ":" <<
                    ntohs(sessions[i].sinRemote.sin_port) << "." <<
                    endl;
        }
        else {
        	printf("%s", WSAGetLastErrorMessage("accept() failed"));
        }
    }
}
