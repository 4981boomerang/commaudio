#include "ClientTCP.h"
using namespace std;


/*-----------------------------------------------------------------------
--  SOURCE FILE:    client.cpp
--
--  PROGRAM:        COMP 4981 COMM Audio
--
--  FUNCTIONS:
--                  int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance,
--							  		  LPSTR lspszCmdParam, int nCmdShow);
--                  LRESULT CALLBACK Idle(HWND, UINT, WPARAM, LPARAM);
--
--
--  DATE:           Mar 27, 2017
--
--  DESIGNER:       Aing Ragunathan
--
--
--  NOTES:
--					This program consists of a client that is capable of connecting
--					to a server and listen to its audio broadcast. And is able to 
--					request songs to play as well as transfer audio files entirely.
--					The client is also able to connect to other clients and 
--					create a voip chat session.
----------------------------------------------------------------------------*/





/*---------------------------------------------------------------------------------
--  FUNCTION:		WinMain
--
--  DATE:			Mar 20, 2017
--
--  DESIGNER:		Aing Ragunathan
--
--
--  INTERFACE:		int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance,
--                                  	LPSTR lspszCmdParam, int nCmdShow)
--
--
--  RETURNS:		WM_QUITS message value or 0 if no messages sent
--
--  NOTES:
--					This function creates the main window, registers it, displays it
--					and sends messages.
-----------------------------------------------------------------------------------*/
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance,
	LPSTR lspszCmdParam, int nCmdShow)
{
	MSG Msg;
	WNDCLASSEX Wcl;
	mainInst = hInst;

	Wcl.cbSize = sizeof(WNDCLASSEX);
	Wcl.style = CS_HREDRAW | CS_VREDRAW;
	Wcl.hIcon = LoadIcon(NULL, IDI_APPLICATION); // large icon 
	Wcl.hIconSm = NULL; // use small version of large icon
	Wcl.hCursor = LoadCursor(NULL, IDC_ARROW);  // cursor style

	Wcl.lpfnWndProc = Idle;
	Wcl.hInstance = hInst;
	Wcl.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); //white background
	Wcl.lpszClassName = Name;

	Wcl.lpszMenuName = TEXT("MYMENU"); // The menu class
	Wcl.cbClsExtra = 0;      // no extra memory needed
	Wcl.cbWndExtra = 0;

	// Register the class
	if (!RegisterClassEx(&Wcl))
		return 0;

	hwnd = CreateWindow(Name, Name, WS_OVERLAPPEDWINDOW, 10, 10,
		475, 700, NULL, NULL, hInst, NULL);

	if (hwnd == NULL) {
		MessageBox(NULL, "Main window failed to open, program shutting down", "ERROR", MB_OK);
		return 0;
	}

	// Open up a Winsock v2.2 session
	WORD wVersionRequested = MAKEWORD(2, 2);
	WSADATA wsaData;
	WSAStartup(wVersionRequested, &wsaData);	

	//initialize LibZPlay library

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	while (GetMessage(&Msg, NULL, 0, 0))
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	return Msg.wParam;
}

/*---------------------------------------------------------------------------------
--  FUNCTION:		Idle
--
--  DATE:			Mar 20, 2017
--
--  DESIGNER:		Aing Ragunathan
--
--  INTERFACE:		LRESULT CALLBACK Idle(HWND hwnd, UINT Message,
--	                                        WPARAM wParam, LPARAM lParam)
--                           HWND hwnd: A handle to the window
--                           UINT Message: Recieved message
--                           WPARAM wParam: Additional message information
--                           LPARAM lParam: Additional message information
--
--  RETURNS:      
--
--  NOTES:
--					Idle state for managing connections, threads and UI
-----------------------------------------------------------------------------------*/
LRESULT CALLBACK Idle(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))  //Parsing the menu selections
		{
		case IDM_CONNECT:
			//TCP Connect/Commands Thread
			hThreadArray[numberOfThreads] = CreateThread(NULL, 0, connect, NULL, 0, &dwThreadIdArray[numberOfThreads]);	
			numberOfThreads++;
			break;
		case IDM_DOWNLOAD_FILE:
			//Download file Thread
			hThreadArray[numberOfThreads] = CreateThread(NULL, 0, downloadFile, NULL, 0, &dwThreadIdArray[numberOfThreads]);	
			numberOfThreads++;
			break;
		case IDM_UPLOAD_PACKET:
			//Upload file Thread
			hThreadArray[numberOfThreads] = CreateThread(NULL, 0, uploadFile, NULL, 0, &dwThreadIdArray[numberOfThreads]);	
			numberOfThreads++;
			break;
		case IDM_MUSIC:
			//UDP Connect/Music Thread
			hThreadArray[numberOfThreads] = CreateThread(NULL, 0, music, NULL, 0, &dwThreadIdArray[numberOfThreads]);	
			numberOfThreads++;
			break;
		case IDM_VOIP:
			//UDP VOIP Recv
			hThreadArray[numberOfThreads] = CreateThread(NULL, 0, microphoneStart, NULL, 0, &dwThreadIdArray[numberOfThreads]); 
			numberOfThreads++;

			//UDP VOIP Send
			hThreadArray[numberOfThreads] = CreateThread(NULL, 0, microphoneStart, NULL, 0, &dwThreadIdArray[numberOfThreads]);
			numberOfThreads++;
			break;
		}
		break;
		//END OF MENU SWITCH

	case WM_PAINT:
		PAINTSTRUCT ps;
		HDC hdc;
		hdc = BeginPaint(hwnd, &ps);
		EndPaint(hwnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		closesocket(Socket);
		WSACleanup();
		return 0;
	}
	//END OF MESSAGE SWITCH

	return DefWindowProc(hwnd, Message, wParam, lParam);
}

/*---------------------------------------------------------------------------------
--  FUNCTION:		Connect
--
--  DATE:			Mar 20, 2017
--
--  DESIGNER:		Aing Ragunathan
--
--  INTERFACE:  
--					
--
--  RETURNS:		
--
--  NOTES:
--					Connects to the server for tcp commands only.
--					Uses default server ip or port isn't included.
-----------------------------------------------------------------------------------*/
DWORD WINAPI connect(LPVOID lpParam){
	struct hostent *host;
	TCHAR ip[16] = DEFAULT_IP;
	int port = DEFAULT_PORT;

	//get IP and port from GUI

	//Resolve IP address
	if ((host = gethostbyname(ip)) == NULL)
	{
		MessageBox(hwnd,
			"Unable to resolve host name",
			"Error: GetHostByName",
			MB_ICONERROR);
		SendMessage(hwnd, WM_DESTROY, NULL, NULL);
		return false;
	}

	//Create TCP socket
	if ((Socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		MessageBox(hwnd,
			"socket creation failed!",
			"Error: Socket",
			MB_ICONERROR);
		SendMessage(hwnd, WM_DESTROY, NULL, NULL);
		return false;
	}

	// Set up our socket address structure
	SOCKADDR_IN SockAddr;
	SockAddr.sin_port = htons(port);
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_addr.s_addr = *((unsigned long*)host->h_addr);

	// Connecting to the server
	if (connect(Socket, (struct sockaddr *)&SockAddr, sizeof(SockAddr)) == SOCKET_ERROR)
	{
		MessageBox(hwnd,
			"Failed to connect",
			"Error: Connect",
			MB_ICONERROR);
		return false;
	}

	//get commands from the server
	hThreadArray[numberOfThreads] = CreateThread(NULL, 0, recvCommand, NULL, 0, &dwThreadIdArray[numberOfThreads]);
	numberOfThreads++;

	return true;
}


/*---------------------------------------------------------------------------------
--  FUNCTION:		Music
--
--  DATE:			Mar 20, 2017
--
--  DESIGNER:		Michael Goll
--
--  INTERFACE:
--
--  RETURNS:		
--
--  NOTES:
--					Thread for playing music from the server.
--					Music from the server arrives in the form of UDP packets.
--					Packets are handled by the packetizer class.
--					Music is played by the AudioPlayer class
-----------------------------------------------------------------------------------*/
DWORD WINAPI music(LPVOID lpParam) {
	struct hostent *host;
	TCHAR ip[16] = DEFAULT_IP;
	int port = DEFAULT_PORT;

	//get IP and port from GUI

	//Resolve IP address
	if ((host = gethostbyname(ip)) == NULL)
	{
		MessageBox(hwnd,
			"Unable to resolve host name",
			"Error: GetHostByName",
			MB_ICONERROR);
		SendMessage(hwnd, WM_DESTROY, NULL, NULL);
		return false;
	}

	//Create UDP socket with completion routine
	
	//Set up socket address structure
	
	//create audio player

	//while(user doesn't stall){
		//recv packet

		//play packet's contents
	//}

	return true;
}



/*---------------------------------------------------------------------------------
--  FUNCTION:		recvCommand
--
--  DATE:			Mar 27, 2017
--
--  DESIGNER:		Aing Ragunathan
--
--  INTERFACE:
--
--  RETURNS:
--
--  NOTES:
--					Threaded function used to receive commands from the server. Waits for
--					control messages on a loop until the program ends. 
--					Command messages can update the client or songs list.
-----------------------------------------------------------------------------------*/
DWORD WINAPI recvCommand(LPVOID lpParam) {
	char messageBuffer[PACKET_SIZE];
	ClientData *clientData;
	SongData *songData;
		
	while (1) {
		//Get header from server
		if (recv(Socket, messageBuffer, sizeof(int), 0) == -1) {
			perror("recvServerMessage - Recv control message failed!");
			return false;
		}
		//manage server command
		int* header = reinterpret_cast<int*>(messageBuffer);

		switch (*header)
		{
		case SONG_UPDATE:
			if (recv(Socket, messageBuffer + sizeof(int), sizeof(SongData) - sizeof(int), 0) == -1) {
				perror("recvServerMessage - Recv control message failed!");
				//update GUI
				return false;
			}
			songData = (SongData *)messageBuffer;	//extract song from buffer
			SongData recvSongData;
			recvSongData.SID = songData->SID;	//copy song id over
			sprintf_s(recvSongData.artist, STR_MAX_SIZE, "%s", songData->artist);
			sprintf_s(recvSongData.title, STR_MAX_SIZE, "%s", songData->title);
			songs.push_back(recvSongData); //add to the list of clients
			break;
		case CLIENT_UPDATE:
			if (recv(Socket, messageBuffer + sizeof(int), sizeof(ClientData) - sizeof(int), 0) == -1) {
				perror("recvServerMessage - Recv control message failed!");
				//update GUI
				return false;
			}
			clientData = (ClientData *)messageBuffer;	//extract client from buffer
			ClientData recvClientData;
			sprintf_s(recvClientData.username, STR_MAX_SIZE, "%s", clientData->username);
			sprintf_s(recvClientData.ip, STR_MAX_SIZE, "%s", clientData->ip);
			clients.push_back(recvClientData);	//add to the list of clients
			break;
		case SONG_DOWNLOAD:
			
		default:
			//packetize download
			break;
		}		
	}

	return true;
}

/*---------------------------------------------------------------------------------
--  FUNCTION:		uploadFile
--
--  DATE:			Mar 27, 2017
--
--	REVISIONS:
--					April 8, 2017 - Using packetizer functions
--
--  DESIGNER:		Aing Ragunathan
--
--  INTERFACE:		none
--
--  RETURNS:		void
--
--  NOTES:
--					Sends a contol message to the server before sending an entire song 
--					file using the Packetizer functions.
-----------------------------------------------------------------------------------*/
DWORD WINAPI uploadFile(LPVOID lpParam) {
	TCHAR fileBuffer[PACKET_SIZE] = { 0 };	//buffer for file
											//TCHAR filename[MAX_PATH] = "McLaren.txt";
	//TCHAR filename[MAX_PATH] = TEST_FILE;
	char filename[MAX_PATH] = TEST_FILE;
	long totalNumberOfPackets;	
	long lastPacketSize;	

	/*	GUI	*/
	//get filename from GUI	-> char * filename
	//get artist
	//get title
	
	SongData uploadRequest;
	char * messageBuffer;

	//prepare song request packet, NO SID
	sprintf_s(uploadRequest.artist, STR_MAX_SIZE, "%s", TEST_ARTIST);
	sprintf_s(uploadRequest.title, STR_MAX_SIZE, "%s", TEST_TITLE);

	messageBuffer = (char *) &uploadRequest;	//make struct sendable 
	send(Socket, messageBuffer, strlen(messageBuffer), 0);


	//How would I preform validation for opening file?
		//file does not exists
		//file failed to open
		//can we make SoundFilePacketizer::makePacketsFromFile return bool?
	packer.makePacketsFromFile(filename);
	totalNumberOfPackets = packer.getTotalPackets();
	lastPacketSize = packer.getLastPackSize();

	//send all packets except for last one
	for (int i = 0; i < totalNumberOfPackets; i++) {
		send(Socket, packer.getNextPacket(), PACKET_SIZE, 0);
	}
		
	//send last packet
	send(Socket, packer.getNextPacket(), lastPacketSize, 0); 	


	//update GUI	- make function 


	return true;
}

/*---------------------------------------------------------------------------------
--  FUNCTION:		downloadFile
--
--  DATE:			April 7, 2017
--
--  DESIGNER:		Aing Ragunathan
--
--  INTERFACE:
--
--  RETURNS:
--
--  NOTES:
--					Receives a file from the server. Number of packets must be specified.
-----------------------------------------------------------------------------------*/
//WIP
DWORD WINAPI downloadFile(LPVOID lpParam) {
	char* sid = "1";

	/*	GUI	*/
	//get filename from GUI	-> char * filename
	//get corresponding SID of filename


	//send request song packet (SID)
	SongData downloadRequest;
	char * messageBuffer;

	//prepare song request packet, NO SID
	downloadRequest.SID = atoi(sid);
	messageBuffer = (char *)&downloadRequest;	//make struct sendable 
	send(Socket, messageBuffer, strlen(messageBuffer), 0);
	
	//update GUI	- make function 
	return true;
}
