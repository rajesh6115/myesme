#include <iostream>
#include <cstring>
#include <sstream>

#include <sys/socket.h>
#include <event.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define HOST "127.0.0.1"
#define PORT 80

class AsyncTcpSocket{
public:
	AsyncTcpSocket(struct event_base *base, std::string name);
	int Connect(void);
	static void ReadCallBack(struct bufferevent *bev, void *ptr);
	static void EventCallBack(struct bufferevent *bev, short events, void *arg);
	int Open(std::string host, uint16_t port);
private:
	struct event_base *m_base;
	struct bufferevent *m_bufferEvent;
	struct sockaddr_in m_servAddr;
	std::string m_host;
	uint16_t m_port;
	std::string m_name;
};

AsyncTcpSocket::AsyncTcpSocket(struct event_base *base, std::string name):m_base(base),m_name(name){
	memset(&m_servAddr, 0x00, sizeof(m_servAddr));
}

int AsyncTcpSocket::Open(std::string host, uint16_t port){
	m_host = host;
	m_port = port;
	m_bufferEvent = bufferevent_socket_new(m_base, -1, BEV_OPT_CLOSE_ON_FREE);
	if(m_bufferEvent == NULL){
		return -1;
	}
	m_servAddr.sin_family = AF_INET;
	m_servAddr.sin_addr.s_addr = inet_addr(m_host.c_str());
	m_servAddr.sin_port = htons(m_port);
	bufferevent_setcb(m_bufferEvent, &AsyncTcpSocket::ReadCallBack, NULL, &AsyncTcpSocket::EventCallBack, this );
	bufferevent_enable(m_bufferEvent, EV_READ|EV_WRITE);
	return 0;
}

int AsyncTcpSocket::Connect(void){
	if(bufferevent_socket_connect(m_bufferEvent, (struct sockaddr *)&m_servAddr, sizeof(m_servAddr)) < 0){
		bufferevent_free(m_bufferEvent);
		return -1;
	}
	return 0;
}

void AsyncTcpSocket::ReadCallBack(struct bufferevent *bev, void *ptr){
	char buf[1024];
	int n;
	struct evbuffer *input = bufferevent_get_input(bev);
	while ((n = evbuffer_remove(input, buf, sizeof(buf))) > 0) { 
		fwrite(buf, 1, n, stdout);
	}
	fprintf(stdout, "##################%s#########\n", ((AsyncTcpSocket*)ptr)->m_name.c_str());
}


void AsyncTcpSocket::EventCallBack(struct bufferevent *bev, short events, void *arg){
	if(events & BEV_EVENT_CONNECTED){
		printf("Connected %s:%u FOR %s \n", ((AsyncTcpSocket *)arg)->m_host.c_str(), ((AsyncTcpSocket *)arg)->m_port, ((AsyncTcpSocket *)arg)->m_name.c_str());
		evbuffer_add_printf(bufferevent_get_output(bev), "GET %s\r\n", "/");
	}else if(events & BEV_EVENT_ERROR){
		printf("Error in Connection : %s\n",((AsyncTcpSocket *) arg)->m_name.c_str());
	}
}

int main(int argc, char *argv[]){
	std::cout << "Hello Async Socket Programming" <<std::endl;
	struct event_base *base = NULL;
	base = event_base_new();
	if(base == NULL){ 
		return -1;
	}
	AsyncTcpSocket *objp;
	std::ostringstream oss;
	for(int i=0; i <10; i++){
		oss.str("");
		oss.clear();
		oss << "object";
		oss << i;
		objp = new AsyncTcpSocket(base, oss.str());
		objp->Open(HOST, PORT);
		objp->Connect();
	}
	event_base_dispatch(base);
	event_base_free(base);
	return 0;
}
