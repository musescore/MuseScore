#ifndef _MEM_H
#define _MEM_H

// Abstract Memory handler superclass for all memory handler objects
class MemoryHandler {
public:
	virtual void Write(unsigned int addr, unsigned char data) = 0;
	virtual unsigned char Read(unsigned int addr) = 0;
	virtual void Reset() = 0;
};

#endif // _MEM_H
