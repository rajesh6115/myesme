#include <iostream>
#include <SmsDbInsertConfig.hpp>
#include <MessageQueue.hpp>
#include <MySqlWrapper.hpp>
#include <clogger.hpp>
#include <signal.h>
#include <unistd.h>

#define DFL_SLEEP 1
#define MYSQL_RECON_SLEEP 2
#define MAX_MSG_IN_QUEUE 1000

SmsDbInsertConfig CG_MyAppConfig;
logger_p CG_MyAppLogger;
SysMessageQueue updateQueryQueue;

int IS_PROCESS_RUN=0;

void int_signal(int num){
        IS_PROCESS_RUN = 0;
}

int main(int argc, char * argv[]){
	char configfile[256]={0x00};
	char msg[512]={0x00};
	int errNo;
	char errMsg[256]={0x00};
	uint32_t no_of_msg = 0;
	signal(SIGINT, int_signal);
        if(argc != 2){
                fprintf(stderr, "USAGE: %s <configfile>\n", argv[0]);
                return 1;
        }else{
                snprintf(configfile, 255, "%s", argv[1]);
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
	// In Sert Your Logic Here
	CMySQL sqlobj;
	if(sqlobj.mcfn_Open(CG_MyAppConfig.GetMysqlIp().c_str(), CG_MyAppConfig.GetMysqlDbName().c_str(), CG_MyAppConfig.GetMysqlUser().c_str(), CG_MyAppConfig.GetMysqlPassword().c_str()) ){
                APP_LOGGER(CG_MyAppLogger, LOG_INFO, "Connected To Data Base");
        }else{
                APP_LOGGER(CG_MyAppLogger, LOG_DEBUG, "Failed To Connect to Data Base %s",CG_MyAppConfig.GetMysqlDbName().c_str() );
                return 0;
        }
	// Open Message Queue
	updateQueryQueue.Open(CG_MyAppConfig.GetUpdateQueryQueueName().c_str(), CG_MyAppConfig.GetUpdateQueryQueueMsgSize(), CG_MyAppConfig.GetUpdateQueryQueueNoOfMsg(), O_RDONLY);
	if(!updateQueryQueue.IsOpened()){
		APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "Failed To Open message queue %s", CG_MyAppConfig.GetUpdateQueryQueueName().c_str());
		return 0;
	}
	IS_PROCESS_RUN = 1;
	while(IS_PROCESS_RUN){
		no_of_msg = updateQueryQueue.NoOfMessagesInQueue();
		if( no_of_msg > MAX_MSG_IN_QUEUE){ 
			APP_LOGGER(CG_MyAppLogger, LOG_DEBUG, "No Of Available Message is %u " , no_of_msg);
		}
		if( no_of_msg > 0){
			memset(msg, 0x00, sizeof(msg));
	                updateQueryQueue.Read(msg, 511);
			if(!sqlobj.mcfb_isConnectionAlive()){
				sleep(MYSQL_RECON_SLEEP);
				sqlobj.mcfn_reconnect();
				APP_LOGGER(CG_MyAppLogger, LOG_WARNING, "RECONNECTING MYSQL");
			}
			if(sqlobj.mcfn_Execute(msg, errNo, errMsg)){
				APP_LOGGER(CG_MyAppLogger, LOG_INFO, "%s", msg);
			}else{
				APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "%s :: %d : %s", msg, errNo, errMsg);
			}
		}else{
			sleep(DFL_SLEEP);
		}
	}
	// Close Logger
        logger_cleanup(CG_MyAppLogger);
	return 0;
}
