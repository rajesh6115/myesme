#include "Externs.hpp"
#include "MySqlWrapper.hpp"
#include "Defines.hpp"
#include "Esme.hpp"
extern Esme myEsme;
thread_status_t G_CampaignThStatus=TH_ST_IDLE;

uint32_t IsActiveCampaign(void){
        uint32_t campaignId=0;
        int errNo;
        char errMsg[256]={0x00};
        char tempBuff[256]={0x00};
	if ( G_CampaignThStatus == TH_ST_RUNNING){
		APP_LOGGER(CG_MyAppLogger, LOG_DEBUG, "One Campaign is Already Running");
		return 0;
	}
        CMySQL sqlobj;
        if(sqlobj.mcfn_Open(CG_MyAppConfig.GetMysqlIp().c_str(), CG_MyAppConfig.GetMysqlDbName().c_str(), CG_MyAppConfig.GetMysqlUser().c_str(), CG_MyAppConfig.GetMysqlPassword().c_str()) ){
		APP_LOGGER(CG_MyAppLogger, LOG_INFO, "Connected To Data Base");
        }else{
                APP_LOGGER(CG_MyAppLogger, LOG_DEBUG, "Failed To Connect to Data Base %s",CG_MyAppConfig.GetMysqlDbName().c_str() );
                return 0;
        }
        std::string activeCampaignQuery = "SELECT iSNo FROM CampaignMaster WHERE iActive= 1 AND iStatus=";
        sprintf(tempBuff, "%d", CAMPAIGN_ST_SCHEDULED);
        activeCampaignQuery += tempBuff;
        activeCampaignQuery += " AND dtScheduleDatetime < NOW() AND iCampaignType IN (";
	activeCampaignQuery += CG_MyAppConfig.GetCampaignType();
	activeCampaignQuery +=  ") LIMIT 1";
        if(sqlobj.mcfn_GetResultSet(activeCampaignQuery.c_str(), errNo, errMsg)){
		APP_LOGGER(CG_MyAppLogger, LOG_DEBUG, "CAMPAIGN QUERY= %s ", activeCampaignQuery.c_str() );
        }else{
		APP_LOGGER(CG_MyAppLogger, LOG_DEBUG, "CAMPAIGN QUERY= %s ERROR= %s ", activeCampaignQuery.c_str(), errMsg );
                return 0;
        }
        MYSQL_ROW tempRow;
        tempRow = mysql_fetch_row(sqlobj.m_pRecordsetPtr);
        if(tempRow){
		APP_LOGGER(CG_MyAppLogger, LOG_INFO, "Active Campaign Found with ID=%s", tempRow[0]);
                campaignId = atoi(tempRow[0]);
        }
        mysql_free_result(sqlobj.m_pRecordsetPtr);
        return campaignId;
}

void * CampaignThread(void *arg){
	uint32_t campaignId = *(uint32_t *) arg;
	uint32_t campaignStatus = 0;
	int32_t campaign_error_cnt = 0;
	int errNo;
	char errMsg[256]={0x00};
	char tempBuff[256]={0x00};
	CMySQL sqlobj;
	if(myEsme.GetEsmeState() != Esme::ST_BIND){
		// After Esme in BIND state allow Campaign thread
		// Also Add conditiion for type of binding as it is not required for rx
		APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "Esme Not Binded But Campaign Starting....");
		return NULL;
	}
	if(sqlobj.mcfn_Open(CG_MyAppConfig.GetMysqlIp().c_str(), CG_MyAppConfig.GetMysqlDbName().c_str(), CG_MyAppConfig.GetMysqlUser().c_str(), CG_MyAppConfig.GetMysqlPassword().c_str()) ){
		APP_LOGGER(CG_MyAppLogger, LOG_DEBUG, "Connected To Database");
	}else{
		G_CampaignThStatus = TH_ST_STOP ;
		// Log Error
		APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "NOT ABLE TO CONNECT TO MYSQL");
		return NULL;
	}

	std::string campaignDataQuery="SELECT vcSource,iCampaignType FROM CampaignMaster WHERE iSNO=";
	sprintf(tempBuff, "%d", campaignId);
	campaignDataQuery += tempBuff ;
	if(sqlobj.mcfn_GetResultSet(campaignDataQuery.c_str(), errNo, errMsg)){
		APP_LOGGER(CG_MyAppLogger, LOG_DEBUG, "QUERY: %s : SUCESS", campaignDataQuery.c_str());
	}else{
		G_CampaignThStatus = TH_ST_STOP ;
		APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "QUERY: %s : FAIL", campaignDataQuery.c_str());
		return NULL;
	}
	MYSQL_ROW tempRow;
	tempRow = mysql_fetch_row(sqlobj.m_pRecordsetPtr);
	std::cout << "RESULT: " << "CampaignId" << campaignId << ":" <<tempRow[0] << ":" << std::endl;
	mysql_free_result(sqlobj.m_pRecordsetPtr);
	// Update Campaign So that it will not pick again
	std::string campaignStatusUpdateQuery = "UPDATE CampaignMaster SET iStatus=";
	memset(tempBuff, 0x00, sizeof(tempBuff));
	sprintf(tempBuff, "%d", CAMPAIGN_ST_PICKED);
	campaignStatusUpdateQuery += tempBuff;
	campaignStatusUpdateQuery += ",dtStartDatetime=NOW() WHERE iSNo=";
	memset(tempBuff, 0x00, sizeof(tempBuff));
	sprintf(tempBuff, "%d", campaignId);
	campaignStatusUpdateQuery += tempBuff;
	if(sqlobj.mcfn_Execute(campaignStatusUpdateQuery.c_str(), errNo, errMsg)){
		APP_LOGGER(CG_MyAppLogger, LOG_DEBUG,"Query Executed\n");
	}else{
		G_CampaignThStatus = TH_ST_STOP ;
		APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "QUERY: %s : FAIL", campaignStatusUpdateQuery.c_str());
		return NULL;
	}
	// Start Push Processing Campaign
	std::string msisdnPickingQuery = "SELECT iSNo,vcSource,vcMsisdn,vcMessage,iMsgType FROM SMSMT WHERE iCampaignId=";
	memset(tempBuff, 0x00, sizeof(tempBuff));
	sprintf(tempBuff, "%d", campaignId);
	msisdnPickingQuery += tempBuff;
	msisdnPickingQuery +=" AND iStatus=";
	memset(tempBuff, 0x00, sizeof(tempBuff));
	sprintf(tempBuff, "%d",  SMS_ST_SCHEDULED);
	msisdnPickingQuery += tempBuff;
	msisdnPickingQuery += " LIMIT ";
	memset(tempBuff, 0x00, sizeof(tempBuff));
	sprintf(tempBuff, "%d", SMS_CAMPAIGN_WINDOW_SIZE);
	msisdnPickingQuery += tempBuff;
	G_CampaignThStatus = TH_ST_RUNNING;
	while(G_CampaignThStatus == TH_ST_RUNNING){
		std::string msisdnWindowUpdate = "UPDATE SMSMT SET iStatus=";
		memset(tempBuff, 0x00, sizeof(tempBuff));
		sprintf(tempBuff, "%d", SMS_ST_PICKED);
		msisdnWindowUpdate += tempBuff;
		msisdnWindowUpdate += " WHERE iCampaignId=";
		memset(tempBuff, 0x00, sizeof(tempBuff));
		sprintf(tempBuff, "%d", campaignId);
		msisdnWindowUpdate += tempBuff;
		msisdnWindowUpdate += " AND iSNo IN (";
		std::string submittedMsisdn;
		if(!sqlobj.mcfb_isConnectionAlive()){
			sqlobj.mcfn_reconnect();
			APP_LOGGER(CG_MyAppLogger, LOG_WARNING, "RECONNECTING MYSQL");
		} // Temp Solution Of Mysql Dropped Connection issue
		if(sqlobj.mcfn_GetResultSet(msisdnPickingQuery.c_str(), errNo, errMsg)){
			//std::cout << "Query Executed" << msisdnPickingQuery << std::endl;
			campaign_error_cnt = 0; // reset Counter As Able to connect to Mysql
		}else{
			APP_LOGGER(CG_MyAppLogger, LOG_DEBUG, "Problem in Excuting Query : %s ", msisdnPickingQuery.c_str());
			// Set ERROR Message/state
			//break;
			// 1.TODO Below are workarround for Mysql Dropped Connection issue
			campaign_error_cnt ++;
			if(campaign_error_cnt > MAX_CAMPAIN_ERROR){
				APP_LOGGER(CG_MyAppLogger, LOG_WARNING, "Maximum Error Happen For Mysql Stopping Campaign with ID= %d", campaignId);
				break;
			}else{
				continue; // Try To Get Campaign Data Again From Mysql 
			}
		}
		MYSQL_ROW tempRow=NULL;
		sms_data_t tempSmsData;
		tempRow = mysql_fetch_row(sqlobj.m_pRecordsetPtr);
		while(tempRow){
			// Prepare sms data
			memset(&tempSmsData, 0x00, sizeof(tempSmsData));
			tempSmsData.id = atoi(tempRow[0]);
			tempSmsData.status= SMS_ST_PICKED;
			if(tempRow[1]){
				strncpy((char *)tempSmsData.party_a, tempRow[1], 15);
			}
			if(tempRow[2]){
				strncpy((char *)tempSmsData.party_b, tempRow[2], 15);
			}
			if(tempRow[3]){
				strncpy((char *)tempSmsData.msg, tempRow[3], 511);
			}
			if(tempRow[4]){
				tempSmsData.type = atoi(tempRow[4]);
			}
			myEsme.SendSms(tempSmsData);
			// Update Query Nos
			submittedMsisdn += tempRow[0];
			submittedMsisdn += ",";
			tempRow = mysql_fetch_row(sqlobj.m_pRecordsetPtr);
		}
		mysql_free_result(sqlobj.m_pRecordsetPtr);
		if(submittedMsisdn.length()){
			submittedMsisdn.pop_back();
			msisdnWindowUpdate += submittedMsisdn;
			msisdnWindowUpdate += ")";
			// 1.TODO Mysql Connection Timeout issue
			if(!sqlobj.mcfb_isConnectionAlive()){
				sqlobj.mcfn_reconnect();
				APP_LOGGER(CG_MyAppLogger, LOG_WARNING, "RECONNECTING MYSQL");
			} // Temp Solution Of Mysql Dropped Connection issue
			if(sqlobj.mcfn_Execute(msisdnWindowUpdate.c_str(), errNo, errMsg)){
				APP_LOGGER(CG_MyAppLogger, LOG_DEBUG,"Query Executed");
				campaign_error_cnt = 0 ; // reset counter 
			}else{
				APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "Problem in Excuting Query: %s :: %d :: %s", msisdnWindowUpdate.c_str(), errNo, errMsg);
				// Set ERROR Message/status
				//break;
				campaign_error_cnt ++;
				if(campaign_error_cnt > MAX_CAMPAIN_ERROR){
					APP_LOGGER(CG_MyAppLogger, LOG_WARNING, "Maximum Error Happen For Mysql Stopping Campaign with ID= %d", campaignId);
					break;
				}
			}
		}else{
			// No Further Record
			break;
		}
		sleep(1);//usleep(); // Window Sleep
	}
	// Once Done Campaign Update Campaign As Completed
	//use same campaignUpdateQuery Variable
	campaignStatusUpdateQuery = "UPDATE CampaignMaster SET iStatus=";
	memset(tempBuff, 0x00, sizeof(tempBuff));
	sprintf(tempBuff, "%d", CAMPAIGN_ST_COMPLETED);
	campaignStatusUpdateQuery += tempBuff;
	campaignStatusUpdateQuery += ",dtCompleteDatetime=NOW() WHERE iSNo=";
	memset(tempBuff, 0x00, sizeof(tempBuff));
	sprintf(tempBuff, "%d", campaignId);
	campaignStatusUpdateQuery += tempBuff;
	if(sqlobj.mcfn_Execute(campaignStatusUpdateQuery.c_str(), errNo, errMsg)){
		APP_LOGGER(CG_MyAppLogger, LOG_DEBUG, "Query Executed\n" );
	}else{
		APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "Problem in Excuting Query: %s", campaignStatusUpdateQuery.c_str());
		G_CampaignThStatus = TH_ST_STOP ;
		return NULL;
	}
	G_CampaignThStatus = TH_ST_STOP ;
	return NULL;
}
