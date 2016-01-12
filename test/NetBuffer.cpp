#include "NetBuffer.hpp"

NetBuffer::NetBuffer(){
	m_length = 0;
	m_pbuffer = NULL;	
}

NetBuffer::NetBuffer(const NetBuffer &robj){
	if(robj.m_length){
		m_length = robj.m_length;
		m_pbuffer = new char[m_length];
		memcpy(m_pbuffer, robj.m_pbuffer, m_length);
	}else{
		m_length = 0;
		m_pbuffer = NULL;
	}
}

NetBuffer::~NetBuffer(){
	if(this->m_length !=0){
		if(this->m_pbuffer){
			delete []m_pbuffer;
		}
		m_length = 0;
		m_pbuffer = NULL;
	}
}

void NetBuffer::PrintHexDump(void){
	unsigned char ch;
	printf("[");
	for(unsigned int i=0; i < this->m_length; i++){
		ch = this->m_pbuffer[i];
		printf("%02X",ch);
	}
	printf("]");

}

void NetBuffer::Append(const void *buf, unsigned int bufferLength){
	const char *buffer = (const char *) buf;
	if((bufferLength == 0) || (buffer == NULL)){
		return;
	}
	else if(this->m_length){
		char *tempBuf=NULL;
		unsigned int tempLen= this->m_length + bufferLength;
		tempBuf = this->m_pbuffer;
		this->m_pbuffer = new char[tempLen];
		memcpy(this->m_pbuffer, tempBuf, this->m_length);
		memcpy(this->m_pbuffer + this->m_length, buffer, bufferLength);
		this->m_length = tempLen;
		delete []tempBuf;
		
	}else{
		this->m_length = bufferLength;
		this->m_pbuffer = new char[this->m_length];
		memcpy(this->m_pbuffer, buffer, this->m_length);
	}
}

NetBuffer& NetBuffer::operator=(const NetBuffer &robj){
	if(robj.m_length){
		Erase();
		this->m_length = robj.m_length;
		this->m_pbuffer = new char[this->m_length];
		memcpy(this->m_pbuffer, robj.m_pbuffer, this->m_length);
	}else{
		Erase();
	}	
	return (*this);
}

const char *NetBuffer::GetBuffer(void) const {
	return this->m_pbuffer;
}

int NetBuffer::GetLength(void){
	return this->m_length;
}

void NetBuffer::Erase(void){
	if(this->m_length != 0){
		if(this->m_pbuffer){
			delete[]m_pbuffer;
		}
		this->m_pbuffer=NULL;
		this->m_length=0;
	}
}
/**
* Erase buffer starting fron begin upto end
* begin should less than end
*/
void NetBuffer::Erase(unsigned int begin, unsigned int end){
	if(this->m_length != 0){
		if(begin >= end ){
			return;
		}
		if(this->m_length > begin && this->m_length >= end){
			int tempLen = this->m_length - (end - begin);
			char *tempBuf = new char[tempLen];
			if(begin != 0){
				memcpy(tempBuf, this->m_pbuffer,begin);
			}
			if( end != this->m_length){
				memcpy(tempBuf+begin, this->m_pbuffer+end, tempLen-begin);
			}
			Erase();
			Append(tempBuf, tempLen);
			delete[]tempBuf;
		}
	}
}
