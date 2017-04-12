/*------------------------------------------------------------------------------
-- SOURCE FILE: CBuff.h - CircularBuffer Class
--
-- PROGRAM: COMP4985 A4
--
-- FUNCTIONS:
--  PUBLIC:
-- 		CBuff()
-- 		~CBuff()
-- 		void empty()
-- 		void push_back(std::string & str)
-- 		std::string & pop()
-- 		const std::string & peek() const
-- 		std::string & operator[] (int x)
-- 		const std::string & operator[] (int x) const
-- 		unsigned int getCapacity() const
-- 		bool isReadyForRead(double percentage = 0.5) const
--  PRIVATE:
-- 		unsigned int checkFull()
--
-- DATE: MAR. 12, 2016
--
-- REVISIONS:
-- Version 1.0 - [EY] - 2016/MAR/12 - Create Class
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- NOTES:
-- Circular Buffer class that uses char * arrays
------------------------------------------------------------------------------*/
#pragma once
#include <array>
#include <mutex>
#include <memory>

#include "Semaphore.h"

//#ifndef CBUFF_H
//#define CBUFF_H
//#define DEFAULT_CBUFF_SIZE 128
#define DEFAULT_CBUFF_SIZE 2048 // luxes
//#define DEFAULT_CHARARR_SIZE 2048
#define DEFAULT_CHARARR_SIZE 2048 // luxes

class CBuff
{

public:
	CBuff();
	~CBuff();

	void empty();
	void push_back(char * str);
	char * pop();
	unsigned int getCapacity() const { return _capacity; };
	bool isReadyForRead(double percentage = 0.5) const;

private:
	int _buffSize;
	int _maxSize;
	int _capacity;
	std::array<char[DEFAULT_CHARARR_SIZE], DEFAULT_CBUFF_SIZE> _buff;
	unsigned int _head;
	unsigned int _tail;
	Semaphore _semHead, _semTail;

	unsigned int checkFull() const;
};
//
//#endif 

