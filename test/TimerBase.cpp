#include <event.h>
#include <iostream>

class TimerBase
{
public:
	TimerBase(struct event_base *l_event_base):m_event_base(l_event_base)
	{
		//event_new((b), -1, 0, (cb), (arg))
		m_timer_event = event_new(m_event_base, -1, EV_PERSIST | EV_TIMEOUT, &TimerBase::TimeOutHandler,this);
		// Default Timeout to 1 Sec
		m_time_val.tv_sec = 1; 
		m_time_val.tv_usec = 0;
	}
	void Start(void);
	void Stop(void);
	void SetTimeout( const unsigned long& sec, const unsigned long& usec);
	virtual void OnTimeOut(void)=0;
private:
	static void TimeOutHandler(int, short, void *);
	struct event_base *m_event_base;
	struct event *m_timer_event;
	struct timeval m_time_val;
};

void TimerBase::TimeOutHandler( int fd, short what, void *arg){
	if(arg){
		TimerBase *childptr = (TimerBase *) arg;
		childptr->OnTimeOut();
	}
}

void TimerBase::Start(void){
	if( m_event_base && m_timer_event){
		evtimer_add( m_timer_event, &m_time_val);
	}
}

void TimerBase::Stop(void){
	if( m_event_base && m_timer_event){
		evtimer_del( m_timer_event);
	}
}

void TimerBase::SetTimeout( const unsigned long& sec, const unsigned long& usec ){
	m_time_val.tv_sec = sec;
	m_time_val.tv_usec = usec;
}

class Timer: public TimerBase
{
public:
	Timer(struct event_base *l_event_base):TimerBase(l_event_base)
	{
	}
	void OnTimeOut(void){
		std::cout << "Timer Got Time Out" << this << std::endl;
	}
};


int main(int argc, char *argv[]){
	struct event_base *l_event_base = event_base_new();
	Timer tobj(l_event_base);
	tobj.Start();
	event_base_dispatch(l_event_base);
	return 0;
}
