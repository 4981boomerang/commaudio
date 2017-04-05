#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include "libzplay.h"
#include "UI.h"

class AudioPlayer {
public:
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
	AudioPlayer();

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
	~AudioPlayer();

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
	bool createPlayer();

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
	libZPlay::ZPlay * getPlayer();

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
	void play();

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
	void pause();

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
	void stop();

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
	void init();
	
private:
	static libZPlay::ZPlay * player;

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
	int __cdecl callbackFunc(void *, void *, libZPlay::TCallbackMessage, unsigned int, unsigned int);
};

#endif

