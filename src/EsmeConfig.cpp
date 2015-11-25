#include "EsmeConfig.hpp"

EsmeConfig::~EsmeConfig(void){
}

EsmeConfig::EsmeConfig(const char *cfgFile){
	myconfig = appconfig_init();
}

int EsmeConfig::Open(const char *cfgFile){
	if(myconfig == NULL){
		myconfig = appconfig_init();
	}
	appconfig_open(myconfig, cfgFile);
}

int EsmeConfig::Close(void){
	appconfig_close(myconfig);
	myconfig = NULL;
}

int EsmeConfig::Load(const char *cfgFile){
	char value[256] = {0x00};
	if(cfgFile == NULL){
		return -1;
	}
	Open(cfgFile);
	// SMSC 
	memset(value, 0x00, sizeof(value));
        if(appconfig_getvalue(myconfig, "smsc", "ip", value)==0){
                ip = value;
        }
	memset(value, 0x00, sizeof(value));
        if(appconfig_getvalue(myconfig, "smsc", "port", value)==0){
                port = atoi(value);
        }
	memset(value, 0x00, sizeof(value));
	if(appconfig_getvalue(myconfig, "smsc", "tps", value)==0){
		smsctps=0;
                smsctps = atoi(value);
		if(smsctps == 0){
			smsctps = 100; // Some Default Value
		}	
        }
	memset(value, 0x00, sizeof(value));
	if(appconfig_getvalue(myconfig, "smsc", "type", value)==0){
		if(strcmp(value, "Tx") == 0){
			smscType = BIND_WRONLY;
		}else if(strcmp(value, "Rx") == 0){
                        smscType = BIND_RDONLY;
                }else if(strcmp(value, "TRx") == 0){
                        smscType = BIND_RDWR;
                }else{
			smscType = BIND_RDONLY; // Default is Receiving
		}
	}
	memset(value, 0x00, sizeof(value));
	if(appconfig_getvalue(myconfig, "smsc", "campaigntype", value)==0){
		campaignType = value;
	}
	// BIND
	memset(value, 0x00, sizeof(value));
	if(appconfig_getvalue(myconfig, "bind", "system_id", value)==0){
		bindSysId = value;
        }
	memset(value, 0x00, sizeof(value));
	if(appconfig_getvalue(myconfig, "bind", "password", value)==0){
		bindPass = value;
        }
	memset(value, 0x00, sizeof(value));
	if(appconfig_getvalue(myconfig, "bind", "system_type", value)==0){
		bindSysType = value;
        }
	memset(value, 0x00, sizeof(value));
	if(appconfig_getvalue(myconfig, "bind", "interface_version", value)==0){
		if(strcmp("5.0", value) == 0)
			esmeSmppVersion = Smpp::InterfaceVersion::V50;
		else 
			esmeSmppVersion = Smpp::InterfaceVersion::V34; // Default Version for all other
        }
	memset(value, 0x00, sizeof(value));
	if(appconfig_getvalue(myconfig, "bind", "ton", value)==0){
		bindTon = atoi(value);
        }
	memset(value, 0x00, sizeof(value));
	if(appconfig_getvalue(myconfig, "bind", "npi", value)==0){
		bindNpi = atoi(value);
        }
	memset(value, 0x00, sizeof(value));
	if(appconfig_getvalue(myconfig, "bind", "address_range", value)==0){
		bindAddrRange = value;
        }
	memset(value, 0x00, sizeof(value));
	if(appconfig_getvalue(myconfig, "messagequeue", "vcmsgidque_name", value)==0){
		vcMsgIdQueueName = value;
        }
	memset(value, 0x00, sizeof(value));
	if(appconfig_getvalue(myconfig, "messagequeue", "vcmsgidque_msg_size", value)==0){
		vcMsgIdQueueMsgSize = 256;
                vcMsgIdQueueMsgSize = atoi(value);
        }
	memset(value, 0x00, sizeof(value));
	if(appconfig_getvalue(myconfig, "messagequeue", "vcmsgidque_no_of_msg", value)==0){
		vcMsgIdQueueNoOfMsg = 10;
		vcMsgIdQueueNoOfMsg = atoi(value);
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


std::string& EsmeConfig::GetSmscIp(void){
	return ip;
}

uint32_t EsmeConfig::GetSmscPort(void){
	return port;
}

Smpp::SystemId EsmeConfig::GetSystemId(void){
	return bindSysId;
}

Smpp::Password EsmeConfig::GetPassword(void){
	return bindPass;
}

Smpp::SystemType EsmeConfig::GetSystemType(void){
	return bindSysType;
}

Smpp::InterfaceVersion EsmeConfig::GetInterfaceVersion(void){
	return esmeSmppVersion;
}

Smpp::Ton EsmeConfig::GetTon(void){
	return bindTon;
}

Smpp::Npi EsmeConfig::GetNpi(void){
	return bindNpi;
}

Smpp::AddressRange EsmeConfig::GetAddressRange(void){
	return bindAddrRange;
}

bind_type_t EsmeConfig::GetSmscType(void){
	return smscType;
}

uint32_t EsmeConfig::GetSmscTps(void){
	return smsctps;
}

#ifdef _MESSAGE_QUEUE_UPDATE_
// Message Queue
std::string EsmeConfig::GetUpdateQueryQueueName(void){
        return updateQueryQueueName;
}

uint32_t EsmeConfig::GetUpdateQueryQueueMsgSize(void){
        return updateQueryQueueMsgSize;
}

uint32_t EsmeConfig::GetUpdateQueryQueueNoOfMsg(void){
        return updateQueryQueueNoOfMsg;
}
#endif

