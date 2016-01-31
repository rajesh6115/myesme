#include <iostream>
#include <EsmeConfig.hpp>
#include <Esme.hpp>
#include <signal.h>
#ifdef WITH_THREAD_POOL
#include <ThreadPool.h>

progschj::ThreadPool g_threadPool;
#endif

int IS_PROCESS_RUN=0;

void int_signal(int num){
	IS_PROCESS_RUN = 0;
}

int main(int argc, char *argv[]){
	//std::cout << "Multi Esme Testing Instance" << std::endl;
	signal(SIGINT, int_signal);
	signal(SIGPIPE, SIG_IGN);
	pthread_t campaignThId;
	uint32_t campaignId=0;
	char configfile[256]={0x00};
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
	Esme::StartReader();
#ifndef WITH_THREAD_POOL
	Esme::StartPduProcess();
	Esme::StartSender();
#endif
	Esme::StartLinkCheck();
	Esme *tmpSmppConn = NULL;
	for(int i=0; i< CG_MyAppConfig.GetMaxinumSmppConnection(); i++){
		APP_LOGGER(CG_MyAppLogger, LOG_DEBUG, "Connection NAME: %s : Config File %s", CG_MyAppConfig.GetSmppConnectionName(i).c_str(), CG_MyAppConfig.GetSmppConnectionConfigFile(i).c_str() ); 
		tmpSmppConn = Esme::GetEsmeInstance(CG_MyAppConfig.GetSmppConnectionName(i), CG_MyAppConfig.GetSmppConnectionConfigFile(i));
	}
	sleep(1);
	IS_PROCESS_RUN = 1;
	sleep(1);
	std::map<uint32_t, campaign_info_t>::iterator l_campaignItr;
	while(IS_PROCESS_RUN){
		if(campaignId = IsActiveCampaign()){
			APP_LOGGER(CG_MyAppLogger,LOG_DEBUG, "There is a Active Campaign with Campaign Id %u",campaignId);
			campaign_info_t tempCampaignInfo;
			tempCampaignInfo.id = campaignId;
			tempCampaignInfo.thStatus = TH_ST_REQ_RUN;
			tempCampaignInfo.status = CAMPAIGN_ST_SCHEDULED;
			// Insert To Campaign Map
			G_smsCampaignMap.insert(std::pair<uint32_t, campaign_info_t>(campaignId, tempCampaignInfo));
			if(pthread_create(&tempCampaignInfo.thId, NULL, CampaignThread, &campaignId)==0){
				APP_LOGGER(CG_MyAppLogger, LOG_DEBUG, "Starting Campaign with Campaign ID %u ", campaignId);
				l_campaignItr = G_smsCampaignMap.find(campaignId);
				if(l_campaignItr != G_smsCampaignMap.end()){
					l_campaignItr->second.thId = tempCampaignInfo.thId;
					while( l_campaignItr->second.thStatus == TH_ST_REQ_RUN){
						sleep(1);
						APP_LOGGER(CG_MyAppLogger, LOG_DEBUG, "Waiting For Campaign with Campaign ID %u  to Start...", campaignId);
					}
				}else{
					APP_LOGGER(CG_MyAppLogger, LOG_WARNING, "No Entries in Campaign Map For Campaign %u", campaignId);
				}
			}else{
				APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "pthread_create Failed For Campaign %u", campaignId);
                                l_campaignItr = G_smsCampaignMap.find(campaignId);
				if(l_campaignItr != G_smsCampaignMap.end()){
					G_smsCampaignMap.erase(l_campaignItr);
				}
			}
		}else{
			for(l_campaignItr = G_smsCampaignMap.begin(); l_campaignItr != G_smsCampaignMap.end(); ){
				if(l_campaignItr->second.thStatus == TH_ST_STOP){
					pthread_join(l_campaignItr->second.thId, NULL);
					APP_LOGGER(CG_MyAppLogger, LOG_INFO, "Removing Campaign %u From Campaign Map", l_campaignItr->second.id);
					G_smsCampaignMap.erase(l_campaignItr++);
					//break; // bread for loop for erasing one at atime
				}else{
					++l_campaignItr;
				}
			}
		}
		sleep(30);
	}
#ifndef WITH_THREAD_POOL
	Esme::StopSender();
#endif
	for(int i=0; i< CG_MyAppConfig.GetMaxinumSmppConnection(); i++){
		tmpSmppConn = Esme::GetEsmeInstance(CG_MyAppConfig.GetSmppConnectionName(i), CG_MyAppConfig.GetSmppConnectionConfigFile(i));
		tmpSmppConn->Stop();
		Esme::RemoveEsmeInstance(CG_MyAppConfig.GetSmppConnectionName(i));
	}
	Esme::StopLinkCheck();
	Esme::StopReader();
#ifndef WITH_THREAD_POOL
	Esme::StopPduProcess();
#endif
	APP_LOGGER(CG_MyAppLogger, LOG_DEFAULT, "SMS Application Exiting...");
	logger_cleanup(CG_MyAppLogger);
	return 0;
}
