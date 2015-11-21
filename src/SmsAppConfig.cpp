#include "SmsAppConfig.hpp"

SmsAppConfig::~SmsAppConfig(void){
}

SmsAppConfig::SmsAppConfig(const char *cfgFile){
	myconfig = appconfig_init();
}

int SmsAppConfig::Open(const char *cfgFile){
	appconfig_open(myconfig, cfgFile);
}

int SmsAppConfig::Close(void){
	appconfig_close(myconfig);
}

int SmsAppConfig::Load(const char *cfgFile){
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
	// SMS Campaign Details
	memset(value, 0x00, sizeof(value));
        if(appconfig_getvalue(myconfig, "smscampaign", "maxcampaign", value)==0){
                noOfCampaign = atoi(value);
        }
	memset(value, 0x00, sizeof(value));
	if(appconfig_getvalue(myconfig, "smscampaign", "campaigntype", value)==0){
		campaignType = value;
	}
	
	// Smpp Connections 
	memset(value, 0x00, sizeof(value));
	if(appconfig_getvalue(myconfig, "smppconnection", "noofconnection", value)==0){
                noOfSmppConnection = atoi(value);
        }

	for(int i=0; i< noOfSmppConnection; i++){
		memset(tempBuf, 0x00, sizeof(tempBuf));
		sprintf(tempBuf, "smppcon%d", i+1);
		memset(value, 0x00, sizeof(value));
		if(appconfig_getvalue(myconfig, tempBuf, "name", value)==0){
                	m_emseNames[i] = value; 
        	}
		memset(value, 0x00, sizeof(value));
                if(appconfig_getvalue(myconfig, tempBuf, "cfgfile", value)==0){
                        m_emseConfigFiles[i] = value;
                }
	}

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

std::string& SmsAppConfig::GetLogPath(void){
	return path;
}

std::string& SmsAppConfig::GetLogName(void){
	return name;
}

std::string& SmsAppConfig::GetLogTag(void){
        return tag;
}

uint32_t SmsAppConfig::GetLogLevel(void){
	return level;
}

uint32_t SmsAppConfig::GetLogInterval(void){
	return interval;
}

std::string SmsAppConfig::GetMysqlIp(void){
	return mysqlIp;
}

uint32_t SmsAppConfig::GetMysqlPort(void){
	return mysqlPort;
}

std::string SmsAppConfig::GetMysqlUser(void){
	return mysqlUser;
}

std::string SmsAppConfig::GetMysqlPassword(void){
	return mysqlPass;
}

std::string SmsAppConfig::GetMysqlDbName(void){
	return mysqlDbName;
}

std::string SmsAppConfig::GetCampaignType(void){
	return campaignType;
}

// SMPP Connections
uint32_t SmsAppConfig::GetMaxinumSmppConnection(void){
	return noOfSmppConnection;
}

std::string SmsAppConfig::GetSmppConnectionName(uint32_t index){
	return m_emseNames[index];
}

std::string SmsAppConfig::GetSmppConnectionConfigFile(uint32_t index){
	return m_emseConfigFiles[index];
}

#ifdef _MESSAGE_QUEUE_UPDATE_
// Message Queue
std::string SmsAppConfig::GetUpdateQueryQueueName(void){
	return updateQueryQueueName;
}

uint32_t SmsAppConfig::GetUpdateQueryQueueMsgSize(void){
	return updateQueryQueueMsgSize;
}

uint32_t SmsAppConfig::GetUpdateQueryQueueNoOfMsg(void){
	return updateQueryQueueNoOfMsg;
}
#endif

