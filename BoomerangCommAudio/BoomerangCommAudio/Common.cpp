#include "stdafx.h"
#include "Common.h"

HWND g_hWnd;
HWND g_hMainDlg;
HWND g_hResult;
BOOL g_bIsServer;
char g_IP[STR_SIZE] = "";
unsigned int g_lTotalRecvBytes;
int g_port = DEFAULT_PORT;
BOOL g_bIsFile;
int g_packetSize = DEFAULT_PAC_SIZE;
int g_packetTimes = DEFAULT_PAC_TIMES;
char g_filename[STR_SIZE] = "";

void SaveSongFile(const std::string& fileName, std::vector<std::string>& data)
{
	FILE* file;

	if (fopen_s(&file, fileName.c_str(), "wb") != 0)
	{
		Display("Failed : Open File");
		return;
	}

	for (size_t i = 0; i < data.size(); i++) {
		fwrite(data[i].c_str(), 1, strlen(data[i].c_str()), file);
	}
	fclose(file);
}
