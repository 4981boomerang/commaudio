#include "stdafx.h"
#include "RecvRoutine.h"
#include "Common.h"

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
				//ReqDownloadSong songData;
				//memcpy(&songData, SI->Buffer, sizeof(ReqDownloadSong));

				//SoundFilePacketizer packer(PACKET_SIZE);
				//packer.makePacketsFromFile(songData.filename);
				//long totalNumberOfPackets = packer.getTotalPackets();
				//int lastPacketSize = packer.getLastPackSize();

				//SI->BytesSEND = 0;
				//SI->SentBytesTotal = 0;
				////send all packets except for last one
				//for (int i = 0; i < totalNumberOfPackets - 1; i++)
				//{
				//	SI->DataBuf.buf = packer.getNextPacket();
				//	SI->DataBuf.len = PACKET_SIZE;
				//	Server::SendTCP(SI->Socket, SI);
				//}

				//SI->DataBuf.buf = packer.getNextPacket();
				//SI->DataBuf.len = lastPacketSize;
				//Server::SendTCP(SI->Socket, SI);

				//char complete[] = "EndOfPacket";
				//SI->DataBuf.buf = complete;
				//SI->DataBuf.len = strlen(complete) + 1;
				//Server::SendTCP(SI->Socket, SI);
			}
			break;

			default:
				break;
			}
		}
	}
}