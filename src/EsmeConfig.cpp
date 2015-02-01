#include "EsmeConfig.hpp"

EsmeConfig::~EsmeConfig(void){
}

EsmeConfig::EsmeConfig(const char *cfgFile){
	myconfig = appconfig_init();
}

int EsmeConfig::Open(const char *cfgFile){
	appconfig_open(myconfig, cfgFile);
}

int EsmeConfig::Close(void){
	appconfig_close(myconfig);
}

int EsmeConfig::Load(const char *cfgFile){
	char value[256] = {0x00};
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
			smscType = 2;
		}else if(strcmp(value, "Rx") == 0){
                        smscType = 1;
                }else if(strcmp(value, "TRx") == 0){
                        smscType = 3;
                }else{
			smscType = 1; // Default is Receiving
		}
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
	
	Close();
}

std::string& EsmeConfig::GetLogPath(void){
	return path;
}

std::string& EsmeConfig::GetLogName(void){
	return name;
}

std::string& EsmeConfig::GetLogTag(void){
        return tag;
}

uint32_t EsmeConfig::GetLogLevel(void){
	return level;
}

uint32_t EsmeConfig::GetLogInterval(void){
	return interval;
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

std::string EsmeConfig::GetMysqlIp(void){
	return mysqlIp;
}

uint32_t EsmeConfig::GetMysqlPort(void){
	return mysqlPort;
}

std::string EsmeConfig::GetMysqlUser(void){
	return mysqlUser;
}

std::string EsmeConfig::GetMysqlPassword(void){
	return mysqlPass;
}

std::string EsmeConfig::GetMysqlDbName(void){
	return mysqlDbName;
}

unsigned int EsmeConfig::GetSmscType(void){
	return smscType;
}
