#include "stdafx.h"
#include "RecvRoutine.h"
#include "Common.h"
#include "Packetizer.h"
#include "Server.h"

#include <iostream>
#include <fstream>

void RecvRoutine::RunThread()
{
	char	*bp, buf[BUF_SIZE];
	int bytes_to_read, n;

	bp = buf;
	bytes_to_read = BUF_SIZE;

	char temp[STR_SIZE];
	while (true)
	{
		while ((n = recv(socket, bp, bytes_to_read, 0)) < BUF_SIZE)
		{
			bp += n;
			bytes_to_read -= n;
			if (n == 0)
				break;
		}

		if (isFile)
		{
			std::string recvFileData(buf);
			size_t found = recvFileData.find("EndOfPacket");
			if (found != std::string::npos)
			{
				isFile = false;

				fwrite(buf, 1, found, file);
				fclose(file);
				file = NULL;

				sprintf_s(temp, STR_SIZE, "Save a song file: %s", fileName.c_str());
				Display(temp);
			}
			else
			{
				fwrite(buf, 1, PACKET_SIZE, file);
			}

			sprintf_s(temp, STR_SIZE, "default: length=%d", PACKET_SIZE);
			Display(temp);
		}
		else
		{
			int header;
			memcpy(&header, buf, sizeof(int));
			switch (header)
			{
			case PH_REQ_UPLOAD_SONG:
			{
				isFile = true;
				ReqUploadSong songData;
				memcpy(&songData, buf, sizeof(ReqUploadSong));
				fileName = std::string(songData.filename);

				fopen_s(&file, fileName.c_str(), "wb");

				sprintf_s(temp, STR_SIZE, "Upload - file: %s, title: %s, artist: %s",
					songData.filename, songData.title, songData.artist);
				Display(temp);
			}
			break;

			case PH_REQ_DOWNLOAD_SONG:
			{
				char sbuf[BUF_SIZE];
				memset((char *)sbuf, 0, sizeof(sbuf));

				ReqDownloadSong songData;
				memcpy(&songData, buf, sizeof(ReqDownloadSong));

				SoundFilePacketizer packer(PACKET_SIZE);
				packer.makePacketsFromFile(songData.filename);
				long totalNumberOfPackets = packer.getTotalPackets();
				int lastPacketSize = packer.getLastPackSize();

				LPSOCKET_INFORMATION SocketInfo;
				if ((SocketInfo = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR,
					sizeof(SOCKET_INFORMATION))) == NULL)
				{
					sprintf_s(temp, STR_SIZE, "GlobalAlloc() failed with error %d", GetLastError());
					Display(temp);
					return;
				}
				// Fill in the details of our accepted socket.
				SocketInfo->Socket = socket;
				ZeroMemory(&(SocketInfo->Overlapped), sizeof(OVERLAPPED));
				SocketInfo->BytesSEND = 0;
				SocketInfo->BytesRECV = 0;
				SocketInfo->DataBuf.len = BUF_SIZE;
				SocketInfo->DataBuf.buf = SocketInfo->Buffer;

				for (int i = 0; i < totalNumberOfPackets - 1; i++)
				{
					memset((char *)sbuf, 0, sizeof(sbuf));
					memcpy(sbuf, packer.getNextPacket(), PACKET_SIZE);
					send(socket, sbuf, BUF_SIZE, 0);
				}

				memcpy(sbuf, packer.getNextPacket(), lastPacketSize);
				send(socket, sbuf, BUF_SIZE, 0);

				char complete[] = "EndOfPacket";
				memset((char *)sbuf, 0, sizeof(sbuf));
				memcpy(sbuf, complete, strlen(complete));
				send(socket, sbuf, BUF_SIZE, 0);

				sprintf_s(temp, STR_SIZE, "Send a song file: %s", songData.filename);
				Display(temp);
				
				GlobalFree(SocketInfo);
			}
			break;

			default:
				break;
			}
		}
	}
}