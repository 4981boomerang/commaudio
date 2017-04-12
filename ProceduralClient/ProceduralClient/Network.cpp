#include "Network.h"

commonResources common;

/*---------------------------------------------------------------------------------
-- FUNCTION:	  clientStart
--
-- DATE:		  April 3, 2017
--
-- DESIGNER:	  Michael Goll
--
-- DEVELOPER:     Michael Goll
--
-- INTERFACE:     bool clientStart()
--
-- RETURNS:       bool - Whether or not WSA was started up successfully.
--
-- REVISIONS:
--
-- NOTES:         Starts WSAStartup to use network functionality.
-----------------------------------------------------------------------------------*/
bool clientStart(HWND hDlg) {
	WORD wVersionRequested = MAKEWORD(2, 2);
	WSADATA wsadata = { 0 };

	common.wsadata = wsadata;

	if ((WSAStartup(wVersionRequested, &common.wsadata)) == -1) {
		//show user there is an error
		showMessageBox(hDlg, "Cannot start WSAStartup", "WSAStartup Error", MB_ICONERROR);
		int errorNo = GetLastError();
		return FALSE;
	}
}

/*---------------------------------------------------------------------------------
-- FUNCTION:	  clientStop
--
-- DATE:		  April 3, 2017
--
-- DESIGNER:	  Michael Goll
--
-- DEVELOPER:     Michael Goll
--
-- INTERFACE:     void clientStop(bool stopTCP, bool stopUDP)
--
-- PARAMETER:     bool stopTCP - A bool flag that specifies whether or not to stop
--                               the TCP side.
--
--                bool stopUDP - A bool flag that specifies whether or not to stop
--                               the UDP thread.
--
-- RETURNS:       void
--
-- REVISIONS:
--
-- NOTES:         Stops either the TCP or UDP thread, or both.
-----------------------------------------------------------------------------------*/
void clientStop(bool stopTCP, bool stopUDP) {
	if (stopTCP)
		common.tcpRunning = FALSE;

	if (stopUDP)
		common.udpRunning = FALSE;
}

/*--------------------------------------------------------------------------
-- FUNCTION: callbackRoutine
--
-- DATE: APR. 09, 2017
--
-- REVISIONS: 
-- Version 1.0 - [MG] - 2016/APR/09 - DESCRIPTION
-- Version 1.0 - [EY] - 2016/APR/09 - Made it class function 
--
-- DESIGNER: Mike Goll
--
-- PROGRAMMER: Mike Goll
--
-- INTERFACE: returntype callbackRoutine (DWORD error, DWORD transferred, DWORD flags)
-- DWORD error -- the error value
-- DWORD transferred -- number of bytes transderref
-- DWORD flags -- flags assocated with wsarecv
--
-- NOTES:
-- callback function for handling the wsarecv
--------------------------------------------------------------------------*/
void CALLBACK completionRoutineUDP(DWORD error, DWORD transferred, LPWSAOVERLAPPED ol, DWORD flags)
{
	int retVal;
	DWORD bRecv;

	switch (error) {
		//no error here
	case 0:
		//push data received into the circular buffer
		common.cbuff.push_back(common.rcvBufUDP.buf);

		//register again
		if ((retVal = WSARecvFrom(common.udpSocket, &common.rcvBufUDP, 1, &bRecv, &flags, 0, 0, &common.udpOL, completionRoutineUDP)) == SOCKET_ERROR) {
			retVal = WSAGetLastError();

			if (retVal != WSA_IO_PENDING) {
				clientStop(FALSE, TRUE);
				closesocket(common.udpSocket);
				return;
			}
		}
		break;
		

	case WSA_OPERATION_ABORTED:
		clientStop(FALSE, TRUE);
		break;

	default:
		break;
	}
}

/*---------------------------------------------------------------------------------
-- FUNCTION:	  startTCP
--
-- DATE:		  April 7, 2017
--
-- DESIGNER:	  Michael Goll
--
-- DEVELOPER:     Aing Ragunathan, Michael Goll
--
-- INTERFACE:     void startTCP()
--
-- RETURNS:       void
--
-- REVISIONS:
--
-- NOTES:         Runs through the TCP setup processes.
-----------------------------------------------------------------------------------*/
void startTCP(HWND hDlg) {
	hostent * hp;
	int errorReturn;

	//dns query, an IP address would return itself
	if (!(hp = gethostbyname(getDestination(hDlg)))) {
		errorReturn = GetLastError();
		return;
	}

	common.hp = hp;

	if (!createSocket(0)) {
		showMessageBox(hDlg, "Cannot create TCP socket.", "TCP Socket Creation Error", MB_ICONERROR);
		errorReturn = WSAGetLastError();
		return;
	}

	common.serverAddrTCP.sin_family = AF_INET;
	common.serverAddrTCP.sin_port = htons(TCP_PORT);
	memcpy((char *)&common.serverAddrTCP.sin_addr, common.hp->h_addr, common.hp->h_length);

	if (!tcpConnect()) {
		showMessageBox(hDlg, "Cannot connect to the server.", "Connection Error", MB_ICONERROR);
		return;
	}

	swapButtons(hDlg, IDC_CONNECT, IDC_DISCONNECT);
	SetDlgItemText(hDlg, IDC_EDIT1, "Connected");
}

/*---------------------------------------------------------------------------------
-- FUNCTION:	  startUDP
--
-- DATE:		  April 7, 2017
--
-- DESIGNER:	  Michael Goll
--
-- DEVELOPER:     Aing Ragunathan, Michael Goll
--
-- INTERFACE:     void startTCP()
--
-- RETURNS:       void
--
-- REVISIONS:
--
-- NOTES:         Runs through the UDP setup processes.
-----------------------------------------------------------------------------------*/
void startUDP(HWND hDlg) {

	int retVal;
	DWORD flags = MSG_PARTIAL, recv;
	hostent * hp;
	common.udpRunning = TRUE;
	char * buf = { 0 };

	clientStart(common.hDlg);
	
	//dns query, an IP address would return itself
	if (!(hp = gethostbyname(getDestination(hDlg)))) {
		retVal = GetLastError();
		return;
	}

	common.udpHP = hp;

	if (!createSocket(1)) {
		showMessageBox(hDlg, "Cannot create UDP socket.", "UDP Socket Creation Error", MB_ICONERROR);
		return;
	}

	if ((retVal = setsockopt(common.udpSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&flags, sizeof(flags))) == -1) {
		showMessageBox(hDlg, "Error setting socket option.", "Set Socket Option Error", MB_ICONERROR);
		closesocket(common.udpSocket);
		return;
	}

	common.serverAddrUDP.sin_family = AF_INET;
	common.serverAddrUDP.sin_port = htons(5001);
	common.serverAddrUDP.sin_addr.s_addr = htonl(INADDR_ANY);

	if ((retVal = bind(common.udpSocket, (SOCKADDR *)&common.serverAddrUDP, sizeof(SOCKADDR_IN))) == -1) {
		retVal = WSAGetLastError();
		closesocket(common.udpSocket);
		showMessageBox(hDlg, "Cannot Bind Socket, Aborting...", "Socket Bind Error", MB_OK | MB_ICONERROR);
		return;
	}

	memset(&common.mreq, 0, sizeof(common.mreq));
	common.mreq.imr_multiaddr.s_addr = inet_addr(MCAST_IP);
	common.mreq.imr_interface.s_addr = htonl(INADDR_ANY);

	//register for the multicast group
	addToMultiCast(hDlg, common.udpSocket, common.mreq);

	if (buf == 0) {
		buf = static_cast<char *>(malloc(PACKET_SIZE * sizeof(char)));
		common.rcvBufUDP.buf = buf;
		common.rcvBufUDP.len = PACKET_SIZE;
	}

	//register comp routine
	if ((retVal = WSARecvFrom(common.udpSocket, &common.rcvBufUDP, 1, &recv, &flags, 0, 0, &common.udpOL, completionRoutineUDP)) == SOCKET_ERROR) {
		retVal = WSAGetLastError();

		if (retVal != WSA_IO_PENDING) {
			//let user know that receiving failed
			showMessageBox(hDlg, "UDP receive error.", "UDP Receiving Error", MB_ICONERROR);
			closesocket(common.udpSocket);
			return;
		}
	}

	//continuously listen for datagrams
	//SleepEx will set the thread in an alertable state in which Windows
	//will run the completion routine, it won't otherwise.
	while (common.udpRunning) {
		if (SleepEx(INFINITE, TRUE) != WAIT_IO_COMPLETION) {
			//error
			break;
		}
	}

	removeFromMultiCast(hDlg, common.udpSocket, common.mreq);
}

/*---------------------------------------------------------------------------------
-- FUNCTION:	  createSocket
--
-- DATE:		  April 7, 2017
--
-- DESIGNER:	  Michael Goll
--
-- DEVELOPER:     Michael Goll
--
-- INTERFACE:     bool createSocket(int connectionType)
--
-- PARAMETER:     int connectionType - Which protocol type you want a socket for
--                                     0 specifies a TCP socket.
--                                     1 specifies a UDP socket.
--
-- RETURNS:       bool - Whether or not the socket was successfully created
--
-- REVISIONS:
--
-- NOTES:         Creates either a UDP socket or a TCP socket.
-----------------------------------------------------------------------------------*/
bool createSocket(int connectionType) {
	switch (connectionType) {
	case 0:
		/*if ((common.tcpSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED)) == -1) {
			closesocket(common.tcpSocket);
			return FALSE;
		}*/
		if ((common.tcpSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
			closesocket(common.tcpSocket);
			return FALSE;
		}
		break;

	case 1:
		if ((common.udpSocket = WSASocket(AF_INET, SOCK_DGRAM, 0, 0, 0, WSA_FLAG_OVERLAPPED)) == -1) {
			closesocket(common.udpSocket);
			return FALSE;
		}
		break;
	}
	return TRUE;
}

/*---------------------------------------------------------------------------------
-- FUNCTION:	  fillServerInfo
--
-- DATE:		  April 7, 2017
--
-- DESIGNER:	  Michael Goll
--
-- DEVELOPER:     Aing Ragunathan, Michael Goll
--
-- INTERFACE:     void fillServerInfo(int connectionType)
--
-- PARAMETER:     int connectionType - Which protocol the server structure port is 
--                                     filled out.
--                                     0 specifies TCP.
--                                     1 specifies UDP.
--
-- RETURNS:       void
--
-- REVISIONS:
--
-- NOTES:         Fills out the server address structure information based on
--                which protocol is specified.
--                The port changes based on the default port for TCP/UDP
-----------------------------------------------------------------------------------*/
void fillServerInfo(int connectionType) {
	switch (connectionType) {
	case 0:
		common.serverAddrTCP.sin_family = AF_INET;
		common.serverAddrTCP.sin_port = htons(TCP_PORT);
		memcpy((char *)&common.serverAddrTCP.sin_addr, common.hp->h_addr, common.hp->h_length);
		break;

	case 1:
		common.serverAddrUDP.sin_family = AF_INET;
		common.serverAddrUDP.sin_port = htons(UDP_PORT);
		memcpy((char *)&common.serverAddrUDP.sin_addr, common.hp->h_addr, common.hp->h_length);
		break;
	}
}

/*---------------------------------------------------------------------------------
-- FUNCTION:	  tcpConnect
--
-- DATE:		  April 7, 2017
--
-- DESIGNER:	  Michael Goll
--
-- DEVELOPER:     Aing Ragunathan, Michael Goll
--
-- INTERFACE:     bool tcpConnect()
--
-- RETURNS:       bool - Whether or not the connection was successful.
--
-- REVISIONS:     
--
-- NOTES:         Forms a connection between the client and the specified
--                IP address.
-----------------------------------------------------------------------------------*/
bool tcpConnect() {
	int retVal;
	if ((retVal = connect(common.tcpSocket, (sockaddr *)&common.serverAddrTCP, sizeof(common.serverAddrTCP))) == SOCKET_ERROR) {
		retVal = WSAGetLastError();
		if (retVal != WSAEWOULDBLOCK && retVal != WSAEISCONN) {
			return FALSE;
		}
	}
	return TRUE;
}

/*---------------------------------------------------------------------------------
--  FUNCTION:	   tcpRecv
--
--  DATE:		   Mar 27, 2017
--
--  DESIGNER:	   Aing Ragunathan
--
--  INTERFACE:     bool tcpRecv()
--
--  RETURNS:
--
--  REVISIONS:     Integrated with the client UI | Michael Goll | April 7, 2017
--
--  NOTES:         Receives control messages from the server and updates lists accordingly
--				   or initiates downloading a file from the server.
-----------------------------------------------------------------------------------*/
bool tcpRecv() {
	CONTROL_MSG *controlMessage;
	INFO_CLIENT *clientData;
	_INFO_SONG *songData;

	//Get header from server
	if (recv(common.tcpSocket, common.messageBuffer, sizeof(int), 0) == -1) {
		perror("recvServerMessage - Recv control message failed!");
		return false;
	}
	//manage server update or download response
	int* header = reinterpret_cast<int*>(common.messageBuffer);
	//memcpy(&header, messageBuffer, sizeof(int));
	switch (*header)
	{
	case SONG_UPDATE:
		if (recv(common.tcpSocket, common.messageBuffer + sizeof(int), sizeof(_INFO_SONG) - sizeof(int), 0) == -1) {
			perror("recvServerMessage - Recv control message failed!");
			return false;
		}
		songData = (_INFO_SONG *)common.messageBuffer;	//extract song from buffer
		//songs.push_back(SongData());	//create a new song object in the vector
		_INFO_SONG recvSongData;
		recvSongData.SID = songData->SID;	//copy song id over
		sprintf_s(recvSongData.artist, STR_MAX_SIZE, "%s", songData->artist);
		sprintf_s(recvSongData.title, STR_MAX_SIZE, "%s", songData->title);
		//songs.push_back(recvSongData);
		break;

	case CLIENT_UPDATE:
		if (recv(common.tcpSocket, common.messageBuffer + sizeof(int), sizeof(INFO_CLIENT) - sizeof(int), 0) == -1) {
			perror("recvServerMessage - Recv control message failed!");
			return false;
		}
		clientData = (INFO_CLIENT *)common.messageBuffer;	//extract client from buffer
													//clients.push_back(ClientData());	//create a new client object in the vector
		INFO_CLIENT recvClientData;
		sprintf_s(recvClientData.username, STR_MAX_SIZE, "%s", clientData->username);
		sprintf_s(recvClientData.ip, STR_MAX_SIZE, "%s", clientData->ip);
		break;

	//case SONG_DOWNLOAD:
		/*
		while (sizeof(messageBuffer) == PACKET_SIZE) {
			if (recv(Socket, messageBuffer, sizeof(int), 0) == -1) {
				perror("recvServerMessage - Recv control message failed!");
				return false;
			}
		}
		
		break;
		*/
	}

	return true;
}

/*---------------------------------------------------------------------------------
--  FUNCTION:	downloadFile
--
--  DATE:		Mar 27, 2017
--
--  DESIGNER:	Aing Ragunathan
--
--  INTERFACE:  bool downloadFile()
--
--  RETURNS:
--
--  REVISIONS:  Integrated with the client UI | Michael Goll | April 7, 2017
--
--  NOTES:      Receives a file from the server. Number of packets must be specified.
-----------------------------------------------------------------------------------*/
bool downloadFile() {
	char* sid = "1";
	char filename[MAX_PATH] = TEST_FILE;

	/*	GUI	*/
	//get filename from GUI	-> char * filename
	//get corresponding SID of filename

	HANDLE fileInputHandle = CreateFile(getFileName(common.hDlg), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);


	//send request song packet (SID)
	_ReqDownloadSong downloadRequest;
	char * messageBuffer;

	//prepare song request packet, NO SID
	downloadRequest.SID = atoi(sid);
	messageBuffer = (char *)&downloadRequest;	//make struct sendable 
	send(common.tcpSocket, messageBuffer, strlen(messageBuffer), 0);

	//update GUI	- make function 
	return true;
}

/*---------------------------------------------------------------------------------
--  FUNCTION:		uploadFile
--
--  DATE:			Mar 27, 2017
--
--  DESIGNER:		Aing Ragunathan
--
--  INTERFACE:		none
--
--  RETURNS:		void
--
--  REVISIONS:     Integrated with the client UI | Michael Goll | April 7, 2017
--
--  NOTES:          Sends a contol message to the server before sending a file.
-----------------------------------------------------------------------------------*/
bool uploadFile(HWND hDlg) {
	TCHAR fileBuffer[PACKET_SIZE] = { 0 };	//buffer for file
											//TCHAR filename[MAX_PATH] = "McLaren.txt";
											//TCHAR filename[MAX_PATH] = TEST_FILE;
	char * filename = { getFileName(common.hDlg) };
	long totalNumberOfPackets;
	long lastPacketSize;
	char title[STR_MAX_SIZE] = "test";
	char artist[STR_MAX_SIZE] = "test1111";
	SoundFilePacketizer packer;


	/*	GUI	*/
	//get filename from GUI	-> char * filename
	//get artist
	//get title

	ReqUploadSong uploadRequest;
	char * messageBuffer;
	

	uploadRequest.header = 6;
	//prepare song request packet, NO SID
	sprintf_s(uploadRequest.artist, STR_MAX_SIZE, "%s", artist);
	sprintf_s(uploadRequest.title, STR_MAX_SIZE, "%s", title);

	//GetDlgItemText(hDlg, IDC_FILEPATH, filename, MAX_PATH);
	//sprintf_s(uploadRequest.filename, FILENAME_MAX, "%s", filename);
	//strcpy_s(uploadRequest.filename, MAX_PATH, filename);
	strcpy_s(uploadRequest.filename, FILENAME_MAX, "06 - Little Wing.flac");
	
	messageBuffer = (char *)&uploadRequest;	//make struct sendable 
	send(common.tcpSocket, messageBuffer, sizeof(ReqUploadSong), 0);


	//How would I preform validation for opening file?
	//file does not exists
	//file failed to open
	//can we make SoundFilePacketizer::makePacketsFromFile return bool?
	packer.makePacketsFromFile(uploadRequest.filename);
	totalNumberOfPackets = packer.getTotalPackets();
	lastPacketSize = packer.getLastPackSize();

	//send all packets except for last one
	for (int i = 0; i < totalNumberOfPackets - 1; i++) {
		char* temp = packer.getNextPacket();
		send(common.tcpSocket, temp, strlen(temp), 0);
		Sleep(1);
	}

	//send last packet
	char* temp = packer.getNextPacket();
	send(common.tcpSocket, temp, strlen(temp), 0);
	char complete[] = "EndOfPacket";
	send(common.tcpSocket, complete, strlen(complete)+1, 0);


	//update GUI	- make function 
	
	return true;
}

/*---------------------------------------------------------------------------------
-- FUNCTION:	  addToMultiCast
--
-- DATE:		  April 3, 2017
--
-- DESIGNER:	  Eva Yu
--
-- DEVELOPER:     Michael Goll
--
-- INTERFACE:     void removeFromMultiCast(SOCKET& s, ip_mreq& mreq)
--
-- RETURNS:	      none
--
-- REVISIONS:     
--
-- NOTES:         Adds the client to the multicast group on the server.
-----------------------------------------------------------------------------------*/
void addToMultiCast(HWND hDlg, SOCKET& s, ip_mreq& mreq) {
	int retVal;
	if ((retVal = setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof(mreq)) == -1)) {
		showMessageBox(hDlg, "Cannot join multicast group.", "Socket Option Error - SockOption Join", MB_ICONERROR);
	}
}

/*---------------------------------------------------------------------------------
-- FUNCTION:	  removeFromMultiCast
--
-- DATE:		  April 3, 2017
--
-- DESIGNER:	  Eva Yu
--
-- DEVELOPER:     Michael Goll
--
-- INTERFACE:     void removeFromMultiCast(SOCKET& s, ip_mreq& mreq)
--
-- RETURNS:	      none
--
-- REVISIONS:
--
-- NOTES:         Adds the client to the multicast group on the server.
-----------------------------------------------------------------------------------*/
void removeFromMultiCast(HWND hDlg, SOCKET& s, ip_mreq& mreq) {
	int retVal;
	if ((retVal = setsockopt(s, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&mreq, sizeof(mreq))) == -1) {
		retVal = WSAGetLastError();
		//showMessageBox(hDlg, "Cannot leave multicast group.", "Socket Option Error - SockOption Leave", MB_ICONERROR);
	}
}