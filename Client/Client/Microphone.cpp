#include "Microphone.h"

using namespace libZPlay;

/*---------------------------------------------------------------------------------
--  FUNCTION:		microphoneStart
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
--					Threaded function used to setup VOIP between clients. 
-----------------------------------------------------------------------------------*/
DWORD WINAPI microphoneStart(LPVOID lpParam) {
	ZPlay* player;
	

	player = 0;//CreateZPlay();

	if (player == 0) {
		perror("Microphone: Player failed!");
		return 0;
	}

	int ver = player->GetVersion();	//get library version

	//check version
	if (ver < 200) {
		perror("Microphone: version 2.00 or above required!");
		return 0;
	}



	// open wavein using Line In
	if (player->OpenFile("wavein://src=line;volume=50;", sfAutodetect) == 0) {
		perror("Microphone: Line in file failed!");
		player->Release();
		return 0;
	}

	//REPLACE WITH OUTSTREAM TO BUFFER
	// set wave output to disk file, mp3 encoding, no playback
	if (player->SetWaveOutFile(OUTPUT_FILE, sfMp3, 0) == 0)	{
		perror("Microphone: ouput file failed!");
		player->Release();
		return 0;
	}

	//set callback to intercept message (MsgStreamNeedMoreData)
	player->SetCallbackFunc(myCallbackFunc, (TCallbackMessage)MsgStreamNeedMoreData, NULL);


	// start playing from line-in
	if (player->Play() == 0) {
		perror("Microphone: Play failed!");
		player->Release(); // delete ZPlay class
		return 0;
	}

	return 0;
}

/*---------------------------------------------------------------------------------
--  FUNCTION:		myCallbackFunc
--
--  DATE:			Mar 27, 2017
--
--  DESIGNER:		Mike Goll
--
--  INTERFACE:
--
--  RETURNS:
--
--  NOTES:
--					Threaded function used to send VOIP packets to another client. 
-----------------------------------------------------------------------------------*/
int  __stdcall  myCallbackFunc(void* instance, void *user_data, TCallbackMessage message, unsigned int param1, unsigned int param2)
{
	/* 
		Requires a UDP socket

	if (send(udpSocket, (const char*) param1, param2, 0) == -1) {
		return 2;
	}

	*/
	return 1;
}


