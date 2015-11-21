#include <iostream>
#include "Esme.hpp"
#include "Externs.hpp"
#include "MySqlWrapper.hpp"
#include "pthread.h"
#include "Defines.hpp"
#include <signal.h>
extern thread_status_t G_CampaignThStatus;
Esme myEsme;
int IS_PROCESS_RUN=0;

void int_signal(int num){
	IS_PROCESS_RUN = 0;
}

int main(int argc, char *argv[]){
	char configfile[256]={0x00};
	pthread_t campaignThId;
	uint32_t campaignId=0;
	signal(SIGINT, int_signal);
	if(argc != 2){
		fprintf(stderr, "USAGE: %s <configfile>\n", argv[0]);
		return 1;
	}else{
		sprintf(configfile, "%s", argv[1]);
	}
	// Open and Read Cofigurations
	CG_MyAppConfig.Load(configfile);
	// Open Logger
	CG_MyAppLogger = logger_init();
	logger_filepath(CG_MyAppLogger, CG_MyAppConfig.GetLogPath().c_str());
	logger_filename(CG_MyAppLogger, CG_MyAppConfig.GetLogName().c_str());
	logger_apptag(CG_MyAppLogger, CG_MyAppConfig.GetLogTag().c_str());
	logger_level(CG_MyAppLogger, CG_MyAppConfig.GetLogLevel());
	logger_interval(CG_MyAppLogger, CG_MyAppConfig.GetLogInterval());

	APP_LOGGER(CG_MyAppLogger, LOG_DEFAULT, "Compiled On %s %s", __DATE__, __TIME__);
	APP_LOGGER(CG_MyAppLogger, LOG_DEFAULT, "Report Bugs to %s",PACKAGE_BUGREPORT);
	// Open Esme Connection
	std::cout << CG_MyAppConfig.GetSmscIp() << ":" << CG_MyAppConfig.GetSmscPort() << std::endl;
	myEsme.Start();
	// Start Main Loop Of Application
	IS_PROCESS_RUN = 1;
	while(IS_PROCESS_RUN){
		// Check Type Of Connection 
		// If Tx or RDWR Then Only Start Campaign Other Wise Just Wait For Other Thread Finish There Job
		APP_LOGGER(CG_MyAppLogger, LOG_DEBUG, "SMSC TYPE= %u ", CG_MyAppConfig.GetSmscType());
		if((CG_MyAppConfig.GetSmscType() == Esme::BIND_WRONLY) || (CG_MyAppConfig.GetSmscType()==Esme::BIND_RDWR)){
			if(campaignId = IsActiveCampaign()){
				APP_LOGGER(CG_MyAppLogger,LOG_DEBUG, "There is a Active Campaign with Campaign Id %u",campaignId);
				// TODO : Multiple Campaign At a time
				// If One Campaign is Running We Should Not Start Another One
				// Make thread pool for campaigns. So that There will be limited threads 
				// to schedule
				// 1. Quick fix done using thread status for campaign thread in IsActiveCampaign
				if(pthread_create(&campaignThId, NULL, CampaignThread, &campaignId)==0){
					APP_LOGGER(CG_MyAppLogger, LOG_DEBUG, "Starting Campaign with Campaign ID %u ", campaignId);
					sleep(5); // Give time To Start later change with thread status logic
				}
			}
		}
		if(myEsme.GetEsmeState() == Esme::ST_ERROR){
			APP_LOGGER(CG_MyAppLogger, LOG_DEBUG, "SMSC ERROR OCCURED ");
			myEsme.Stop();
		}
		if(myEsme.GetEsmeState() == Esme::ST_IDLE){
			APP_LOGGER(CG_MyAppLogger, LOG_WARNING, "RE-CONNECTING TO SMSC");
			if(myEsme.Start() == -1){
				APP_LOGGER(CG_MyAppLogger, LOG_WARNING, "FAIL TO RE-CONNECTING TO SMSC. TRY TO CONNECT AFTER 60 SEC");
				sleep(60);
			}
		}
		sleep(DFL_SLEEP_VALUE);
	}

	myEsme.Stop();
	// Close Logger
	logger_cleanup(CG_MyAppLogger);
	return 0;
}
