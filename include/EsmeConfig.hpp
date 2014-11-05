#ifndef _ESME_CONFIG_H_
#define _ESME_CONFIG_H_
#include <iostream>
#include <cstring>
#include "appconfig.hpp"
#include "smpp.hpp"

class EsmeConfig{
	private:
	appconfig_p myconfig;
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
        Smpp::SystemId GetSystemId(void);
        Smpp::Password GetPassword(void);
        Smpp::SystemType GetSystemType(void);
        Smpp::InterfaceVersion GetInterfaceVersion(void);
        Smpp::Ton GetTon(void);
        Smpp::Npi GetNpi(void);
        Smpp::AddressRange GetAddressRange(void);
};
#endif

