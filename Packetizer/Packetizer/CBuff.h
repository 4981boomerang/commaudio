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
-- Circular Buffer class that uses 
------------------------------------------------------------------------------*/
#include <array>
#include <mutex>
#include <memory>

#include "Node.h"
#include "Semaphore.h"

#ifndef CBUFF_H
#define CBUFF_H

#define DEFAULT_SIZE 128 

class CBuff
{

public:
	CBuff();
	~CBuff();

	void empty();
	void push_back(std::string & str);
	std::string & pop();
	const std::string & peek() const;
	
	std::string & operator[] (int x);
	const std::string & operator[] (int x) const;

	unsigned int getCapacity() const;
	bool isReadyForRead(double percentage = 0.5) const;

private:
	int _buffSize;
	int _maxSize;
	int _capacity;
	std::array<std::string, DEFAULT_SIZE> _buff;
	unsigned int _head;
	unsigned int _tail;
	Semaphore _semHead, _semTail;

	unsigned int checkFull() const;
};

#endif 

