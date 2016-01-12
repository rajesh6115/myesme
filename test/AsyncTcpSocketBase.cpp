#include <iostream>
#include <event.h>
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
class AsyncSocketBase{
public:
	AsyncSocketBase(struct event_base *l_event_base, const std::string& host, const unsigned short port);
	int OpenConnection();
	int CloseConnection();
	ssize_t Write(const void *l_buffer, const size_t l_buffer_size);
	virtual void OnDataRecived(const void *l_buffer, const size_t length)=0;
private:
	// private member functions
	static void recv_buffer_handler( evutil_socket_t fd, short events, void *arg);
	
	// private data members
	struct sockaddr_in m_serv_addr;
	struct event_base *m_event_base;
	struct event *m_read_event;
	struct event *m_write_event;
	evutil_socket_t m_sd;
	const std::string m_host;
	const unsigned short m_port;
};

AsyncSocketBase::AsyncSocketBase(struct event_base *l_event_base, const std::string& l_host, const unsigned short l_port):m_event_base(l_event_base), m_host(l_host), m_port(l_port)
{
	m_read_event = NULL;
	m_write_event= NULL;
	m_sd = -1;
	memset(&m_serv_addr, 0x00, sizeof(m_serv_addr));
}

void AsyncSocketBase::recv_buffer_handler( evutil_socket_t fd, short events, void *arg)
{
	AsyncSocketBase *socket_obj_p = (AsyncSocketBase *) arg;
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

int AsyncSocketBase::OpenConnection()
{
	if(m_sd == -1){
		m_sd = socket(AF_INET, SOCK_STREAM, 0);
	}
	if(m_sd < 0){
		return -1;
	}
	evutil_make_socket_nonblocking( m_sd);
	if( m_read_event == NULL){
		m_read_event = event_new(m_event_base, m_sd, EV_READ | EV_PERSIST, &AsyncSocketBase::recv_buffer_handler, this);
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

int AsyncSocketBase::CloseConnection()
{
	if(m_read_event){
		event_del(m_read_event);
		event_free(m_read_event);
		m_read_event = NULL;
	}
	if(m_sd > 0){
		//close(m_sd);
		evutil_closesocket(m_sd);
		m_sd = -1;
	}
	return 0;
}

ssize_t AsyncSocketBase::Write(const void *l_buffer, const size_t l_buffer_size){
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
}


class AsyncTcpSocket: public AsyncSocketBase
{
public:
	AsyncTcpSocket(struct event_base *l_event_base, const std::string& l_host, const unsigned short &l_port ): AsyncSocketBase(l_event_base, l_host, l_port)
{

}
	void OnDataRecived(const void *l_buffer, const size_t l_buffer_size)
	{
		std::cout << "\nData Recived" << this << std::endl;
		std::cout.write((const char *)l_buffer, l_buffer_size);
	}
};

int main( int argc, char *argv[]){
	struct event_base *l_event_base = event_base_new();
	AsyncTcpSocket asynSockObj1(l_event_base, "127.0.0.1", 80);
	AsyncTcpSocket asynSockObj2(l_event_base, "127.0.0.1", 80);
	AsyncTcpSocket asynSockObj3(l_event_base, "127.0.0.1", 80);
	asynSockObj1.OpenConnection();
	asynSockObj2.OpenConnection();
	asynSockObj3.OpenConnection();
	const char *msg = "GET / \n\n\r";
	asynSockObj1.Write(msg, strlen(msg));
	asynSockObj2.Write(msg, strlen(msg));
	asynSockObj3.Write(msg, strlen(msg));
	event_base_dispatch(l_event_base);
	return 0;
}
