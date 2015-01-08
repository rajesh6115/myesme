#ifndef _ESME_CONFIG_H_
#define _ESME_CONFIG_H_
#include <iostream>
#include <cstring>
#include <cstdint>
#include "appconfig.hpp"
#include "smpp.hpp"

class EsmeConfig{
	private:
	appconfig_p myconfig;
	// Logger
	std::string path;
	std::string name;
	std::string tag;
	uint32_t level;
	uint32_t interval;
	//Mysql
	std::string mysqlIp;
	uint32_t mysqlPort;
	int smsctps;
	std::string mysqlUser;
	std::string mysqlPass;
	std::string mysqlDbName;
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
        int Open(const char *cfgFile );
        int Close(void);

        public:
        EsmeConfig(const char *cfgFile=NULL);
        ~EsmeConfig(void);
        int Load(const char *cfgFile );
	std::string& GetLogPath(void);
	std::string& GetLogName(void);
	std::string& GetLogTag(void);
	uint32_t GetLogLevel(void);
	uint32_t GetLogInterval(void);
	// SMSC
	std::string& GetSmscIp(void);
	uint32_t GetSmscPort(void);

        Smpp::SystemId GetSystemId(void);
        Smpp::Password GetPassword(void);
        Smpp::SystemType GetSystemType(void);
        Smpp::InterfaceVersion GetInterfaceVersion(void);
        Smpp::Ton GetTon(void);
        Smpp::Npi GetNpi(void);
        Smpp::AddressRange GetAddressRange(void);
	// Mysql
	std::string GetMysqlIp(void);
	uint32_t GetMysqlPort(void);
	std::string GetMysqlUser(void);
	std::string GetMysqlPassword(void);
	std::string GetMysqlDbName(void);
};
#endif

