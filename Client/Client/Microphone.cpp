#include "Microphone.h"

using namespace libZPlay;

DWORD WINAPI microphoneStart(LPVOID lpParam) {
	ZPlay* player;

	player = CreateZPlay();

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

	// set wave output to disk file, mp3 encoding, no playback
	if (player->SetWaveOutFile(OUTPUT_FILE, sfMp3, 0) == 0)	{
		perror("Microphone: ouput file failed!");
		player->Release();
		return 0;
	}

	// start playing
	if (player->Play() == 0) {
		perror("Microphone: Play failed!");
		player->Release(); // delete ZPlay class
		return 0;
	}

	return 0;
}
