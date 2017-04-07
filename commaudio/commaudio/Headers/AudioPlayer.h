#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include "libzplay.h"
#include "UI.h"
#include "CBuff.h"
#include "Main.h"

#define BUFSIZE 1024

class AudioPlayer {
public:
	
	static AudioPlayer& instance() {
		return aInstance;
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
	libZPlay::ZPlay * createPlayer();

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

	/*--------------------------------------------------------------------------------------------
	-- FUNCTION:   getBuf
	--
	-- DATE:       April 6, 2017
	--
	-- DESIGNER:   Michael Goll, Aing Ragunathan, Eva Yu, Jamie Lee
	--
	-- PROGRAMMER: Michael Goll
	--
	-- INTERFACE:  char * pop()
	--
	-- PARAMETER:  none
	--
	-- RETURNS:    CBuff& - returns a reference to the player's circular buffer.
	--
	-- NOTES:      Used to get a reference to the circular buffer inside of the player.
	--------------------------------------------------------------------------------------------*/
	CBuff& getBuf() {return cbuff;}

	/*--------------------------------------------------------------------------------------------
	-- FUNCTION:   pop
	--
	-- DATE:       April 6, 2017
	--
	-- DESIGNER:   Michael Goll, Aing Ragunathan, Eva Yu, Jamie Lee
	--
	-- PROGRAMMER: Michael Goll
	--
	-- INTERFACE:  char * pop()
	--
	-- PARAMETER:  none
	--
	-- RETURNS:    char * - the data string containing the music information.
	--
	-- NOTES:      Pops the head element of the circular buffer into the player's buffer
	--             to be used by LibZPlay internally.
	--------------------------------------------------------------------------------------------*/
	char * pop();
	
private:
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

	static AudioPlayer aInstance;
	libZPlay::ZPlay * player;
	libZPlay::TStreamFormat format;
	//internal buffer for LibZPlay to use
	char buffer[BUFSIZE];
	//circular buffer for application to use
	CBuff cbuff;
};

#endif

