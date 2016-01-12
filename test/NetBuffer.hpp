#ifndef _NETBUFFER_H_
#define _NETBUFFER_H_
#include <iostream>
#include <cstdio>
#include <cstring>

class NetBuffer{
	private:
	char* m_pbuffer;
	unsigned int m_length;
	public:
	NetBuffer();
	NetBuffer(const NetBuffer &robj);
	~NetBuffer();
	void PrintHexDump(void);
	void Append(const void *buf, unsigned int bufferLength);
	const char *GetBuffer(void) const;
	int GetLength(void);
	void Erase(void);
	void Erase(unsigned int begin, unsigned int end);
	NetBuffer& operator=(const NetBuffer &robj);
};

#endif

