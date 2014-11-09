#include <iostream>
#include "Esme.hpp"
#include "Externs.hpp"
#include "MySqlWrapper.hpp"

uint32_t IsActiveCampaign(){
	CMySQL sqlobj;
	uint32_t campaignId;
	int errNo;
	char errMsg[256]={0x00};
	if(sqlobj.mcfn_Open("127.0.0.1", "test", "root", "padmapur") ){
		std::cout << "Connection Estalishe to Database" << std::endl;
		
	}else{
		// Log Error
		return 0;
	}
	std::string activeCampaignQuery = "SELECT iSNo FROM CampaignMaster WHERE iActive=1 and dtStartDatetime < NOW() LIMIT 1";
	if(sqlobj.mcfn_GetResultSet(activeCampaignQuery.c_str(), errNo, errMsg)){
		std::cout << "Query Executed" << std::endl;
	}else{
		std::cout << "Problem in Excuting Query" << activeCampaignQuery << std::endl;
		return 0;
	}
	MYSQL_ROW tempRow;
	tempRow = mysql_fetch_row(sqlobj.m_pRecordsetPtr);
	std::cout << "result="<< tempRow[0] << std::endl;
	campaignId = atoi(tempRow[0]);
	mysql_free_result(sqlobj.m_pRecordsetPtr);
	return campaignId;
}

int main(int argc, char *argv[]){
	Esme myEsme;
	uint32_t campaignId=0;
	std::cout << "hello Esms" << std::endl;
	// Open and Read Cofigurations
	CG_MyAppConfig.Load("/usr/local/etc/MyEsme.xml");
	// Open Logger
	CG_MyAppLogger = logger_init();
	logger_filepath(CG_MyAppLogger, CG_MyAppConfig.GetLogPath().c_str());
	logger_filename(CG_MyAppLogger, CG_MyAppConfig.GetLogName().c_str());
	logger_apptag(CG_MyAppLogger, CG_MyAppConfig.GetLogTag().c_str());
	logger_level(CG_MyAppLogger, CG_MyAppConfig.GetLogLevel());
	logger_interval(CG_MyAppLogger, CG_MyAppConfig.GetLogInterval());

	APP_LOGGER(CG_MyAppLogger, LOG_DEFAULT, "Compiled On %s %s", __DATE__, __TIME__);
	APP_LOGGER(CG_MyAppLogger, LOG_DEFAULT, "Report Bugs to %s",PACKAGE_BUGREPORT);
	// Open Data bases
	if(campaignId = IsActiveCampaign()){
		std::cout << "There is a Active Campaign with Campaign Id " << campaignId << std::endl;
		// Start Campaign Thread
	}
	// Open Esme Connection
	std::cout << CG_MyAppConfig.GetSmscIp() << ":" << CG_MyAppConfig.GetSmscPort() << std::endl;
	if(myEsme.OpenConnection(CG_MyAppConfig.GetSmscIp(), CG_MyAppConfig.GetSmscPort()) == 0){
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
	

	// Start pushing Pdus
	Smpp::SystemId sysId=CG_MyAppConfig.GetSystemId();
	Smpp::Password pass=CG_MyAppConfig.GetPassword();
	myEsme.Bind( sysId, pass, BIND_RDWR);
	//myEsme.Bind( sysId, pass, BIND_WRONLY);
	while(myEsme.GetEsmeState() != Esme::ST_BIND){
		sleep(1);
	}
	// Start Keep Alive Thread
	myEsme.StartLinkCheck();
	while(myEsme.GetEnquireLinkThStatus() != TH_ST_RUNNING){
		sleep(1);
	}
	myEsme.SendSubmitSm("080008", "8884882772", 1, (Smpp::Uint8 *)"Wel Come to SMS Test", 20);
	sleep(10);
	myEsme.StopLinkCheck();
	myEsme.UnBind();
	while(myEsme.GetEsmeState() != Esme::ST_UNBIND){
		sleep(1);
	}
	myEsme.StopReader();
	myEsme.StopPduProcess();
	myEsme.CloseConnection();
	// Close Logger
	logger_cleanup(CG_MyAppLogger);
	return 0;
}
