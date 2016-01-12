/*
 * Client Socket Using libevent
 */

#include <stdio.h>
#include <event.h>

void say_hello(int fd, short event, void *arg)
{
	static unsigned long sec=0;
	fprintf(stdout, "%02ld ", sec++);
	fflush(stdout);
}


int main(int argc, char *argv[]){
	printf("libevent Learning \n");
	//1. Allocate A Event Base Structure using event_base_new(void);
	struct event_base *l_ev_base;
	l_ev_base = event_base_new();
	if(l_ev_base == NULL){
		fprintf(stderr, "Failed To Create event_base structure\n");
		return -1;
	}
	//2. Create A event structure 
	struct event *ev;
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	ev = event_new(l_ev_base, -1, EV_PERSIST, say_hello, NULL);
	//ev = evtimer_new(l_ev_base, say_hello, NULL);
	if(ev == NULL){
		fprintf(stderr, "Failed To Allocate a Event\n");
		event_base_free(l_ev_base);
		return -1;
	}
	evtimer_add(ev, &tv);	
	event_base_set(l_ev_base, ev);
	printf("Using Libevent with backend method %s.", event_base_get_method(l_ev_base));	
	event_base_dispatch(l_ev_base);
	if(ev)
		event_free(ev);
	event_base_free(l_ev_base);
	return 0;
}
