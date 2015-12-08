#include <stdio.h>
#include <event.h>
#include <sys/socket.h>
#include <string.h>
#define HOST "127.0.0.1"
#define PORT 80

void socket_event_read_cb(struct bufferevent *bev, void *ptr){
	char buf[1024];
	int n;
	struct evbuffer *input = bufferevent_get_input(bev);
	while ((n = evbuffer_remove(input, buf, sizeof(buf))) > 0) {
		fwrite(buf, 1, n, stdout);
	}
}

void socket_event_cb(struct bufferevent *bev, short events, void *arg){
	if(events & BEV_EVENT_CONNECTED){
		printf("Socket Connected To %s:%d\n", HOST, PORT);
		evbuffer_add_printf(bufferevent_get_output(bev), "GET %s\r\n", "/");
	}else if(events & BEV_EVENT_ERROR){
		printf("ERROR IN CONNECTING");
	}
}

int main(int argc, char *argv[]){
	struct event_base *base = NULL;
	struct bufferevent *bev = NULL;
	struct sockaddr_in sin;
	
	/* get event base */
	base = event_base_new();
	if(base == NULL){
		return -1;
	}
	
	/*Prepare sockaddr_in*/
	memset(&sin, 0x00, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = inet_addr(HOST);
	sin.sin_port = htons(PORT);
	
	/*Get Buffer Socket Event*/
	bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
	if(bev == NULL){
		event_base_free(base);
		return -1;
	}
	
	/*Set Call Back For Events*/
	//bufferevent_setcb(bev, NULL, NULL, socket_event_cb, NULL);
	bufferevent_setcb(bev, socket_event_read_cb , NULL, socket_event_cb, NULL);
	bufferevent_enable(bev, EV_READ|EV_WRITE);
	// bufferevent_setcb(bev, readcb, NULL, eventcb, base);	
	/* Try To Connect with new buffer event*/
	if(bufferevent_socket_connect(bev, (struct sockaddr *)&sin, sizeof(sin))<0){
		/*Error Occured in Connect*/
		bufferevent_free(bev);
		event_base_free(base);
		return -1;
	}
	event_base_dispatch(base);
	event_base_free(base);
	return 0;
}
