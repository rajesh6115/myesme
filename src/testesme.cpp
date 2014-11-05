#include <iostream>
#include "Esme.hpp"
#include "Externs.hpp"
int main(int argc, char *argv[]){
	Esme myEsme;
	std::cout << "hello Esms" << std::endl;
	// Open and Read Cofigurations
	CG_MyAppConfig.Load("/usr/local/etc/MyEsme.xml");
	// Open Logger
	CG_MyAppLogger = logger_init();
	logger_filepath(CG_MyAppLogger, "/var/log/myesme");
	logger_filename(CG_MyAppLogger, "myesme");
	logger_apptag(CG_MyAppLogger, "ESME");
	logger_open(CG_MyAppLogger);
	APP_LOGGER(CG_MyAppLogger, LOG_DEFAULT, "Compiled On %s %s", __DATE__, __TIME__);
	APP_LOGGER(CG_MyAppLogger, LOG_DEFAULT, "Report Bugs to %s",PACKAGE_BUGREPORT);
	// Open Data bases

	// Open Esme Connection
	if(myEsme.OpenConnection("127.0.0.1",2775) == 0){
		std::cout << "socket connected" << std::endl;
	}else{
		std::cerr << "socket Connection fail" << std::endl;
		return -1;
	}
	getchar();
	// Start Reading Thread
	myEsme.StartReader();
	while(myEsme.GetRcvThStatus() != TH_ST_RUNNING){
		sleep(1);
	}
	
	// Start Pdu Processing Thread
	myEsme.StartPduProcess();
	while(myEsme.GetPduProcessThStatus() != TH_ST_RUNNING){
		sleep(1);
	}
	// Start Keep Alive Thread


	// Start pushing Pdus
	Smpp::SystemId sysId("smppclient1");
	Smpp::Password pass("password");
	myEsme.Bind( sysId, pass, BIND_RDWR);
	sleep(10);
	myEsme.SendSubmitSm("6969", "9853493896", 1, (Smpp::Uint8 *)"Wel Come to SMS Test", 20);
	sleep(10);
	myEsme.UnBind();
	myEsme.StopReader();
	myEsme.StopPduProcess();
	myEsme.CloseConnection();
	// Close Logger
	logger_cleanup(CG_MyAppLogger);
	return 0;
}
