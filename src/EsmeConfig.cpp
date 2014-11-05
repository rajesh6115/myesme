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
	memset(value, 0x00, sizeof(value));
	if(appconfig_getvalue(myconfig, "bind", "system_id", value)==0){
                printf("value for %s of module %s is %s\n", "element1", "module1", value);
        }
	Close();
}

Smpp::SystemId EsmeConfig::GetSystemId(void){

}

Smpp::Password EsmeConfig::GetPassword(void){

}

Smpp::SystemType EsmeConfig::GetSystemType(void){

}

Smpp::InterfaceVersion EsmeConfig::GetInterfaceVersion(void){

}

Smpp::Ton EsmeConfig::GetTon(void){

}

Smpp::Npi EsmeConfig::GetNpi(void){

}
