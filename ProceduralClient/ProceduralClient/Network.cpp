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
-- FUNCTION: completionRoutine
--
-- DATE: APR. 09, 2017
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/APR/09 - Created Function 
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE: void completionRoutine (DWORD error, DWORD transferred, LPWSAOVERLAPPED ol, DWORD flags)
-- DWORD error -- the error value
-- DWORD transferred -- number of bytes transderref
-- DWORD flags -- flags assocated with wsarecv
--
-- NOTES:
-- static function to help direct to class function
--------------------------------------------------------------------------*/
void callbackRoutine(DWORD error, DWORD transferred, LPWSAOVERLAPPED ol, DWORD flags)
{
	
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

		//empty the receiving buffer
		memset(common.rcvBufUDP.buf, 0, sizeof(common.rcvBufUDP.buf));

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
	common.serverAddrUDP.sin_port = htons(UDP_PORT);
	memcpy((char *)&common.serverAddrUDP.sin_addr, common.udpHP->h_addr, common.udpHP->h_length);

	if ((retVal = bind(common.udpSocket, (SOCKADDR *)&common.serverAddrUDP, sizeof(SOCKADDR_IN))) == -1) {
		retVal = WSAGetLastError();
		closesocket(common.udpSocket);
		showMessageBox(hDlg, "Cannot Bind Socket, Aborting...", "Socket Bind Error", MB_OK | MB_ICONERROR);
		return;
	}

	if (buf == 0) {
		buf = static_cast<char *>(malloc(PACKET_SIZE * sizeof(char)));
		common.rcvBufUDP.buf = buf;
		common.rcvBufUDP.len = 1024;
	}

	memset(&common.mreq, 0, sizeof(common.mreq));
	common.mreq.imr_multiaddr.s_addr = inet_addr(MCAST_IP);
	common.mreq.imr_interface.s_addr = htonl(INADDR_ANY);

	//register for the multicast group
	addToMultiCast(hDlg, common.udpSocket, common.mreq);

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
		if ((common.tcpSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED)) == -1) {
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
	ControlMessage *controlMessage;
	ClientData *clientData;
	SongData *songData;

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
		if (recv(common.tcpSocket, common.messageBuffer + sizeof(int), sizeof(SongData) - sizeof(int), 0) == -1) {
			perror("recvServerMessage - Recv control message failed!");
			return false;
		}
		songData = (SongData *)common.messageBuffer;	//extract song from buffer
		//songs.push_back(SongData());	//create a new song object in the vector
		SongData recvSongData;
		recvSongData.SID = songData->SID;	//copy song id over
		sprintf_s(recvSongData.artist, STR_MAX_SIZE, "%s", songData->artist);
		sprintf_s(recvSongData.title, STR_MAX_SIZE, "%s", songData->title);
		//songs.push_back(recvSongData);
		break;

	case CLIENT_UPDATE:
		if (recv(common.tcpSocket, common.messageBuffer + sizeof(int), sizeof(ClientData) - sizeof(int), 0) == -1) {
			perror("recvServerMessage - Recv control message failed!");
			return false;
		}
		clientData = (ClientData *)common.messageBuffer;	//extract client from buffer
													//clients.push_back(ClientData());	//create a new client object in the vector
		ClientData recvClientData;
		sprintf_s(recvClientData.username, STR_MAX_SIZE, "%s", clientData->username);
		sprintf_s(recvClientData.ip, STR_MAX_SIZE, "%s", clientData->ip);
		break;

	case SONG_REQUEST:
		//get file from server and save to disk
		if (!downloadFile()) {
			return false;
		}
		break;
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
	//differentiate file packet from command
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
bool uploadFile() {
	HANDLE fileOutputHandle;	//Handle to the requested file to send
	TCHAR fileBuffer[PACKET_SIZE] = { 0 };	//buffer for file
											//TCHAR filename[MAX_PATH] = "McLaren.txt";
	TCHAR filename[MAX_PATH] = TEST_FILE;
	OVERLAPPED ol = { 0 };
	DWORD bytesRead;
	std::string temp;


	fileOutputHandle = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	//validate file exists and send
	if (fileOutputHandle == INVALID_HANDLE_VALUE) {
		//give user an error
	}
	else {
		do {
			if (!ReadFile(fileOutputHandle, fileBuffer, PACKET_SIZE - 1, &bytesRead, &ol)) {
				printf("Terminal failure: Unable to read from file.\n GetLastError=%08x\n", GetLastError());
				//read from file error, notify user
				CloseHandle(fileOutputHandle);
				return false;
			}

			//append a null character to cut off the string if the entire buffer isn't used
			if (strlen(fileBuffer) < 1023) {
				fileBuffer[bytesRead] = '\0';
			}

			//send(Socket, fileBuffer, strlen(fileBuffer), 0); //send the buffer to the server		
			send(common.tcpSocket, fileBuffer, bytesRead, 0); //send the buffer to the server		
			ol.Offset += bytesRead;	//move the reading window 
			temp = fileBuffer;	//reset the buffer
			temp.resize(PACKET_SIZE);
		} while (strlen(fileBuffer) >= PACKET_SIZE - 1);
	}


	return true;
}

/*---------------------------------------------------------------------------------
-- FUNCTION:	 requestSong
--
-- DATE:		 April 3, 2017
--
-- DESIGNER:	 Aing Ragunathan
--
-- DEVELOPER:    Aign Ragunathan
--
-- INTERFACE:    bool requestSong(int song)
--
-- PARAMETER:    int song - SID of the song requested
--
-- RETURNS:	     bool - Whether or not the request was successful.
--
-- REVISIONS:    Integrated with the client UI | Michael Goll | April 7, 2017
--
-- NOTES:        Sends a song request to the server.
-----------------------------------------------------------------------------------*/
bool requestSong(int song) {
	ControlMessage *controlMessage;
	char *messageBuffer;

	//setup control message
	controlMessage = new ControlMessage();
	controlMessage->header = SONG_REQUEST;
	controlMessage->SID = song;
	messageBuffer = (char *)controlMessage;

	//send message to server
	if (send(common.tcpSocket, messageBuffer, strlen(messageBuffer), 0) == -1)
		return false;

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