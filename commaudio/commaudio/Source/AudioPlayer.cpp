#include <cstdlib>
#include <stdio.h>

#include "../Headers/AudioPlayer.h"

using namespace libZPlay;

ZPlay * AudioPlayer::player = nullptr;

AudioPlayer::AudioPlayer() {
	createPlayer();
}

AudioPlayer::~AudioPlayer() {
	if (player)
		player->Release();
}

bool AudioPlayer::createPlayer() {
	if (player = CreateZPlay()) {

	}
	
	return false;
}

ZPlay * AudioPlayer::getPlayer() {
	return player;
}

void AudioPlayer::play() {
	TStreamStatus status;
	player->Play();

	//while it's playing
	while (true) {
		player->GetStatus(&status);

		//Error occurs during playback
		if (!status.fPlay)
			break;


	}
}

void AudioPlayer::stop() {
	player->Stop();
}

int __stdcall callbackFunc(void * instance, void * userData, TCallbackMessage message, unsigned int param1, unsigned int param2) {

	switch (message) {
	case MsgStreamNeedMoreData:
		//player needs more data, grab from buffer
		//player->PushDataToStream(buffer, read);
		break;
	}

	return 0;
}