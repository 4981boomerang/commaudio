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
Network::Network(UI * uiOrig) : serverAddr{ 0 }, wsadata{ 0 }, sendBufTCP{ 0 }, sendBufUDP{ 0 }, rcvBufUDP{ 0 }, rcvBufTCP{ 0 },
		wVersionRequested(MAKEWORD(2, 2)), tcpRunning(FALSE), udpRunning(FALSE), ui(uiOrig) {
	//initialize overlapped structures
	memset(&tcpOL, 0, sizeof(WSAOVERLAPPED));
	memset(&udpOL, 0, sizeof(WSAOVERLAPPED));
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
Network::~Network() {
	WSACleanup();
}

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
		ui->showMessageBox("Cannot start WSAStartup", "WSAStartup Error", MB_OK | MB_ICONERROR);
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
}

//called after every successful read
//TODO: Get the callback to be called nicely in the class
void CALLBACK compRoutine(DWORD error, DWORD transferred, LPWSAOVERLAPPED ol, DWORD flags) {
#if 0
	int retVal;

	switch (error) {
		//no error here
	case 0:
		AudioPlayer::instance().getBuf().push_back(rcvBufUDP.buf);

		//empty the buffer
		memset(rcvBufUDP.buf, 0, sizeof(rcvBufUDP.buf));

		//register again
		if ((retVal = WSARecvFrom(udpSocket, &rcvBufUDP, 1, &recv, &flags, 0, 0, &udpOL, compRoutine)) == SOCKET_ERROR) {
			retVal = WSAGetLastError();

			if (retVal != WSA_IO_PENDING) {
				closesocket(sockets[1]);
				ClientStop(FALSE, TRUE);
				return;
			}
		}

		break;

	case WSA_OPERATION_ABORTED:
		clientStop(FALSE, TRUE);
		break;

	default:
		clientStop(FALSE, TRUE);
		break;
	}
#endif
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
	clientStart();

	//dns query, an IP address would return itself
	if (!(hp = gethostbyname(dest))) {
		WSACleanup();
		return;
	}

	if (!createSocket(0)) {
		ui->showMessageBox("Cannot create TCP socket.", "TCP Socket Creation Error", MB_OK | MB_ICONERROR);
		errorReturn = WSAGetLastError();
		return;
	}

	fillServerInfo(0);

	if (!tcpConnect()) {
		ui->showMessageBox("Cannot connect to the server.", "Connection Error", MB_OK | MB_ICONERROR);
		return;
	}

	ui->swapButtons(IDC_CONNECT, IDC_DISCONNECT);
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

	//dns query, an IP address would return itself
	if (!(hp = gethostbyname(dest))) {
		WSACleanup();
		return;
	}

	if (!createSocket(1)) {
		ui->showMessageBox("Cannot create UDP socket.", "UDP Socket Creation Error", MB_OK | MB_ICONERROR);
		return;
	}

	fillServerInfo(1);

	//register comp routine
	if ((retVal = WSARecvFrom(udpSocket, &rcvBufUDP, 1, &recv, &flags, 0, 0, &udpOL, compRoutine)) == SOCKET_ERROR) {
		retVal = WSAGetLastError();

		if (retVal != WSA_IO_PENDING) {
			//let user know that receiving failed
			retVal = WSAGetLastError();
			ui->showMessageBox("UDP receive error.", "UDP Receiving Error", MB_OK | MB_ICONERROR);
			closesocket(udpSocket);
			return;
		}
	}

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
			WSACleanup();
			return FALSE;
		}
		break;

	case 1:
		if ((udpSocket = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, 0, 0, WSA_FLAG_OVERLAPPED)) == -1) {
			closesocket(udpSocket);
			WSACleanup();
			return FALSE;
		}
		break;
	}
	return TRUE;
}

//changes depending on the type of connection
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
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(TCP_PORT);
		memcpy((char *)&serverAddr.sin_addr, hp->h_addr, hp->h_length);
		break;

	case 1:
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(UDP_PORT);
		memcpy((char *)&serverAddr.sin_addr, hp->h_addr, hp->h_length);
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
	if ((retVal = connect(tcpSocket, (sockaddr *)&serverAddr, sizeof(serverAddr))) == SOCKET_ERROR) {
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
--  FUNCTION:	   requestSong
--
--  DATE:		   April 3, 2017
--
--  DESIGNER:	   Aing Ragunathan
--
--  INTERFACE:     int song - SID of the song requested
--
--  RETURNS:	   none
--
--  REVISIONS:     Integrated with the client UI | Michael Goll | April 7, 2017
--
--  NOTES:         Sends a song request to the server.
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