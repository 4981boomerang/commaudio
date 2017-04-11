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
#include "AudioPlayer.h"

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
#define SONG_REQUEST	3

#define MCAST_IP "235.0.0.1"

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

class Network {

public:

	static void CALLBACK completionRoutine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);

	Network(UI*);
	~Network();

	void setIP(std::string ipAddress) { ip = ipAddress; }
	void setPort(int portNum) { port = portNum; }

	bool clientStart();
	void clientStop(bool, bool);

	bool createSocket(int);
	void fillServerInfo(int);
	bool tcpConnect();
	bool tcpRecv();

	bool requestSong(int);
	bool downloadFile();
	bool uploadFile();

	void callbackRoutine(DWORD error, DWORD transferred, DWORD flags);

	//Multicast Functions
	void addToMultiCast(SOCKET&, ip_mreq&);
	void removeFromMultiCast(SOCKET&, ip_mreq&);


	/*--------------------------------------------------------------------------
	-- FUNCTION: startTCPthread
	--
	-- DATE: APR. 09, 2017
	--
	-- REVISIONS: 
	-- Version 1.0 - [EY] - 2016/APR/09 
	--
	-- DESIGNER: Someone from Stack Overflow
	--
	-- PROGRAMMER: Eva Yu
	--
	-- INTERFACE: void  startTCPthread ()
	--  
	--
	-- RETURNS: 
	-- a thread to the function called
	--
	-- NOTES:
	-- inline function that starts a thread by using a lambda 
	-- used to get around initiating class functions 
	-- for threads
	-- this function is for TCP control thread
	--------------------------------------------------------------------------*/
	void startTCPthread() {
		TCPthread = std::thread([this] { this->startTCP(); });
	};


	/*--------------------------------------------------------------------------
	-- FUNCTION: startUDPthread
	--
	-- DATE: APR. 09, 2017
	--
	-- REVISIONS: 
	-- Version 1.0 - [EY] - 2016/APR/09 
	--
	-- DESIGNER: Someone from Stack Overflow
	--
	-- PROGRAMMER: Eva Yu
	--
	-- INTERFACE: void startUDPthread ()
	--  
	--
	-- RETURNS: 
	-- a thread to the function called
	--
	-- NOTES:
	-- inline function that starts a thread by using a lambda 
	-- used to get around initiating class functions 
	-- for threads
	-- this function is for 
	--------------------------------------------------------------------------*/
	void startUDPthread() {
		UDPthread = std::thread( [this] { this->startUDP(); });
	};
	
private:
	void startTCP();
	void startUDP();

	std::thread TCPthread;
	std::thread UDPthread;
	SOCKET tcpSocket, udpSocket;
	sockaddr_in serverAddrTCP, serverAddrUDP;
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
	ip_mreq mreq;
	
	struct Helper {
		WSAOVERLAPPED * ol;
		Network * thisPtr;
	};
	// the sneaky way to get around completion routines 
	Helper help;
};



#endif
