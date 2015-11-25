#include "SmsDbInsertConfig.hpp"

#ifdef _MESSAGE_QUEUE_UPDATE_
SmsDbInsertConfig::~SmsDbInsertConfig(void){
}

SmsDbInsertConfig::SmsDbInsertConfig(const char *cfgFile){
	myconfig = appconfig_init();
}

int SmsDbInsertConfig::Open(const char *cfgFile){
	appconfig_open(myconfig, cfgFile);
}

int SmsDbInsertConfig::Close(void){
	appconfig_close(myconfig);
}

int SmsDbInsertConfig::Load(const char *cfgFile){
	char value[256] = {0x00};
	char tempBuf[256] = {0x00};
	if(cfgFile == NULL){
		return -1;
	}
	Open(cfgFile);
	// Logger
	memset(value, 0x00, sizeof(value));
	if(appconfig_getvalue(myconfig, "logger", "path", value)==0){
		path = value;
        }
	memset(value, 0x00, sizeof(value));
	if(appconfig_getvalue(myconfig, "logger", "name", value)==0){
		name = value;
        }
	memset(value, 0x00, sizeof(value));
        if(appconfig_getvalue(myconfig, "logger", "tag", value)==0){
                tag = value;
        }
	memset(value, 0x00, sizeof(value));
	if(appconfig_getvalue(myconfig, "logger", "level", value)==0){
		level = atoi(value);
        }
	memset(value, 0x00, sizeof(value));
	if(appconfig_getvalue(myconfig, "logger", "interval", value)==0){
		interval = atoi(value);
        }
	// MySql
	memset(value, 0x00, sizeof(value));
        if(appconfig_getvalue(myconfig, "mysql", "ip", value)==0){
		mysqlIp = value;
        }
	memset(value, 0x00, sizeof(value));
        if(appconfig_getvalue(myconfig, "mysql", "port", value)==0){
                mysqlPort = atoi(value);
        }
	memset(value, 0x00, sizeof(value));
        if(appconfig_getvalue(myconfig, "mysql", "user", value)==0){
		mysqlUser = value;
        }
	memset(value, 0x00, sizeof(value));
        if(appconfig_getvalue(myconfig, "mysql", "password", value)==0){
		mysqlPass = value;
        }
	memset(value, 0x00, sizeof(value));
        if(appconfig_getvalue(myconfig, "mysql", "name", value)==0){
		mysqlDbName = value;
        }
	// Message Queue Details
	memset(value, 0x00, sizeof(value));
        if(appconfig_getvalue(myconfig, "messagequeue", "updateqryque_name", value)==0){
                updateQueryQueueName = value;
        }
        memset(value, 0x00, sizeof(value));
        if(appconfig_getvalue(myconfig, "messagequeue", "updateqryque_msg_size", value)==0){
                updateQueryQueueMsgSize = atoi(value);
		if (updateQueryQueueMsgSize < 1){
			updateQueryQueueMsgSize = 256; // Some Default Value
		}
        }
        memset(value, 0x00, sizeof(value));
        if(appconfig_getvalue(myconfig, "messagequeue", "updateqryque_no_of_msg", value)==0){
                updateQueryQueueNoOfMsg = atoi(value);
		if(updateQueryQueueNoOfMsg < 1){
			updateQueryQueueNoOfMsg = 10; //some Default value
		}
        }
	Close();
}

std::string& SmsDbInsertConfig::GetLogPath(void){
	return path;
}

std::string& SmsDbInsertConfig::GetLogName(void){
	return name;
}

std::string& SmsDbInsertConfig::GetLogTag(void){
        return tag;
}

uint32_t SmsDbInsertConfig::GetLogLevel(void){
	return level;
}

uint32_t SmsDbInsertConfig::GetLogInterval(void){
	return interval;
}

std::string SmsDbInsertConfig::GetMysqlIp(void){
	return mysqlIp;
}

uint32_t SmsDbInsertConfig::GetMysqlPort(void){
	return mysqlPort;
}

std::string SmsDbInsertConfig::GetMysqlUser(void){
	return mysqlUser;
}

std::string SmsDbInsertConfig::GetMysqlPassword(void){
	return mysqlPass;
}

std::string SmsDbInsertConfig::GetMysqlDbName(void){
	return mysqlDbName;
}

// Message Queue
std::string SmsDbInsertConfig::GetUpdateQueryQueueName(void){
	return updateQueryQueueName;
}

uint32_t SmsDbInsertConfig::GetUpdateQueryQueueMsgSize(void){
	return updateQueryQueueMsgSize;
}

uint32_t SmsDbInsertConfig::GetUpdateQueryQueueNoOfMsg(void){
	return updateQueryQueueNoOfMsg;
}
#endif

