#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include "libzplay.h"
#include "UI.h"

class AudioPlayer {
public:
	AudioPlayer();
	~AudioPlayer();

	bool createPlayer();
	libZPlay::ZPlay * getPlayer();
	void play();
	void stop();
	
private:
	static libZPlay::ZPlay * player;
};

#endif

