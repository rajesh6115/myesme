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


