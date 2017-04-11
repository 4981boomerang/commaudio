#pragma comment(lib, "libzplay.lib")

#define WIN32_LEAN_AND_MEAN

#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H


#include "libzplay.h"
#include "UI.h"
#include "CBuff.h"
#include "Main.h"
#include "Network.h"

#define BUFSIZE 1024

extern commonResources common;
	
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
	libZPlay::ZPlay * createPlayer();


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
	void play(HWND, libZPlay::ZPlay *);

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
	void pause(HWND, libZPlay::ZPlay *);

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
	void stop(libZPlay::ZPlay *);

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
	void init(libZPlay::ZPlay *);

#endif

