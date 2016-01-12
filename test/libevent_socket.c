#include <stdio.h>
#include <event.h>
#include <stdlib.h>
#include <string.h>
void do_read_from_socket_cb(evutil_socket_t fd, short events, void *arg){
	char buffer[256]={0x00};
	int ret;
	ret = recv(fd, buffer, sizeof(buffer), 0);
	if(ret > 0){
		printf("Some Data Are Present to Read From Socket\n");
		printf("MSG FROM SERVER: \n%s\n", buffer);
	}else{
		// Some Error Close The Connection 
		printf("Error In Reading Further\n");
		close(fd);
	}
}

int main(int argc, char *argv[]){
	printf("Client Socket Implementation Using libevent\n");
	struct sockaddr_in serveraddr;
	// Create Event Base
	struct event_base *l_event_base = event_base_new();
	//1. define socket descriptor
	evutil_socket_t tcp_fd;
	//2. create socket
	tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
	// 3. Make Non Blocking Socket
	evutil_make_socket_nonblocking( tcp_fd);
	// Create Event For Handling Reading On Socket
	struct event *read_event = event_new(l_event_base, tcp_fd, EV_READ|EV_PERSIST, do_read_from_socket_cb, NULL);
	// Add Read Event
	event_add(read_event, NULL);
	memset(&serveraddr, 0x00, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serveraddr.sin_port = htons(80);
	connect(tcp_fd, (const struct sockaddr *)&serveraddr, sizeof(serveraddr));
	char *msg = "GET / \n\n\r";
	write(tcp_fd, msg, strlen(msg));
	printf("Evtering Event Loop\n");
	event_base_dispatch(l_event_base);
	//. Connect 
	// Close
	return 0;
}
