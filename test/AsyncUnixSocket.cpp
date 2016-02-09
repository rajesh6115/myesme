#include <iostream>
#include <vector>
#include <event.h>
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h> 
class AsyncUnixSocket{
public:
	AsyncUnixSocket(struct event_base *l_event_base, const std::string& host);
	int OpenConnection();
	int CloseConnection();
	ssize_t Write(const void *l_buffer, const size_t l_buffer_size);
	virtual void OnDataRecived(const void *l_buffer, const size_t length)=0;
private:
	// private member functions
	static void recv_buffer_handler( evutil_socket_t fd, short events, void *arg);
	static void send_event_handler( evutil_socket_t fd, short events, void *arg);
	
	// private data members
	struct sockaddr_un m_serv_addr;
	struct event_base *m_event_base;
	struct event *m_read_event;
	struct event *m_write_event;
	evutil_socket_t m_sd;
	const std::string m_host;
	std::vector<uint8_t> m_write_buffer;
};

AsyncUnixSocket::AsyncUnixSocket(struct event_base *l_event_base, const std::string& l_host):m_event_base(l_event_base), m_host(l_host)
{
	m_read_event = NULL;
	m_write_event= NULL;
	m_sd = -1;
	memset(&m_serv_addr, 0x00, sizeof(m_serv_addr));
}

void AsyncUnixSocket::recv_buffer_handler( evutil_socket_t fd, short events, void *arg)
{
	AsyncUnixSocket *socket_obj_p = (AsyncUnixSocket *) arg;
	char l_buffer[4096];
	ssize_t l_buffer_size;
	l_buffer_size = recv(fd, l_buffer, sizeof(l_buffer), 0);
	if(l_buffer_size > 0){
		socket_obj_p->OnDataRecived(l_buffer, l_buffer_size);
	}else{
		socket_obj_p->CloseConnection();
	}
}

void AsyncUnixSocket::send_event_handler( evutil_socket_t fd, short events, void *arg){
	AsyncUnixSocket *socket_obj_p = (AsyncUnixSocket *) arg;
        ssize_t l_ret_val;
	l_ret_val = send( fd, &socket_obj_p->m_write_buffer[0], socket_obj_p->m_write_buffer.size(), 0);
	if(l_ret_val == socket_obj_p->m_write_buffer.size()){
                socket_obj_p->m_write_buffer.clear();
        }else if( l_ret_val > 0){
                socket_obj_p->m_write_buffer.erase(socket_obj_p->m_write_buffer.begin(), socket_obj_p->m_write_buffer.begin()+l_ret_val);
                event_add(socket_obj_p->m_write_event, NULL);
        }
	/*else{
                // error condition 
		return;
        }
	*/
}

int AsyncUnixSocket::OpenConnection()
{
	if(m_sd == -1){
		m_sd = socket(AF_UNIX, SOCK_STREAM, 0);
	}
	if(m_sd < 0){
		return -1;
	}
	evutil_make_socket_nonblocking( m_sd);
	if( m_read_event == NULL){
		m_read_event = event_new(m_event_base, m_sd, EV_READ | EV_PERSIST| EV_ET, &AsyncUnixSocket::recv_buffer_handler, this);
	}
	if(m_read_event == NULL){
		return -1;
	}else{
		event_add( m_read_event, NULL);
	}
	m_serv_addr.sun_family = AF_UNIX;
	strncpy(m_serv_addr.sun_path, m_host.c_str(), m_host.length());
	return connect( m_sd, (const struct sockaddr *) &m_serv_addr, sizeof(m_serv_addr));
}

int AsyncUnixSocket::CloseConnection()
{
	if(m_sd > 0){
		evutil_closesocket(m_sd);
		m_sd = -1;
	}

	if(m_read_event){
		//event_del(m_read_event); // giving SEGFAULT . FIND proper way to delete pending events
		event_free(m_read_event);
		m_read_event = NULL;
	}
	if(m_write_event){
                event_free(m_write_event);
                m_write_event = NULL;
        }

	return 0;
}

ssize_t AsyncUnixSocket::Write(const void *l_buffer, const size_t l_buffer_size){
	if(m_write_event == NULL){
                m_write_event = event_new(m_event_base, m_sd, EV_WRITE, &AsyncUnixSocket::send_event_handler, this);
        }
	ssize_t l_ret_val=0;
	ssize_t l_bytes_written=0;
	l_ret_val = send( m_sd, l_buffer, l_buffer_size, 0);
	if(l_ret_val == l_buffer_size){
		return l_ret_val;
	}else if( l_ret_val > 0){
		m_write_buffer.insert(m_write_buffer.end(),(const char *) l_buffer+l_ret_val, (const char *) l_buffer+(l_buffer_size - l_ret_val));
		event_add(m_write_event, NULL);
	}else{
		if(l_ret_val == EAGAIN || l_ret_val == EWOULDBLOCK){
			return 0;
		}
		return -1;
	}
}


class AsyncTcpSocket: public AsyncUnixSocket
{
public:
	AsyncTcpSocket(struct event_base *l_event_base, const std::string& l_host ): AsyncUnixSocket(l_event_base, l_host)
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
	AsyncTcpSocket asynSockObj1(l_event_base, "\0hidden");
	if(asynSockObj1.OpenConnection() == -1){
		std::cerr << "Error In Opening Unix Socket" << std::endl;
		return -1;
	}
	const char *msg = "GET / \n\n\r";
	asynSockObj1.Write(msg, strlen(msg));
	event_base_dispatch(l_event_base);
	return 0;
}
