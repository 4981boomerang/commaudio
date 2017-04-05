#include <cstdlib>
#include <stdio.h>

#include "../Headers/AudioPlayer.h"

using namespace libZPlay;

ZPlay * AudioPlayer::player = nullptr;

/*--------------------------------------------------------------------------------------------
-- FUNCTION:   AudioPlayer
--
-- DATE:       April 3, 2017
--
-- DESIGNER:   Michael Goll, Aing Ragunathan, Eva Yu, Jamie Lee
--
-- PROGRAMMER: Michael Goll
--
-- INTERFACE:  AudioPlayer()
--
-- PARAMETER:  none
--
-- RETURNS:    N/A
--
-- NOTES:      Constructor, creates the player.
--------------------------------------------------------------------------------------------*/
AudioPlayer::AudioPlayer() {
	if (!player)
		createPlayer();
}

/*--------------------------------------------------------------------------------------------
-- FUNCTION:   ~AudioPlayer
--
-- DATE:       April 3, 2017
--
-- DESIGNER:   Michael Goll, Aing Ragunathan, Eva Yu, Jamie Lee
--
-- PROGRAMMER: Michael Goll
--
-- INTERFACE:  ~AudioPlayer()
--
-- PARAMETER:  none
--
-- RETURNS:    N/A
--
-- NOTES:      Destructor, releases the player resources if it is initialized.
--------------------------------------------------------------------------------------------*/
AudioPlayer::~AudioPlayer() {
	if (player)
		player->Release();
}

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
bool AudioPlayer::createPlayer() {
	if (player = CreateZPlay()) {
		return true;
	}
	
	return false;
}

/*--------------------------------------------------------------------------------------------
-- FUNCTION:   getPlayer
--
-- DATE:       April 3, 2017
--
-- DESIGNER:   Michael Goll, Aing Ragunathan, Eva Yu, Jamie Lee
--
-- PROGRAMMER: Michael Goll
--
-- INTERFACE:  libZPlay::ZPlay * getPlayer()
--
-- PARAMETER:  none
--
-- RETURNS:    ZPlay * - The player created for playing music.
--
-- NOTES:      Creates a libZPlay player for playing music.
--------------------------------------------------------------------------------------------*/
ZPlay * AudioPlayer::getPlayer() {
	return player;
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
void AudioPlayer::play() {
	TStreamStatus status;
	player->Play();

	//while it's playing
	while (true) {
		//check the status of playback
		player->GetStatus(&status);

		//Error occurs during playback, abort
		if (!status.fPlay)
			break;

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
void AudioPlayer::pause() {
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
void AudioPlayer::stop() {
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
*/
void AudioPlayer::init() {
	/*if ((player->OpenStream(1, 1, &buffer, read, format)) == 0) {
		//error occurred opening the stream
		//will we need the callback if we have the buffer handling things?
	}*/

	//disables the callback functionality, possibly temporary depending
	//on how the player works when the buffer is already defined
	player->SetCallbackFunc(0, (TCallbackMessage)MsgStreamNeedMoreData, 0);
}

/*--------------------------------------------------------------------------------------------
-- FUNCTION:   callbackFunc
--
-- DATE:       April 3, 2017
--
-- DESIGNER:   LibZPlay Library
--
-- PROGRAMMER: Michael Goll
--
-- INTERFACE:  void init()
--
-- PARAMETER:  void *           - instance of the application, casted to ZPlay type.
--             void *           - user data to be used.
--             TCallbackMessage - The libZPlay message for the player to react to.
--             unsigned int     - Parameter 1, not used but required in the interface.
--             unsigned int     - Parameter 2, not used but required in the interface.
--
-- RETURNS:    int - Always returns 0, required by the library.
--
-- NOTES:      Callback function for the player, called when player needs more data
--             in the buffer.
--------------------------------------------------------------------------------------------*/

//should not need this depending on how the circular buffer works.
int __cdecl AudioPlayer::callbackFunc(void * instance, void * userData, TCallbackMessage message, unsigned int param1, unsigned int param2) {
	switch (message) {
	case MsgStreamNeedMoreData:
		//player needs more data, grab from buffer
		//player->PushDataToStream(buffer, read);
		break;
	}

	return 0;
}