#ifndef ASYNC_TCP_SOCKET_HPP
#define ASYNC_TCP_SOCKET_HPP
#include <event.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <cstdlib>
#include <NetBuffer.hpp>
#include <deque>

class AsyncTcpSocket{
public:
	AsyncTcpSocket(struct event_base *l_event_base, const std::string& host, const unsigned short port);
	int OpenConnection();
	int CloseConnection();
	ssize_t Write(const void *l_buffer, const size_t l_buffer_size);
	virtual void OnDataRecived(const void *l_buffer, const size_t length)=0;
private:
	static void read_event_handler( evutil_socket_t fd, short events, void *arg);
	static void write_event_handler( evutil_socket_t fd, short events, void *arg);
	struct sockaddr_in m_serv_addr;
	struct event_base *m_event_base;
	struct event *m_read_event;
	struct event *m_write_event;
	evutil_socket_t m_sd;
	const std::string m_host;
	const unsigned short m_port;
//TODO: 
//1. Try To Remove NetBuffer With std::vector.
// As We Can Implement Some Reservation Of Memory To Avoid new and delete always size got change
	std::deque<NetBuffer> m_write_buffers;
};

#endif

#ifdef ASYNC_TCP_SOCKET_HPP

AsyncTcpSocket::AsyncTcpSocket(struct event_base *l_event_base, const std::string& l_host, const unsigned short l_port):m_event_base(l_event_base), m_host(l_host), m_port(l_port)
{
	m_read_event = NULL;
	m_write_event= NULL;
	m_sd = -1;
	memset(&m_serv_addr, 0x00, sizeof(m_serv_addr));
}

void AsyncTcpSocket::read_event_handler( evutil_socket_t fd, short events, void *arg)
{
	AsyncTcpSocket *socket_obj_p = (AsyncTcpSocket *) arg;
	char l_buffer[4096];
	ssize_t l_buffer_size;
	l_buffer_size = recv(fd, l_buffer, sizeof(l_buffer), 0);
	if(l_buffer_size > 0){
		socket_obj_p->OnDataRecived(l_buffer, l_buffer_size);
	}else{
		// Some Error So Close Socket
		socket_obj_p->CloseConnection();
	}
}

void AsyncTcpSocket::write_event_handler( evutil_socket_t fd, short events, void *arg){
	AsyncTcpSocket *socket_obj_p = (AsyncTcpSocket *) arg;
	static NetBuffer tempObj;
	if(tempObj.GetLength() == 0){
		tempObj = socket_obj_p->m_write_buffers.front();
		socket_obj_p->m_write_buffers.pop_front();
	}
	ssize_t ret_val;
	ret_val = send(fd, tempObj.GetBuffer(), tempObj.GetLength(), 0);
	if(ret_val == tempObj.GetLength()){
		tempObj.Erase();
	}else if (ret_val > 0){
		tempObj.Erase(0, ret_val);
		event_add(socket_obj_p->m_write_event, NULL);
	}else{
		// error
		tempObj.Erase(); // Loss Of Data
		socket_obj_p->CloseConnection();
	}
}

int AsyncTcpSocket::OpenConnection()
{
	if(m_sd == -1){
		m_sd = socket(AF_INET, SOCK_STREAM, 0);
	}
	if(m_sd < 0){
		return -1;
	}
	evutil_make_socket_nonblocking( m_sd);
	if( m_read_event == NULL){
		m_read_event = event_new(m_event_base, m_sd, EV_READ | EV_PERSIST, &AsyncTcpSocket::read_event_handler, this);
	}
	if(m_read_event == NULL){
		return -1;
	}else{
		// We Can Implement TimeOut For Socket Here By replacing NULL with struct timeval
		event_add( m_read_event, NULL);
	}
	m_serv_addr.sin_family = AF_INET;
	m_serv_addr.sin_addr.s_addr = inet_addr(m_host.c_str());
	m_serv_addr.sin_port = htons(m_port);
	return connect( m_sd, (const struct sockaddr *) &m_serv_addr, sizeof(m_serv_addr));
}

int AsyncTcpSocket::CloseConnection()
{
	if(m_read_event){
		event_del(m_read_event);
		event_free(m_read_event);
		m_read_event = NULL;
	}
	if(m_write_event){
		event_del( m_write_event);
		event_free(m_write_event);
		m_write_event = NULL;
	}
	if(m_sd > 0){
		//close(m_sd);
		evutil_closesocket(m_sd);
		m_sd = -1;
	}
	if(!m_write_buffers.empty()){
		// Flush Existing Buffers
		m_write_buffers.erase(m_write_buffers.begin(), m_write_buffers.end());	
	}
	return 0;
}

ssize_t AsyncTcpSocket::Write(const void *l_buffer, const size_t l_buffer_size){
/*
	ssize_t l_ret_val=0;
	ssize_t l_bytes_written=0;
	do{
		l_ret_val = send( m_sd, l_buffer, l_buffer_size, 0);
		if( l_ret_val < 0){
			return -1;
		}else{
			l_bytes_written += l_ret_val;
		}
	}while(l_bytes_written != l_buffer_size);
*/
	if(m_write_event == NULL){
		m_write_event = event_new(m_event_base, m_sd, EV_WRITE, &AsyncTcpSocket::write_event_handler, this);
	}
	NetBuffer tempObj;
	tempObj.Append(l_buffer, l_buffer_size);
	m_write_buffers.push_back(tempObj);
	event_add(m_write_event, NULL);
}

#endif

#include <iostream>
class AsyncSmppSocket: public AsyncTcpSocket
{
public:
	AsyncSmppSocket(struct event_base *l_event_base, const std::string& l_host, const unsigned short &l_port):AsyncTcpSocket(l_event_base, l_host, l_port)
	{
	}
	void OnDataRecived(const void *l_buffer, const size_t l_buffer_size)
        {
                std::cout << "\nData Recived" << this << std::endl;
                std::cout.write((const char *)l_buffer, l_buffer_size);
        }
};

int main (int argc, char *argv[]){
	struct event_base *l_event_base = event_base_new();
	AsyncSmppSocket asynSockObj1(l_event_base, "127.0.0.1", 80);
	asynSockObj1.OpenConnection();
	const char *msg = "GET / \n\n\r";
	asynSockObj1.Write(msg, strlen(msg));
	event_base_dispatch(l_event_base);
	return 0;
}
