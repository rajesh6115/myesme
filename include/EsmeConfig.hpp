#ifndef _ESME_CONFIG_H_
#define _ESME_CONFIG_H_
#include <iostream>
#include <cstring>
#include <cstdint>
#include "appconfig.hpp"
#include "smpp.hpp"
#include "Externs.hpp"

class EsmeConfig{
	private:
	appconfig_p myconfig;
	// Connect
	std::string ip;
	uint32_t port;
        // BIND 
        Smpp::SystemId bindSysId;
        Smpp::Password bindPass;
        Smpp::SystemType bindSysType;
        Smpp::InterfaceVersion esmeSmppVersion;
        Smpp::Ton bindTon;
        Smpp::Npi bindNpi;
        Smpp::AddressRange bindAddrRange;
	uint32_t smsctps;
	bind_type_t smscType;
        int Open(const char *cfgFile );
        int Close(void);
#ifdef _MESSAGE_QUEUE_UPDATE_	
	// Message Queue
        std::string updateQueryQueueName;
        uint32_t updateQueryQueueMsgSize;
        uint32_t updateQueryQueueNoOfMsg;
#endif

        public:
        EsmeConfig(const char *cfgFile=NULL);
        ~EsmeConfig(void);
        int Load(const char *cfgFile );
	// SMSC
	std::string& GetSmscIp(void);
	uint32_t GetSmscPort(void);
	bind_type_t GetSmscType(void);
        Smpp::SystemId GetSystemId(void);
        Smpp::Password GetPassword(void);
        Smpp::SystemType GetSystemType(void);
        Smpp::InterfaceVersion GetInterfaceVersion(void);
        Smpp::Ton GetTon(void);
        Smpp::Npi GetNpi(void);
        Smpp::AddressRange GetAddressRange(void);
	uint32_t GetSmscTps(void);
#ifdef _MESSAGE_QUEUE_UPDATE_
	// Message Queue
        std::string GetUpdateQueryQueueName(void);
        uint32_t GetUpdateQueryQueueMsgSize(void);
        uint32_t GetUpdateQueryQueueNoOfMsg(void);
#endif
};
#endif

