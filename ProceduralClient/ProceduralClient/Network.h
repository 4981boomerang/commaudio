#ifndef NETWORK_H
#define NETWORK_H

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCKAPI_
//Windows Headers
#include <winsock2.h>
#include <ws2tcpip.h>
#include <Windows.h>

//C/C++ Headers
#include <iomanip>
#include <fstream>
#include <thread>

#include "UI.h"
#include "CBuff.h"
#include "AudioPlayer.h"
#include "..\..\BoomerangCommAudio\BoomerangCommAudio\Common.h"


constexpr int TCP_PORT = 5000;
constexpr int UDP_PORT = 5001;

#define WM_SOCKET		104
#define PACKET_SIZE		1024
#define MAX_THREADS		3
#define STR_MAX_SIZE	128
#define STR_NAME		128

#define DEFAULT_IP		"192.168.0.36"
#define TEST_FILE		"06 - Little Wing.flac"

#define SONG_UPDATE		1
#define CLIENT_UPDATE	2
#define SONG_DOWNLOAD	3

#define MCAST_IP "235.0.0.1"

typedef struct _common {
	HWND hDlg;
	sockaddr_in serverAddrTCP, serverAddrUDP;
	SOCKET tcpSocket, udpSocket;
	WSABUF sendBufUDP, sendBufTCP, rcvBufUDP, rcvBufTCP;
	WSADATA wsadata;
	DWORD recv;
	DWORD flags;
	OVERLAPPED tcpOL = { 0 }, udpOL = { 0 };
	bool udpRunning, tcpRunning;
	char messageBuffer[PACKET_SIZE];
	bool file, initial = TRUE;
	hostent * hp, * udpHP;
	ip_mreq mreq;
	CBuff cbuff;
	libZPlay::ZPlay * player;
} commonResources;

	void CALLBACK completionRoutineUDP(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);

	bool clientStart(HWND);
	void clientStop(bool, bool);

	bool createSocket(int);
	void fillServerInfo(int);
	bool tcpConnect();
	bool tcpRecv();

	bool requestSong(int);
	bool downloadFile();
	bool uploadFile(HWND);

	//Multicast Functions
	void addToMultiCast(HWND, SOCKET&, ip_mreq&);
	void removeFromMultiCast(HWND, SOCKET&, ip_mreq&);

	void startTCP(HWND hDlg);
	void startUDP(HWND hDlg);
	
	
	void microphoneStart();
	int  __stdcall  myCallbackFunc(void* instance, void *user_data, libZPlay::TCallbackMessage message, unsigned int param1, unsigned int param2);
#endif
