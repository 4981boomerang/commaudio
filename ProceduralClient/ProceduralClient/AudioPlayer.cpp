#include <cstdlib>
#include <stdio.h>

#include "AudioPlayer.h"

using namespace libZPlay;

int __stdcall callbackFunc(void * instance, void * userData, TCallbackMessage message, unsigned int param1, unsigned int param2);

/*--------------------------------------------------------------------------------------------
-- FUNCTION:   createPlayer
--
-- DATE:       April 3, 2017
--
-- DESIGNER:   Michael Goll, Aing Ragunathan, Eva Yu, Jamie Lee
--
-- PROGRAMMER: Michael Goll
--
-- INTERFACE:  bool createPlayer()
--
-- PARAMETER:  none
--
-- RETURNS:    bool - TRUE if the player was created successfully.
--                    FALSE if an error occurred during player creation.
--
-- NOTES:      Creates a libZPlay player for playing music.
--------------------------------------------------------------------------------------------*/
ZPlay * createPlayer() {
	ZPlay * player;
	if (player = CreateZPlay()) {
		return player;
	}
	
	return nullptr;
}

/*--------------------------------------------------------------------------------------------
-- FUNCTION:   play
--
-- DATE:       April 3, 2017
--
-- DESIGNER:   Michael Goll, Aing Ragunathan, Eva Yu, Jamie Lee
--
-- PROGRAMMER: Michael Goll
--
-- INTERFACE:  void play()
--
-- PARAMETER:  none
--
-- RETURNS:    void
--
-- NOTES:      Sets the state of the player to "play".
--------------------------------------------------------------------------------------------*/
void play(HWND hDlg, ZPlay * player) {
	TStreamStatus status;
	player->Play();

	//while it's playing
	while (true) {
		//check the status of playback
		player->GetStatus(&status);

		//Error occurs during playback, abort
		if (!status.fPlay) {
			break;
		}

		//update UI with new song position
	}
}

/*--------------------------------------------------------------------------------------------
-- FUNCTION:   pause
--
-- DATE:       April 3, 2017
--
-- DESIGNER:   Michael Goll, Aing Ragunathan, Eva Yu, Jamie Lee
--
-- PROGRAMMER: Michael Goll
--
-- INTERFACE:  void pause()
--
-- PARAMETER:  none
--
-- RETURNS:    void
--
-- NOTES:      Sets the state of the player to "pause".
--------------------------------------------------------------------------------------------*/
void pause(HWND hDlg, ZPlay * player) {
	player->Pause();
}

/*--------------------------------------------------------------------------------------------
-- FUNCTION:   stop
--
-- DATE:       April 3, 2017
--
-- DESIGNER:   Michael Goll, Aing Ragunathan, Eva Yu, Jamie Lee
--
-- PROGRAMMER: Michael Goll
--
-- INTERFACE:  void stop()
--
-- PARAMETER:  none
--
-- RETURNS:    void
--
-- NOTES:      Sets the state of the player to "stop".
--------------------------------------------------------------------------------------------*/
void stop(ZPlay * player) {
	player->Stop();
}

/*--------------------------------------------------------------------------------------------
-- FUNCTION:   init
--
-- DATE:       April 3, 2017
--
-- DESIGNER:   Michael Goll, Aing Ragunathan, Eva Yu, Jamie Lee
--
-- PROGRAMMER: Michael Goll
--
-- INTERFACE:  void init()
--
-- PARAMETER:  none
--
-- RETURNS:    void
--
-- NOTES:      Opens a stream for audio and associates the stream with a buffer.
--------------------------------------------------------------------------------------------*/
/*
-- Need the server to also send the format along with the data
-- The "&buffer" needs to be replaced to connect to the circular buffer
-- read is the size of how much to read (packet size)
-- the very first packet needs to be ~21000 bytes in order to get over the header
--   and include at least one valid MP3 frame.
*/
void init(ZPlay * player) {
	char buffer[21000];

	if ((player->OpenStream(0, 1, &buffer, 21000, sfMp3)) == 0) {
		//error occurred opening the stream
		char * error = player->GetError();
	}
}

int __stdcall callbackFunc(void * instance, void * userData, TCallbackMessage message, unsigned int param1, unsigned int param2) {
	ZPlay * player = (ZPlay*)instance;
	char buffer[PACKET_SIZE];

	//grab the next packet in the buffer
	strcpy_s(buffer, PACKET_SIZE, common.cbuff.pop().c_str());

	switch (message) {
	case MsgStreamNeedMoreData:
		//add the packet to the internal buffer
		player->PushDataToStream(buffer, PACKET_SIZE);
		break;

		return 0;
	}
	return 0;
}