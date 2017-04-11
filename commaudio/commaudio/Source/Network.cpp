#include "../Headers/Network.h"


/*---------------------------------------------------------------------------------
-- FUNCTION:	  Network
--
-- DATE:		  April 3, 2017
--
-- DESIGNER:	  Michael Goll
--
-- DEVELOPER:     Michael Goll
--
-- INTERFACE:     Network::Network(UI * uiOrig)
--
-- PARAMETER:     UI * uiOrig - The original UI object created in main.
--
-- RETURNS:       N/A
--
-- REVISIONS:
--
-- NOTES:         Creates the network object and initializes member variables.
-----------------------------------------------------------------------------------*/
Network::Network(UI * uiOrig) : serverAddrTCP{ 0 }, serverAddrUDP{ 0 }, wsadata{ 0 }, sendBufTCP{ 0 }, sendBufUDP{ 0 }, rcvBufUDP{ 0 }, rcvBufTCP{ 0 },
		wVersionRequested(MAKEWORD(2, 2)), tcpRunning(FALSE), udpRunning(FALSE), ui(uiOrig) {
	//initialize overlapped structures
	memset(&tcpOL, 0, sizeof(WSAOVERLAPPED));
	memset(&udpOL, 0, sizeof(WSAOVERLAPPED));
	
	help.ol = &udpOL;
	help.thisPtr = this;
}

/*---------------------------------------------------------------------------------
-- FUNCTION:	  ~Network
--
-- DATE:		  April 3, 2017
--
-- DESIGNER:	  Michael Goll
--
-- DEVELOPER:     Michael Goll
--
-- INTERFACE:     Network::~Network()
--
-- RETURNS:       N/A
--
-- REVISIONS:
--
-- NOTES:         Releases all of the Network resources that will not be released
--                automatically.
-----------------------------------------------------------------------------------*/
Network::~Network() {}

/*---------------------------------------------------------------------------------
-- FUNCTION:	  clientStart
--
-- DATE:		  April 3, 2017
--
-- DESIGNER:	  Michael Goll
--
-- DEVELOPER:     Michael Goll
--
-- INTERFACE:     bool Network::clientStart()
--
-- RETURNS:       bool - Whether or not WSA was started up successfully.
--
-- REVISIONS:
--
-- NOTES:         Starts WSAStartup to use network functionality.
-----------------------------------------------------------------------------------*/
bool Network::clientStart() {
	if (WSAStartup(wVersionRequested, &wsadata) != 0) {
		//show user there is an error
		//ui->showMessageBox("Cannot start WSAStartup", "WSAStartup Error", MB_ICONERROR);
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
-- INTERFACE:     void Network::clientStop(bool stopTCP, bool stopUDP)
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
void Network::clientStop(bool stopTCP, bool stopUDP) {
	if (stopTCP)
		tcpRunning = FALSE;

	if (stopUDP)
		udpRunning = FALSE;

	TCPthread.join();
	UDPthread.join();
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
void Network::completionRoutine(DWORD error, DWORD transferred, LPWSAOVERLAPPED ol, DWORD flags)
{
	reinterpret_cast<Helper *>(&ol)->thisPtr->callbackRoutine(error, transferred, flags);
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
void Network::callbackRoutine(DWORD error, DWORD transferred, DWORD flags)
{
	int retVal;
	DWORD bRecv;
	
	switch (error) {
		//no error here
	case 0:
		//push data received into the circular buffer
		AudioPlayer::instance().getBuf().push_back(rcvBufUDP.buf);

		//empty the receiving buffer
		memset(rcvBufUDP.buf, 0, sizeof(rcvBufUDP.buf));

		//register again
		if ((retVal = WSARecvFrom(udpSocket, &rcvBufUDP, 1, &bRecv, &flags, 0, 0, help.ol, completionRoutine)) == SOCKET_ERROR) {
			retVal = WSAGetLastError();

			if (retVal != WSA_IO_PENDING) {
				closesocket(udpSocket);
				clientStop(FALSE, TRUE);
				return;
			}
		}

		break;

	case WSA_OPERATION_ABORTED:
		//ui->showMessageBox("Error: Operation Aborted", "Operation Aborted", MB_ICONERROR);
		clientStop(FALSE, TRUE);
		break;

	default:
		clientStop(FALSE, TRUE);
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
-- INTERFACE:     void Network::startTCP()
--
-- RETURNS:       void
--
-- REVISIONS:
--
-- NOTES:         Runs through the TCP setup processes.
-----------------------------------------------------------------------------------*/
void Network::startTCP() {

	int errorReturn;

	//dns query, an IP address would return itself
	if (!(hp = gethostbyname(dest))) {
		return;
	}

	if (!createSocket(0)) {
		//ui->showMessageBox("Cannot create TCP socket.", "TCP Socket Creation Error", MB_ICONERROR);
		errorReturn = WSAGetLastError();
		return;
	}

	fillServerInfo(0);

	if (!tcpConnect()) {
		//ui->showMessageBox("Cannot connect to the server.", "Connection Error", MB_ICONERROR);
		return;
	}

	//ui->swapButtons(IDC_CONNECT, IDC_DISCONNECT);
	
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
-- INTERFACE:     void Network::startTCP()
--
-- RETURNS:       void
--
-- REVISIONS:
--
-- NOTES:         Runs through the UDP setup processes.
-----------------------------------------------------------------------------------*/
void Network::startUDP() {

	int retVal;
	DWORD flags = MSG_PARTIAL, recv;

	memset(&mreq, 0, sizeof(mreq));
	mreq.imr_multiaddr.s_addr = inet_addr(MCAST_IP);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);

	//dns query, an IP address would return itself
	if (!(hp = gethostbyname(dest))) {
		return;
	}

	if (!createSocket(1)) {
		//ui->showMessageBox("Cannot create UDP socket.", "UDP Socket Creation Error", MB_ICONERROR);
		return;
	}

	fillServerInfo(1);

	//register comp routine
	if ((retVal = WSARecvFrom(udpSocket, &rcvBufUDP, 1, &recv, &flags, 0, 0, &udpOL, completionRoutine) ) == SOCKET_ERROR) {
		retVal = WSAGetLastError();

		if (retVal != WSA_IO_PENDING) {
			//let user know that receiving failed
			retVal = WSAGetLastError();
			//ui->showMessageBox("UDP receive error.", "UDP Receiving Error", MB_ICONERROR);
			closesocket(udpSocket);
			return;
		}
	}

	//register for the multicast group
	addToMultiCast(udpSocket, mreq);

	//continuously listen for datagrams
	//SleepEx will set the thread in an alertable state in which Windows
	//will run the completion routine, it won't otherwise.
	while (udpRunning) {
		if (SleepEx(INFINITE, TRUE) != WAIT_IO_COMPLETION) {
			//error
			break;
		}
	}
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
-- INTERFACE:     bool Network::createSocket(int connectionType)
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
bool Network::createSocket(int connectionType) {
	switch (connectionType) {
	case 0:
		if ((tcpSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED)) == -1) {
			closesocket(tcpSocket);
			return FALSE;
		}
		break;

	case 1:
		if ((udpSocket = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, 0, 0, WSA_FLAG_OVERLAPPED)) == -1) {
			closesocket(udpSocket);
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
-- INTERFACE:     void Network::fillServerInfo(int connectionType)
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
void Network::fillServerInfo(int connectionType) {
	switch (connectionType) {
	case 0:
		serverAddrTCP.sin_family = AF_INET;
		serverAddrTCP.sin_port = htons(TCP_PORT);
		memcpy((char *)&serverAddrTCP.sin_addr, hp->h_addr, hp->h_length);
		break;

	case 1:
		serverAddrUDP.sin_family = AF_INET;
		serverAddrUDP.sin_port = htons(UDP_PORT);
		memcpy((char *)&serverAddrUDP.sin_addr, hp->h_addr, hp->h_length);
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
bool Network::tcpConnect() {
	int retVal;
	if ((retVal = connect(tcpSocket, (sockaddr *)&serverAddrTCP, sizeof(serverAddrTCP))) == SOCKET_ERROR) {
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
bool Network::tcpRecv() {
	ControlMessage *controlMessage;
	ClientData *clientData;
	SongData *songData;

	//Get header from server
	if (recv(tcpSocket, messageBuffer, sizeof(int), 0) == -1) {
		perror("recvServerMessage - Recv control message failed!");
		return false;
	}
	//manage server update or download response
	int* header = reinterpret_cast<int*>(messageBuffer);
	//memcpy(&header, messageBuffer, sizeof(int));
	switch (*header)
	{
	case SONG_UPDATE:
		if (recv(tcpSocket, messageBuffer + sizeof(int), sizeof(SongData) - sizeof(int), 0) == -1) {
			perror("recvServerMessage - Recv control message failed!");
			return false;
		}
		songData = (SongData *)messageBuffer;	//extract song from buffer
		//songs.push_back(SongData());	//create a new song object in the vector
		SongData recvSongData;
		recvSongData.SID = songData->SID;	//copy song id over
		sprintf_s(recvSongData.artist, STR_MAX_SIZE, "%s", songData->artist);
		sprintf_s(recvSongData.title, STR_MAX_SIZE, "%s", songData->title);
		//songs.push_back(recvSongData);
		break;

	case CLIENT_UPDATE:
		if (recv(tcpSocket, messageBuffer + sizeof(int), sizeof(ClientData) - sizeof(int), 0) == -1) {
			perror("recvServerMessage - Recv control message failed!");
			return false;
		}
		clientData = (ClientData *)messageBuffer;	//extract client from buffer
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
bool Network::downloadFile() {
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
bool Network::uploadFile() {
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
			send(tcpSocket, fileBuffer, bytesRead, 0); //send the buffer to the server		
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
-- INTERFACE:    bool Network::requestSong(int song)
--
-- PARAMETER:    int song - SID of the song requested
--
-- RETURNS:	     bool - Whether or not the request was successful.
--
-- REVISIONS:    Integrated with the client UI | Michael Goll | April 7, 2017
--
-- NOTES:        Sends a song request to the server.
-----------------------------------------------------------------------------------*/
bool Network::requestSong(int song) {
	ControlMessage *controlMessage;
	char *messageBuffer;

	//setup control message
	controlMessage = new ControlMessage();
	controlMessage->header = SONG_REQUEST;
	controlMessage->SID = song;
	messageBuffer = (char *)controlMessage;

	//send message to server
	if (send(tcpSocket, messageBuffer, strlen(messageBuffer), 0) == -1)
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
-- INTERFACE:     void Network::removeFromMultiCast(SOCKET& s, ip_mreq& mreq)
--
-- RETURNS:	      none
--
-- REVISIONS:     
--
-- NOTES:         Adds the client to the multicast group on the server.
-----------------------------------------------------------------------------------*/
void Network::addToMultiCast(SOCKET& s, ip_mreq& mreq) {
	int retVal;
	if ((retVal = setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof(mreq)) == -1)) {
		//ui->showMessageBox("Cannot join multicast group.", "Socket Option Error - SockOption Join", MB_ICONERROR);
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
-- INTERFACE:     void Network::removeFromMultiCast(SOCKET& s, ip_mreq& mreq)
--
-- RETURNS:	      none
--
-- REVISIONS:
--
-- NOTES:         Adds the client to the multicast group on the server.
-----------------------------------------------------------------------------------*/
void Network::removeFromMultiCast(SOCKET& s, ip_mreq& mreq) {
	int retVal;
	if ((retVal = setsockopt(s, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&mreq, sizeof(mreq)) == -1)) {
		//ui->showMessageBox("Cannot leave multicast group.", "Socket Option Error - SockOption Leave", MB_ICONERROR);
	}
}