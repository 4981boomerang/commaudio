#ifndef NETWORK_H
#define NETWORK_H

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCKAPI_
//Windows Headers
#include <winsock2.h>
#include <Windows.h>

//C/C++ Headers
#include <iomanip>
#include <fstream>
#include <thread>

#include "UI.h"
#include "AudioPlayer.h"

constexpr int TCP_PORT = 5000;
constexpr int UDP_PORT = 5001;

#define WM_SOCKET		104
#define PACKET_SIZE		1024
#define MAX_THREADS		3
#define STR_MAX_SIZE	128
#define STR_NAME		128

#define DEFAULT_IP		"192.168.0.22"
#define TEST_FILE		"06 - Little Wing.flac"

#define SONG_UPDATE		1
#define CLIENT_UPDATE	2
#define SONG_REQUEST	3

typedef struct {
	int header;
	union {
		int numOfSongs;
		int numOfClients;
		int SID;
	};
} ControlMessage;

typedef struct {
	int header;
	char username[STR_NAME];
	char ip[STR_NAME];
} ClientData;

typedef struct {
	int header;
	int SID;
	char title[STR_MAX_SIZE];
	char artist[STR_MAX_SIZE];
} SongData;

void CALLBACK compRoutine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);

class Network {

public:
	Network(UI*);
	~Network();

	void setIP(std::string ipAddress) { ip = ipAddress; }
	void setPort(int portNum) { port = portNum; }

	bool clientStart();
	void clientStop(bool, bool);

	void startTCP();
	void startUDP();

	bool createSocket(int);
	void fillServerInfo(int);
	bool tcpConnect();
	bool tcpRecv();

	bool requestSong(int);
	bool downloadFile();
	bool uploadFile();


private:
	//0 --> TCP | 1 --> UDP
	SOCKET tcpSocket, udpSocket;
	sockaddr_in serverAddr;
	WSAOVERLAPPED tcpOL, udpOL;
	WSADATA wsadata;
	WSABUF sendBufUDP, sendBufTCP, rcvBufUDP, rcvBufTCP;
	WORD wVersionRequested;
	bool udpRunning, tcpRunning;
	char * dest;
	char messageBuffer[PACKET_SIZE];
	hostent * hp;
	std::string ip;
	int port;
	UI * ui;
};



#endif
