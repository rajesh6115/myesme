#ifndef _ESME_H_
#define _ESME_H_
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <iostream>
#include <cstring>
#include <map>
#include <queue>
#include <pthread.h>
// all defines will be in one File
#include <Defines.hpp>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iconv.h>
#include "EsmeConfig.hpp"
#include "NetBuffer.hpp"
#include "smpp.hpp"
#include "Externs.hpp"
#ifdef _MYSQL_UPDATE_
#include "MySqlWrapper.hpp"
#endif

#ifdef _MESSAGE_QUEUE_UPDATE_
#include <MessageQueue.hpp>
#endif
#include <mutex>
#ifdef WITH_LIBEVENT
#include <event.h>
#endif
class Esme{
	public:
		typedef enum ESME_STATE{
			ST_IDLE=0,
			ST_NOTCONNECTED=1,
			ST_CONNECTED=2,
			ST_BIND_REQ=3,
			ST_BIND=4,
			ST_BIND_FAIL=5,
			ST_UNBIND_REQ=6,
			ST_UNBIND=7,
			ST_UNBIND_FAIL=8,
			ST_ERROR=9,
		}esme_state_t;
	private:
#ifdef _MYSQL_UPDATE_
		CMySQL m_sqlobj;
#endif

		// smsc details
		std::string m_cfgFile;
		EsmeConfig m_smppCfg;
		std::string m_esmeid; // Readable Name For Esme Instance
		std::string m_host;
		uint16_t m_port;
#ifdef WITH_LIBEVENT
		struct event_base *m_base;
		struct bufferevent *m_bufferEvent;
		struct event *m_linkCheckEvent;
		struct timeval m_linkCheckTimerValue;
#else
		int m_esmeSocket;
		// Regulated Instance Creation
		// Variable Need To Make Multi socket Reading
		static fd_set m_readfdset;
		static int m_max_fd;
#endif
		struct sockaddr_in m_smscInfo;
		esme_state_t m_esmeState;
		bind_type_t m_esmeType;
		uint32_t m_esmeTps;
		account_type_t m_esmeAccountType;
		Smpp::SystemId m_sysId;
		Smpp::Password m_sysPass;
		Smpp::SystemType m_sysType;
		Smpp::InterfaceVersion m_IfVersion;
		Smpp::AddressRange m_addrRange;
		Smpp::Ton m_esmeTon;
		Smpp::Npi m_esmeNpi;
		Smpp::RegisteredDelivery m_registerDeliveryReport;
		bool m_isSendSms;
		bool m_isLive;
		// Data structures to maintain temp sms // avoid db interaction
		std::map<uint32_t, sms_data_t> sendSmsMap;
		std::mutex m_SendSmsMapLock;
		int RegisterSmsData(uint32_t ,sms_data_t);
		int UnRegisterSmsData(uint32_t);
		// TODO: For What receiveSmsMap Required?
		std::map<int32_t, sms_data_t> receiveSmsMap;
		// Data Base Update/Insert	
		int DoDatabaseUpdate(NetBuffer &tmpNetBuf);
		// SMS Qeues For Sending And Receiving
		std::queue<sms_data_t> sendQueue;
		std::mutex m_sendQueueLock;

		std::queue<NetBuffer> receiveQueue;
		std::mutex m_rcvQueueLock;
		// Registring/Unregistering Functions For re-transmission of PDUs
		int RegisterPdu(NetBuffer &tmpNetBuf);
		int UnRegisterPdu(NetBuffer &tmpNetBuf);	
		// Will Send A Enquiry Link to Check SMPP Session Is Live
		int SendEnquireLink(void);
		////		class scope   /////
		// Sequence Number
		// Mutex Lock Required for Sequence Number Generation
		static Smpp::Uint32 m_esmePduSequenceNum;
		static pthread_mutex_t m_seqNumLock;
		// Register for Pdu So that Retransmissiion can happen
		// As std::map insert and erase are not thread safe 
		// We Required A Lock for Those Operation
		static std::map<uint32_t, NetBuffer> pduRegister;
		static std::mutex pduRegisterLock;
		// For vcMsgId to iSNo mappint
		static std::map<std::string, sms_data_t> m_vcMsgIdMap;
		static std::mutex m_vcMsgIdMapLock;
		// For vcMsgId to iStatus mapping, 
		//special case when we are processing delivery report 
		// before processing submit_sm_resp for same
		static std::map<std::string, uint8_t> m_vcMsgIdSyncMap;
#ifndef WITH_LIBEVENT
		// Receiving thread 
		// TODO: 
		// 1. One Thread For Reading For All Object
		// 2. Multi Read Concept have to Implement Here
		// 3. Assumption If Data Available Then As a SMPP Frame Minimum
		static pthread_t rcvThId;
		static thread_status_t rcvThStatus;
		static void *ThSmsReader(void *);
#endif

#ifndef WITH_LIBEVENT
		// Link Thread 
		// TODO: Decide About This Thread Very Less Job to Thread Resource
		// One Thread for Making Live All Socket
		// 1. Waisting Resource By Sending Enquire Link Periodically
		// 2. Think about Dynamic Connection on Demand
		static pthread_t linkThId;
		static thread_status_t linkThStatus;
		static void *LinkCheckThread(void *);
#else
		static void OnLinkCheckTimeOut(int fd, short event, void *arg);
#endif

#ifdef WITH_THREAD_POOL
		void OnReceivePdu(void);
		void SendOneSms(void);
#else
		// Sending Thread
		// One Thread For Sending is Enough
		// TODO: Think About Managing Congection Controll
		static void *ThSmsSender(void *);
		static pthread_t sendThId;
		static thread_status_t sendThStatus;

		// OnReceivePduThread -> Responsible for Processing SMS Packet 
		// TODO:
		// 1. This Thread Is Responsible For Processing One SMPP Frame At atime
		// 2. Design Each Frame Processing Such That should not block at any time
		// 3. Separate Database Interaction From Processing SMS PACKET
		static pthread_t pduProcessThId;
		static thread_status_t pduProcessThStatus;
		static void *ThOnReceivePdu(void *);
#endif	
		// One Map For Instance of This Class
		static std::map<std::string, Esme *> esmeInstanceMap;
		static uint32_t maxEsmeInstance;
		// MySql Update In Different Application

#ifdef _MESSAGE_QUEUE_UPDATE_
		SysMessageQueue m_updateQueryMsgQueue;
		std::string m_updateQueryMsgQueueName;
		uint32_t m_noOfMsgsInUpdateQueryMsgQueue;
		uint32_t m_sizePerMsgInUpdateQueryMsgQueue;
#endif
		~Esme(void);
#ifdef WITH_LIBEVENT
		Esme(std::string name, std::string cfgFile, struct event_base *base);
#endif
		Esme(std::string name, std::string cfgFile);
		Esme(const Esme&)=delete;
		Esme &operator=(const Esme&)=delete;
		// TODO: For Re-Bind Logic
		int Bind(void); 
		NetBuffer m_readNetBuffer;
	public:
		Smpp::Uint32 GetNewSequenceNumber(void);
		int OpenConnection(std::string host, uint32_t port);
		int OpenConnection(void);
		int CloseConnection(void);
		esme_state_t GetEsmeState(void);
		int Write(NetBuffer &); // Just Write all Bytes
		int Read(NetBuffer &); // Frammer will be implemented here 
		int Bind(Smpp::SystemId sysId, Smpp::Password pass, Smpp::SystemType sysType, Smpp::InterfaceVersion ifVer, Smpp::Ton ton, Smpp::Npi npi, Smpp::AddressRange range, bind_type_t bindType);
		int SendSubmitSm(uint32_t seqNo, const Smpp::Char *srcAddr, Smpp::Uint8 srcTon, Smpp::Uint8 srcNpi, const Smpp::Char *destAddr, Smpp::Uint8 destTon, Smpp::Uint8 destNpi, uint8_t type, const Smpp::Uint8 *sms, Smpp::Uint32 length);
		int UnBind(void);
// If libevent is There There is no need of Separate Reader Thread
#ifdef WITH_LIBEVENT
		static void EventCallBack(bufferevent*, short int, void*);
		static void ReadCallBack(bufferevent*, void*);
#else
		// Reader Having Class Scope
		static int StartReader(void);
		static int StopReader(void);
		static thread_status_t GetRcvThStatus(void);
#endif

#ifndef WITH_THREAD_POOL
		// One Sender Per Class Is Enough
		static int StartSender(void);
		static int StopSender(void);
		static thread_status_t GetSenderThStatus(void);
		// TODO: Decide One/Multiple Thread
		static int StartPduProcess(void);
		static int StopPduProcess(void);
		static thread_status_t GetPduProcessThStatus(void);
#endif
#ifndef WITH_LIBEVENT
// From Lib Event Once Timer will be Implemented 
// we can elemenate Linkcheck thread
// TODO: Introduce Libevent Timer For Linkcheck Thread
// new signature required start and stop linkcheck timer
// on linkcheck time out has to design
		// Can Be Managed In Single Thread
		static int StartLinkCheck(void);
		static int StopLinkCheck(void);
		static thread_status_t GetEnquireLinkThStatus(void);
#endif
		int Start(void);
		int Stop(void);
		std::string GetEsmeName(void);
#ifndef WITH_LIBEVENT
		int GetEsmeSocket(void);
#endif
		uint32_t GetEsmeTps(void);
		account_type_t GetEsmeAccountType(void);
		bool IsSendSms(void);
		uint32_t NoOfSmsInQueue(void);
		int SendSms(sms_data_t sms);
		bind_type_t GetEsmeType(void);
		// class Scope//
		//1. Controlled and Named Object
		static void RemoveEsmeInstance(std::string name);
		static uint32_t GetNumberOfEsmeInstance(bind_type_t type);
#ifdef WITH_LIBEVENT
		static Esme * GetEsmeInstance(std::string name, std::string cfgFile, struct event_base *base);
#else
		static Esme * GetEsmeInstance(std::string name, std::string cfgFile);
#endif
};
#endif

