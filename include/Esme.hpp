#ifndef _ESME_H_
#define _ESME_H_
#include <iostream>
#include <cstring>
#include <map>
#include <queue>
#include <pthread.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "NetBuffer.hpp"
#include "smpp.hpp"
#include "Externs.hpp"

#define DFL_USLEEP_VALUE 500000
#define DFL_SLEEP_VALUE 3
#define DFL_SMPP_URL "127.0.0.1"
#define DFL_SMPP_PORT 2775

#define BIND_RDONLY 1
#define BIND_WRONLY 2
#define BIND_RDWR 3

class Esme{
	public: 
		enum STATE{
			ST_IDLE,
			ST_CONNECTED,
			ST_DISCONNECTED,
			ST_BIND,
			ST_BIND_FAIL,
			ST_UNBIND,
			ST_UNBIND_FAIL,
		};
	private:
		// smsc details 
		std::string host;
		int port;
		int smscSocket;
		struct sockaddr_in smscInfo;
		enum STATE state;
		bool isLive;
		// Data structures to maintain temp sms // avoid db interaction
		std::map<int64_t, sms_data_t> sendSmsMap;
		std::map<int64_t, sms_data_t> receiveSmsMap;
		// Register for Pdu So that Retransmissiion can happen
		std::map<uint32_t, NetBuffer> pduRegister;
		int RegisterPdu(NetBuffer &tmpNetBuf);
		int UnRegisterPdu(NetBuffer &tmpNetBuf);	
		// Data Base Update/Insert	
		int DoDatabaseUpdate(NetBuffer &tmpNetBuf);
		//
		std::queue<sms_data_t> sendQueue;
		std::queue<NetBuffer> receiveQueue;
		// Sending Thread
		
		static void *ThSmsSender(void *);
		pthread_t sendThId;
		thread_status_t sendThStatus;
		// Receiving thread
		pthread_t rcvThId;
		thread_status_t rcvThStatus;
		static void *ThSmsReader(void *);
		// Link Thread 
		pthread_t linkThId;
		thread_status_t linkThStatus;
		static void *LinkCheckThread(void *);
		int SendEnquireLink(void);
		// OnReceivePduThread
		pthread_t pduProcessThId;
		thread_status_t pduProcessThStatus;
		static void *ThOnReceivePdu(void *);
		// Sequence Number
		static Smpp::Uint32 esmePduSequenceNum;
	public:
		~Esme(void);
		Esme(std::string host=DFL_SMPP_URL, uint32_t port=DFL_SMPP_PORT);
		Esme(const Esme&);
		Esme &operator=(const Esme&);
		Smpp::Uint32 GetNewSequenceNumber(void);
		int OpenConnection(std::string host=DFL_SMPP_URL, uint32_t port=DFL_SMPP_PORT);
		int CloseConnection(void);
		enum STATE GetEsmeState(void);
		int Write(NetBuffer &); // Just Write all Bytes
		int Read(NetBuffer &); // Frammer will be implemented here 
		int Bind(Smpp::SystemId sysId, Smpp::Password pass, uint8_t bindType);
		int SendSubmitSm(const Smpp::Char *srcAddr, const Smpp::Char *destAddr, uint8_t type, const Smpp::Uint8 *sms, Smpp::Uint32 length);
		int UnBind(void);
		int StartSender(void);
		int StopSender(void);
		int StartReader(void);
		int StopReader(void);
		int StartPduProcess(void);
		int StopPduProcess(void);
		int StartLinkCheck(void);
		int StopLinkCheck(void);
		thread_status_t GetSenderThStatus(void);
		thread_status_t GetPduProcessThStatus(void);
		thread_status_t GetRcvThStatus(void);
		thread_status_t GetEnquireLinkThStatus(void);
		int SendSms(sms_data_t sms);
};
#endif

