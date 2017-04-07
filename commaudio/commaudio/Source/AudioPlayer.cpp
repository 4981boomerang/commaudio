#include <cstdlib>
#include <stdio.h>

#include "../Headers/AudioPlayer.h"

using namespace libZPlay;

AudioPlayer AudioPlayer::aInstance;

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
AudioPlayer::AudioPlayer() : player(createPlayer()), cbuff() {}

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
ZPlay * AudioPlayer::createPlayer() {
	if (player = CreateZPlay()) {
		return player;
	}
	
	return nullptr;
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
-- the very first packet needs to be ~21000 bytes in order to get over the header
--   and include at least one valid MP3 frame.
*/
void AudioPlayer::init() {
	if ((player->OpenStream(0, 1, &buffer, 21000, format)) == 0) {
		//error occurred opening the stream
		player->SetCallbackFunc(callBackFunc, (TCallbackMessage) MsgStreamNeedMoreData, 0);
	}
}