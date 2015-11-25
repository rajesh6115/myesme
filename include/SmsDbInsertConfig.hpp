#ifndef _SMS_DBINSERT_CONFIG_H_
#define _SMS_DBINSERT_CONFIG_H_

#ifdef _MESSAGE_QUEUE_UPDATE_	
#include <iostream>
#include <cstring>
#include <cstdint>
#include "appconfig.hpp"
#include "smpp.hpp"
#include "Defines.hpp"

class SmsDbInsertConfig{
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
	// Message Queue
        std::string updateQueryQueueName;
        uint32_t updateQueryQueueMsgSize;
        uint32_t updateQueryQueueNoOfMsg;
        int Open(const char *cfgFile );
        int Close(void);

        public:
        SmsDbInsertConfig(const char *cfgFile=NULL);
        ~SmsDbInsertConfig(void);
        int Load(const char *cfgFile );
	std::string& GetLogPath(void);
	std::string& GetLogName(void);
	std::string& GetLogTag(void);
	uint32_t GetLogLevel(void);
	uint32_t GetLogInterval(void);
	// Mysql
	std::string GetMysqlIp(void);
	uint32_t GetMysqlPort(void);
	std::string GetMysqlUser(void);
	std::string GetMysqlPassword(void);
	std::string GetMysqlDbName(void);
	// Message Queue
        std::string GetUpdateQueryQueueName(void);
        uint32_t GetUpdateQueryQueueMsgSize(void);
        uint32_t GetUpdateQueryQueueNoOfMsg(void);
};

#endif
#endif

