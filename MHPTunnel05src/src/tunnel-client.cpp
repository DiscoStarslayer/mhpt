/**
 * $Id: tunnel-client.cpp,v 1.8 2008/04/29 08:14:56 pensil Exp $
 * Copyright (c) 2008 Pensil - www.pensil.jp
 * 
 * MHP用 トンネルクライアント
 */
#define _BITTYPES_H

#include "tunnel-common.h"

#include <pcap.h>

#include <iostream>
#include <string.h>
#include <conio.h>

using namespace std;


// kBufferSize must be larger than the length of kpcEchoMessage.
const int kBufferSize = 4096;
#define maxiMacAddr 8

pcap_t *adhandle;
char errbuf[PCAP_ERRBUF_SIZE];

// INI設定内容
TCHAR szServer[256];
TCHAR szPort[10];
TCHAR szDevice[256];
TCHAR szNickName[30];

struct USER_INFO {
	clock_t last;
	TCHAR szNickName[30];
};

// 自分からサーバーにトンネルするMACアドレスのリスト
MAC_ADDRESS mac[maxMacAddr];

// トンネルしないMACアドレスのリスト(ループ防止のため)
MAC_ADDRESS ignoreMac[maxiMacAddr];

// サーバーへの接続
SOCKET_EX gsd;

u_long nRemoteAddress;
int nPort;

bool needReconnect = false;

/**
 * 受信したパケットを処理するコールバック関数
 * sdex - 通信内容
 * dh - 受信データヘッダ
 * data - 受信データ
 */
void DoCommand(SOCKET_EX * sdex, DATA_HEADER * dh, char * data)
{
	//printf("%d から '%c'コマンド (%d バイト) 受信。\n", (int)dh.doption, dh.dtype, (int)dh.dsize);
	if (dh->dtype == 't') {

		// Type が 88 C8 なら、自動で4台まで認識する
		int findMac = -1;
		for (int i = 0; i < maxiMacAddr; i++) {
			if (ignoreMac[i].used > 0) {
				if (memcmp((char *)&ignoreMac[i], &data[6], 6) == 0) {
					findMac = i;
					break;
				}
			}
		}
		if (findMac == -1) {
			for (int i = 0; i < maxiMacAddr; i++) {
				if (ignoreMac[i].used == 0) {
					memcpy((char *)&ignoreMac[i], &data[6], 6);
					ignoreMac[i].used = 1;
					printf("遠隔PSP認識(%d): MACアドレス = %0X:%0X:%0X:%0X:%0X:%0X\n", i + 1, ignoreMac[i].addr[0], ignoreMac[i].addr[1], ignoreMac[i].addr[2], ignoreMac[i].addr[3], ignoreMac[i].addr[4], ignoreMac[i].addr[5]);
					break;
				}
			}
		}

		pcap_sendpacket(adhandle, data, (int)dh->dsize);
	} else if (dh->dtype == 'c') {
		data[(int)dh->dsize] = 0;
		printf("%s (S%d->R%d S%d/R%d)\n", data, sdex->sendCount, dh->recvCount, SendPacketSize(sdex), RecvPacketSize(sdex));
    } else if (dh->dtype == 'P') {
    	dh->dtype = 'p';
        if (SendCommand(sdex, dh, data)) {
//			printf("ping応答(%d/%d)\n", sdex->sendCount, sdex->recvCount);
        } else {
//			printf("ping応答失敗\n");
        }
//		printf("ping(S%d/%d R%d/%d)\n", dh->sendCount, sdex->sendCount, dh->recvCount ,sdex->recvCount);
	} else if (dh->dtype == 'p') {
//		data[(int)dh->dsize] = 0;
//		printf("ping(S%d->R%d) (Last: S%d R%d)\n", sdex->sendCount, dh->recvCount, SendPacketSize(sdex), RecvPacketSize(sdex));

	} else if (dh->dtype == 'u') {

		// ユーザー確認コマンドに対する応答
		USER_INFO ui;
		memcpy(&ui.last, data, sizeof(clock_t));
		strcpy(ui.szNickName, szNickName);
		dh->dtype = 'I';
		dh->dsize = (short)sizeof(USER_INFO);
		SendCommand(sdex, dh, (char *)&ui);

	} else if (dh->dtype == 'i') {

		// ユーザー確認コマンドの結果受信
		USER_INFO ui;
		memcpy(&ui, data, sizeof(USER_INFO));
		clock_t now = clock();
		printf("%d: %s (Ping: %d)\n", (int)dh->doption, ui.szNickName, (now - ui.last));

	} else {
		printf("サポート外のコマンド'%c'(%d バイト)\n", dh->dtype, (int)dh->dsize);
		CloseConnection(sdex);
	}
}

void DoClose(SOCKET_EX *)
{
	if (needReconnect == false) {
		return;
	}
	do {
		// 切断時の処理
		if (gsd.state != STATE_WAITTOCONNECT) {
//			printf("再接続します...\n");
		}
	    //gsd = EstablishConnection(&gsd, nRemoteAddress, htons(nPort));
	    if (!EstablishConnection(&gsd, nRemoteAddress, htons(nPort))) {
//	        cerr << WSAGetLastErrorMessage("再接続失敗。") << 
//	                endl;
		}
	    Sleep(10);
	} while (gsd.state != STATE_CONNECTED);
}

/**
 * コンソールへの入力を送信するスレッド(チャット用)
 */ 
DWORD WINAPI ConsoleSender(void *) 
{
	SOCKET_EX * sdex = &gsd;
    //int nRetval = 0;
    char acReadBuffer[kBufferSize];
    int nReadBytes;
 
    DATA_HEADER dh;

	printf("\n");
    while (1) {
		fgets( acReadBuffer, kBufferSize , stdin );  /* 標準入力から入力 */
		if (strcmp(acReadBuffer, "/logout\n") == 0) {
			
			// ログアウトコマンド

			needReconnect = false;

			char sendData[kBufferSize];
			snprintf(sendData, sizeof(sendData), "%s がログアウトしました", szNickName);
			dh.dtype = 'C';
			dh.dsize = (short) strlen(sendData);
			SendCommand(&gsd, &dh, sendData);

			printf("\n切断しています...\n");
			CloseConnection(sdex);
			return 3;
		
		} else if (strcmp(acReadBuffer, "/users\n") == 0) {
			
			// Usersコマンド

			clock_t pingClock = clock();
			char sendData[sizeof(clock_t)];
			
			dh.dtype = 'U';
			dh.dsize = sizeof(int);
			
			memcpy(sendData, (char *)&pingClock, sizeof(clock_t));
			
			printf("接続中のユーザー : \n");
			SendCommand(&gsd, &dh, sendData);

		} else {
	
			// チャット送信

			char sendData[kBufferSize];
			snprintf(sendData, sizeof(sendData), "<%s> %s", szNickName, acReadBuffer);

			dh.dtype = 'C';
			dh.dsize = (short) strlen(sendData) - 1;

//			if (strlen(acReadBuffer) > 1) {
				SendCommand(sdex, &dh, sendData);
//			}
		}
    };
}

/** 
 * WinPcapでキャプチャーしたパケットを処理するコールバック関数
 */
void packet_handler(u_char *, const struct pcap_pkthdr *header, const u_char *pkt_data)
{
	int doSend = -1;
	for (int i = 0; i < maxMacAddr; i++) {
		if (mac[i].used > 0) {
			if (memcmp((char *)&mac[i], &pkt_data[6], 6) == 0) {
				doSend = i;
			}
		}
	}

	if ((int)pkt_data[12] == 136 && (int)pkt_data[13] == 200 && doSend == -1) {

		bool ignore = false;

		// Type が 88 C8 なら、自動で4台まで認識する
		for (int i = 0; i < maxiMacAddr; i++) {
			if (ignoreMac[i].used > 0) {
				if (memcmp((char *)&ignoreMac[i], &pkt_data[6], 6) == 0) {
//		printf("スルー: to %0X:%0X:%0X:%0X:%0X:%0X from %0X:%0X:%0X:%0X:%0X:%0X %d バイト\n", pkt_data[0], pkt_data[1], pkt_data[2], pkt_data[3], pkt_data[4], pkt_data[5], pkt_data[6], pkt_data[7], pkt_data[8], pkt_data[9], pkt_data[10], pkt_data[11], (int)header->len);
					ignore = true;
					break;
				}
			}
		}
		
		if (!ignore) {
			for (int i = 0; i < maxMacAddr; i++) {
				if (mac[i].used == 0) {
					memcpy((char *)&mac[i], &pkt_data[6], 6);
					mac[i].used = 1;
					doSend = i;
					printf("PSP認識(%d): MACアドレス = %0X:%0X:%0X:%0X:%0X:%0X\n", i + 1, mac[i].addr[0], mac[i].addr[1], mac[i].addr[2], mac[i].addr[3], mac[i].addr[4], mac[i].addr[5]);
					break;
				}
			}
		}
	}

	if (doSend > -1) {
		// 送信元がリダイレクト対象でも、送信先までリダイレクト対象ならば、送る必要は無い
		for (int i = 0; i < maxMacAddr; i++) {
			if (mac[i].used > 0) {
				if (memcmp((char *)&mac[i], &pkt_data[0], 6) == 0) {
//					printf("同エリア内送信のためスルー : PSP(%d) -> PSP(%d)\n", doSend + 1, i + 1);
					doSend = -1;
					break;
				}
			}
		}
	}

	if (doSend > -1) {
		
		DATA_HEADER dh;
		dh.dtype = 'T';
		dh.dsize = (short)header->len;

		do {
			if (SendCommand(&gsd, &dh, (char *)pkt_data)) {
//				printf("送信(%d) %d バイト (S%d R%d)\n", doSend, header->len, gsd.sendCount, gsd.recvCount);
			} else {
//				printf("切断したので再接続します。\n");
			    //gsd = EstablishConnection(&gsd, nRemoteAddress, htons(nPort));
//			    if (!EstablishConnection(&gsd, nRemoteAddress, htons(nPort))) {
//			        cerr << WSAGetLastErrorMessage("再接続失敗。") << 
//			                endl;
//				}
//		        Sleep(100);
			}
		} while (gsd.state != STATE_CONNECTED);
	} else {
//		printf("スルー: to %0X:%0X:%0X:%0X:%0X:%0X from %0X:%0X:%0X:%0X:%0X:%0X %d バイト\n", pkt_data[0], pkt_data[1], pkt_data[2], pkt_data[3], pkt_data[4], pkt_data[5], pkt_data[6], pkt_data[7], pkt_data[8], pkt_data[9], pkt_data[10], pkt_data[11], (int)header->len);
	}
}

DWORD WINAPI DoMain(void *)  
{
    printf("MHP Tunnel Client Ver%d.%d by Pensil\n\n", MAJOR_VERSION, MINOR_VERSION);

	GetSetting(_T("Server"), _T(""), szServer, sizeof(szServer));
	GetSetting(_T("Port"), _T("443"), szPort, sizeof(szPort));
	GetSetting(_T("Device"), _T(""), szDevice, sizeof(szDevice));
	GetSetting(_T("NickName"), _T(""), szNickName, sizeof(szNickName));
	
	while (strlen(szNickName) == 0) {
		InputFromUser("ニックネーム: ", szNickName, sizeof(szNickName));
	}
	SetSetting(_T("NickName"), szNickName);

    // Get host and (optionally) port from the command line
    nPort = atoi(szPort);

	pcap_if_t *alldevs;
	pcap_if_t *d;
	int inum;
	int i=0;

	/* デバイスへのハンドルを作成 */
	if ((adhandle= pcap_open_live(szDevice,	// name of the device
							 65536,			// portion of the packet to capture. 
											// 65536 grants that the whole packet will be captured on all the MACs.
							 1,				// promiscuous mode (nonzero means promiscuous)
							 100,			// read timeout
							 errbuf			// error buffer
							 )) == NULL)
	{
	    
		/* 機器リストを得る */
		if(pcap_findalldevs(&alldevs, errbuf) == -1)
		{
			fprintf(stderr,"Error in pcap_findalldevs: %s\n", errbuf);
			exit(1);
		}
	
		/* 機器リストの表示 */
		for(d=alldevs; d; d=d->next)
		{
			printf("%d. %s", ++i, d->name);
			if (d->description)
				printf(" (%s)\n", d->description);
			else
				printf(" (No description available)\n");
		}
		
		if(i==0)
		{
			printf("\nNo interfaces found! Make sure WinPcap is installed.\n");
			return -1;
		}
		
		printf("使用する無線インターフェイスの番号を選択してください (1-%d): ",i);
		char strTmp[32];
		InputFromUser("", strTmp, sizeof(strTmp));
		sscanf(strTmp, "%d", &inum);

		if(inum < 1 || inum > i)
		{
			printf("\nInterface number out of range.\n");
			/* Free the device list */
			pcap_freealldevs(alldevs);
			return -1;
		}
		
		/* Jump to the selected adapter */
		for(d=alldevs, i=0; i< inum-1 ;d=d->next, i++);
		
		/* Open the device */
		/* Open the adapter */
		if ((adhandle= pcap_open_live(d->name,	// name of the device
								 65536,			// portion of the packet to capture. 
												// 65536 grants that the whole packet will be captured on all the MACs.
								 1,				// promiscuous mode (nonzero means promiscuous)
								 100,			// read timeout
								 errbuf			// error buffer
								 )) == NULL)
		{
			fprintf(stderr,"\nUnable to open the adapter. %s is not supported by WinPcap\n", d->name);
			/* Free the device list */
			pcap_freealldevs(alldevs);
			return -1;
		}
			
		SetSetting(_T("Device") , d->name);
		/* At this point, we don't need any more the device list. Free it */
		pcap_freealldevs(alldevs);
	}
	
    // Find the server's address
    bool connected = false;
    do {
    	if (strlen(szServer) > 0) {
		    cout << "名前解決をします..." << flush;
		    nRemoteAddress = LookupAddress(szServer);
		    if (nRemoteAddress == INADDR_NONE) {
		        cerr << endl << WSAGetLastErrorMessage("lookup address") << 
		                endl;
		    } else {
			    in_addr Address;
			    memcpy(&Address, &nRemoteAddress, sizeof(u_long)); 
			    cout << inet_ntoa(Address) << ":" << nPort << endl;
			    
			    // Connect to the server
			    cout << "サーバーに接続中です..." << flush;
			    if (!EstablishConnection(&gsd, nRemoteAddress, htons(nPort))) {
				    cerr << endl << WSAGetLastErrorMessage("接続に失敗しました。") << endl;
			    } else {
			    	connected = true;
			    }
		    }
    	}
    	if (!connected) {
    		while (InputFromUser("接続先サーバーアドレス: ", szServer, sizeof(szServer)) == 0) {
    		}
    	}
    } while (!connected);
    cout << "接続しました。\nローカルポートは " << gsd.sd << " です。" << endl;

	printf("\n ※終了するには /logout と入力してください\n\n");
    
    needReconnect = true;
    SetSetting(_T("Server"), szServer);
    
    Sleep(1);

    DATA_HEADER dh;
	char sendData[kBufferSize];
	snprintf(sendData, sizeof(sendData), "%s がログインしました", szNickName);
	dh.dtype = 'C';
	dh.dsize = (short) strlen(sendData);
	SendCommand(&gsd, &dh, sendData);

    Sleep(100);

	printf("\n接続中のユーザー :\n");
	dh.dtype = 'U';
	dh.dsize = (short) sizeof(clock_t);
	clock_t now = clock();
	memcpy(sendData, (char *)&now, sizeof(clock_t));
	SendCommand(&gsd, &dh, sendData);

    DWORD nThreadID;
    //HANDLE hEchoHandler = CreateThread(0, 0, EchoHandler, (void*)sd, 0, &nThreadID);
    CreateThread(0, 0, ConsoleSender, (void*)&gsd, 0, &nThreadID);
    //gsd = sd;
	
	bool usePcapLoop = false;
	if (usePcapLoop) {

		// WinPcapの pcap_loop を使う方式。
		// どうもSleep(100)が内部に入ってる気がするので、使わない方向で。
		while (1) {
			/* start the capture */
			pcap_loop(adhandle, 0, packet_handler, NULL);
	  	//cout << "切断されました。再接続します。" << endl;
		}

	} else {

		// WinPcapの pcap_next_ex を使う方式
		// Sleep(0)しか入れてないけど大丈夫かな・・・
		int ret;
		const u_char *pkt_data;
		struct pcap_pkthdr *pkt_header;
		while(needReconnect) {
			if(0<=(ret=pcap_next_ex(adhandle,&pkt_header, &pkt_data))){
			    if(ret==1){
			    	//パケットが滞りなく読み込まれた
			    	// この時点で pkt_header->caplen に
			        // 受信バイト数 pkt_data に受信したデータが含まれています。
					packet_handler(NULL, pkt_header, pkt_data);
					Sleep(0);
			    }
			}
		};
	}
	return 0;
}
