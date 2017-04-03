/*------------------------------------------------------------------------------
-- SOURCE FILE: Packetizer.h - SoundFilePacketizer class
--
-- PROGRAM: COMP4985 A4
--
-- FUNCTIONS:
-- 		SoundFilePacketizer(const int packsize = DEFAULT_PACKSIZE)
-- 		~SoundFilePacketizer()
-- 		std::string & getNextPacket()
-- 		void makePacketsFromFile(const char * fpath)
-- 		long getFileSize() 
-- 		long getTotalPackets()
-- 		long getPacketIndex() 
-- 		void openFile(const char * fpath)
-- 		void closeFile()
-- 		void calcFileSize()
--
-- DATE: MAR. 15, 2016
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/MAR/15 - Created Class 

-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- NOTES:
-- This is a packetizer to packetize a sound file
-- note that if you make a new packet from a file, the exiting 
-- storage of information will be destroyed
------------------------------------------------------------------------------*/
#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <memory>

class SoundFilePacketizer
{
	static const int MAX_PACKS = 32;
	static const int DEFAULT_PACKSIZE = 1024;

public:
	SoundFilePacketizer(const int packsize = DEFAULT_PACKSIZE);
	~SoundFilePacketizer();
	std::string getNextPacket();
	void makePacketsFromFile(const char * fpath);
	long getFileSize() { return filesize; };
	long getTotalPackets();
	long getPacketIndex() { return packindex; };

private:
	void openFile(const char * fpath);
	void closeFile();
	void calcFileSize();

	std::vector <std::string> vPack;
	FILE * fp; // file pointer 
	long filesize; // total size of file 
	long packsize; // size of each packet 
	long packindex; // index of the packet you are at 
};