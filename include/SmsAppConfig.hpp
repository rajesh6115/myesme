#ifndef _SMS_APP_CONFIG_H_
#define _SMS_APP_CONFIG_H_
#include <iostream>
#include <cstring>
#include <cstdint>
#include "appconfig.hpp"
#include "smpp.hpp"
#include "Defines.hpp"

class SmsAppConfig{
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
	std::string campaignType;
	std::string mysqlUser;
	std::string mysqlPass;
	std::string mysqlDbName;
	// SMS Campaign Data
	uint32_t noOfCampaign;
	
	// Esme Connection Details
	uint32_t noOfSmppConnection;
	std::string m_emseConfigFiles[ESME_MAX_NO_OF_INSTANCE];
	std::string m_emseNames[ESME_MAX_NO_OF_INSTANCE];
#ifdef _MESSAGE_QUEUE_UPDATE_	
	// Message Queue
        std::string updateQueryQueueName;
        uint32_t updateQueryQueueMsgSize;
        uint32_t updateQueryQueueNoOfMsg;
#endif
        int Open(const char *cfgFile );
        int Close(void);

        public:
        SmsAppConfig(const char *cfgFile=NULL);
        ~SmsAppConfig(void);
        int Load(const char *cfgFile );
	std::string& GetLogPath(void);
	std::string& GetLogName(void);
	std::string& GetLogTag(void);
	uint32_t GetLogLevel(void);
	uint32_t GetLogInterval(void);
	// For SMS Campaigns
	std::string GetCampaignType(void);
	// Mysql
	std::string GetMysqlIp(void);
	uint32_t GetMysqlPort(void);
	std::string GetMysqlUser(void);
	std::string GetMysqlPassword(void);
	std::string GetMysqlDbName(void);
	// SMS Campaigns
	
	// SMPP Connections
	uint32_t GetMaxinumSmppConnection(void);
	std::string GetSmppConnectionName(uint32_t index);
	std::string GetSmppConnectionConfigFile(uint32_t index);
#ifdef _MESSAGE_QUEUE_UPDATE_
	// Message Queue
        std::string GetUpdateQueryQueueName(void);
        uint32_t GetUpdateQueryQueueMsgSize(void);
        uint32_t GetUpdateQueryQueueNoOfMsg(void);
#endif
};
#endif

