#include "SmsDbInsertConfig.hpp"

#ifdef _MESSAGE_QUEUE_UPDATE_
SmsDbInsertConfig::~SmsDbInsertConfig(void){
	if(myconfig){
		Close();
	}
}

SmsDbInsertConfig::SmsDbInsertConfig(const char *cfgFile){
	myconfig = appconfig_init();
	level = 8; //LOG_DEFAULT;
	interval = 3600;
	mysqlPort = 3306 ;
	updateQueryQueueMsgSize = 256;
	updateQueryQueueNoOfMsg = 10;
}

int SmsDbInsertConfig::Open(const char *cfgFile){
	return appconfig_open(myconfig, cfgFile);
}

int SmsDbInsertConfig::Close(void){
	appconfig_close(myconfig);
	myconfig = NULL;
	return 0;
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
        }else{
                return -1;
        }
	memset(value, 0x00, sizeof(value));
	if(appconfig_getvalue(myconfig, "logger", "name", value)==0){
		name = value;
        }else{
                return -1;
        }
	memset(value, 0x00, sizeof(value));
        if(appconfig_getvalue(myconfig, "logger", "tag", value)==0){
                tag = value;
        }else{
                return -1;
        }
	memset(value, 0x00, sizeof(value));
	if(appconfig_getvalue(myconfig, "logger", "level", value)==0){
		level = atoi(value);
        }else{
                return -1;
        }
	memset(value, 0x00, sizeof(value));
	if(appconfig_getvalue(myconfig, "logger", "interval", value)==0){
		interval = atoi(value);
        }else{
                return -1;
        }
	// MySql
	memset(value, 0x00, sizeof(value));
        if(appconfig_getvalue(myconfig, "mysql", "ip", value)==0){
		mysqlIp = value;
        }else{
                return -1;
        }
	memset(value, 0x00, sizeof(value));
        if(appconfig_getvalue(myconfig, "mysql", "port", value)==0){
                mysqlPort = atoi(value);
        }else{
                return -1;
        }
	memset(value, 0x00, sizeof(value));
        if(appconfig_getvalue(myconfig, "mysql", "user", value)==0){
		mysqlUser = value;
        }else{
                return -1;
        }
	memset(value, 0x00, sizeof(value));
        if(appconfig_getvalue(myconfig, "mysql", "password", value)==0){
		mysqlPass = value;
        }else{
                return -1;
        }
	memset(value, 0x00, sizeof(value));
        if(appconfig_getvalue(myconfig, "mysql", "name", value)==0){
		mysqlDbName = value;
        }else{
                return -1;
        }
	// Message Queue Details
	memset(value, 0x00, sizeof(value));
        if(appconfig_getvalue(myconfig, "messagequeue", "updateqryque_name", value)==0){
                updateQueryQueueName = value;
        }else{
                return -1;
        }
        memset(value, 0x00, sizeof(value));
        if(appconfig_getvalue(myconfig, "messagequeue", "updateqryque_msg_size", value)==0){
                updateQueryQueueMsgSize = atoi(value);
		if (updateQueryQueueMsgSize < 1){
			updateQueryQueueMsgSize = 256; // Some Default Value
		}
        }else{
                return -1;
        }
        memset(value, 0x00, sizeof(value));
        if(appconfig_getvalue(myconfig, "messagequeue", "updateqryque_no_of_msg", value)==0){
                updateQueryQueueNoOfMsg = atoi(value);
		if(updateQueryQueueNoOfMsg < 1){
			updateQueryQueueNoOfMsg = 10; //some Default value
		}
        }else{
                return -1;
        }
	return 0;
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

