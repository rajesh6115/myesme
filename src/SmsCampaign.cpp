#include "Externs.hpp"
#include "MySqlWrapper.hpp"
#include "Defines.hpp"
#include "Esme.hpp"

uint32_t IsActiveCampaign(void){
        uint32_t campaignId=0;
        int errNo;
        char errMsg[256]={0x00};
        char tempBuff[256]={0x00};
	if ( G_smsCampaignMap.size() >=  MAX_SMS_CAMPAIGN){
		APP_LOGGER(CG_MyAppLogger, LOG_WARNING, "LIMIT: Maximum %d Campaigns Can Run in Parallel", MAX_SMS_CAMPAIGN);
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
	char l_tableName[16] = {0x00};
	uint32_t campaignStatus = 0;
	int32_t campaign_error_cnt = 0;
	int errNo;
	char errMsg[256]={0x00};
	char tempBuff[256]={0x00};
	CMySQL sqlobj;
	std::map<uint32_t, campaign_info_t>::iterator l_campaignItr;
	l_campaignItr = G_smsCampaignMap.find(campaignId);
	if(l_campaignItr != G_smsCampaignMap.end()){
		l_campaignItr->second.thStatus = TH_ST_RUNNING;
	}else{
		APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "No Entries in Campaign Map ID %u Exiting...", campaignId);
		return NULL;
	}
	// Prepare SMPP Connection List
	std::list<Esme *> l_esmeInstanceList;
	std::list<Esme *>::iterator l_esmeItr = l_esmeInstanceList.begin();
	Esme *tempSmppConObj=NULL;
	for(int i=0; i< CG_MyAppConfig.GetMaxinumSmppConnection(); i++){
#ifdef WITH_LIBEVENT
		tempSmppConObj = Esme::GetEsmeInstance(CG_MyAppConfig.GetSmppConnectionName(i), CG_MyAppConfig.GetSmppConnectionConfigFile(i), NULL);
#else
		tempSmppConObj = Esme::GetEsmeInstance(CG_MyAppConfig.GetSmppConnectionName(i), CG_MyAppConfig.GetSmppConnectionConfigFile(i));
#endif
		if(tempSmppConObj){
			if(tempSmppConObj->GetEsmeType() != BIND_RDONLY){
				// Add to List
				l_esmeInstanceList.push_back(tempSmppConObj);
			}
		}
	}
	if(l_esmeInstanceList.empty()){
		APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "ERROR: No Tx OR TRx Bind Available");
		l_campaignItr->second.thStatus = TH_ST_STOP ;
		sleep(SMS_CAMPAIGN_PICK_SLEEP); // Give Some Sleep Before Picking Same Campaign Again
		return NULL;
	}

	if(sqlobj.mcfn_Open(CG_MyAppConfig.GetMysqlIp().c_str(), CG_MyAppConfig.GetMysqlDbName().c_str(), CG_MyAppConfig.GetMysqlUser().c_str(), CG_MyAppConfig.GetMysqlPassword().c_str()) ){
		APP_LOGGER(CG_MyAppLogger, LOG_DEBUG, "Connected To Database");
	}else{
		l_campaignItr->second.thStatus = TH_ST_STOP ;
		APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "NOT ABLE TO CONNECT TO MYSQL");
		return NULL;
	}

	std::string campaignDataQuery="SELECT vcSource,vcTableName FROM CampaignMaster WHERE iSNO=";
	sprintf(tempBuff, "%d", campaignId);
	campaignDataQuery += tempBuff ;
	if(sqlobj.mcfn_GetResultSet(campaignDataQuery.c_str(), errNo, errMsg)){
		APP_LOGGER(CG_MyAppLogger, LOG_DEBUG, "QUERY: %s : SUCESS", campaignDataQuery.c_str());
	}else{
		l_campaignItr->second.thStatus = TH_ST_STOP ;
		APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "QUERY: %s : FAIL", campaignDataQuery.c_str());
		return NULL;
	}
	MYSQL_ROW tempRow;
	tempRow = mysql_fetch_row(sqlobj.m_pRecordsetPtr);
	//std::cout << "RESULT: " << "CampaignId" << campaignId << ":" <<tempRow[0] << ":" << std::endl;
	APP_LOGGER(CG_MyAppLogger, LOG_DEBUG, "RESULT: %d : %s ", campaignId, tempRow[0]);
	memset(l_tableName, 0x00, sizeof(l_tableName));
	strcpy(l_tableName, tempRow[1]);
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
		l_campaignItr->second.thStatus = TH_ST_STOP ;
		APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "QUERY: %s : FAIL", campaignStatusUpdateQuery.c_str());
		return NULL;
	}
	// Start Push Processing Campaign
	std::string msisdnPickingQuery = "SELECT iSNo,vcSource,vcMsisdn,vcMessage,iMsgType,iSrcTon,iSrcNpi,iDestTon,iDestNpi FROM ";
	msisdnPickingQuery += l_tableName;
	msisdnPickingQuery += " WHERE iCampaignId=";
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
	l_campaignItr->second.thStatus = TH_ST_RUNNING;
	while(l_campaignItr->second.thStatus == TH_ST_RUNNING){
		// TODO: If  Time Reached 9 PM Stop Campaign
		std::string msisdnWindowUpdate = "UPDATE ";
		msisdnWindowUpdate += l_tableName;
		msisdnWindowUpdate += " SET iStatus=";
		memset(tempBuff, 0x00, sizeof(tempBuff));
		sprintf(tempBuff, "%d", SMS_ST_PICKED);
		msisdnWindowUpdate += tempBuff;
		msisdnWindowUpdate += ",dtSubmitDatetime=NOW() WHERE iCampaignId=";
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
			APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "MYSQLERROR: %d:%s ", errNo, errMsg);
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
		l_esmeItr = l_esmeInstanceList.begin();
		tempRow = mysql_fetch_row(sqlobj.m_pRecordsetPtr);
		while(tempRow){
			// Prepare sms data
			memset(&tempSmsData, 0x00, sizeof(tempSmsData));
			tempSmsData.id = atoi(tempRow[0]);
			tempSmsData.status= SMS_ST_PICKED;
			strncpy((char *)tempSmsData.table, l_tableName, 15);
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
			//iSrcTon
			if(tempRow[5]){
				tempSmsData.src_ton = atoi(tempRow[5]);
			}
			//iSrcNpi
			if(tempRow[6]){
				tempSmsData.src_npi = atoi(tempRow[6]);
			}
			//iDestTon
			if(tempRow[7]){
				tempSmsData.dest_ton = atoi(tempRow[7]);
			}
			//iDestNpi
			if(tempRow[8]){
				tempSmsData.dest_npi = atoi(tempRow[8]);
			}
			//myEsme.SendSms(tempSmsData);
			unsigned long l_campaign_usleep=SMS_CAMPAIGN_ENQUEUE_USLEEP;
			while(*l_esmeItr){
				if(((*l_esmeItr)->NoOfSmsInQueue() > SMS_CAMPAIGN_QUEUE_SIZE) || (!(*l_esmeItr)->IsSendSms())){
					APP_LOGGER(CG_MyAppLogger, LOG_WARNING, "PAUSE QUEUEING OF SMS FOR %u MICROSEC TO HANDLE ERRORS %s : %d", SMS_CAMPAIGN_ENQUEUE_USLEEP, (*l_esmeItr)->GetEsmeName().c_str(), (*l_esmeItr)->GetEsmeState());
					usleep(l_campaign_usleep);
					l_campaign_usleep += 200000;
					if(l_campaign_usleep == 1000000){
						l_campaign_usleep = SMS_CAMPAIGN_ENQUEUE_USLEEP;
					}
					l_esmeItr++;
					if(l_esmeItr == l_esmeInstanceList.end()){
						l_esmeItr = l_esmeInstanceList.begin();
					}
				}else{
					(*l_esmeItr)->SendSms(tempSmsData);
					break;
				}
			}
			l_esmeItr++;
			if(l_esmeItr == l_esmeInstanceList.end()){
				l_esmeItr = l_esmeInstanceList.begin();
			}
			// Update Query Nos
			submittedMsisdn += tempRow[0];
			submittedMsisdn += ",";
			tempRow = mysql_fetch_row(sqlobj.m_pRecordsetPtr);
		}
		if(sqlobj.m_pRecordsetPtr)
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
				APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "MYSQLERROR: %d:%s ", errNo, errMsg);
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
		usleep(CAMPAIGN_WINDOW_USLEEP);//usleep(); // Window Sleep
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
		l_campaignItr->second.thStatus = TH_ST_STOP ;
		return NULL;
	}
	l_campaignItr->second.thStatus = TH_ST_STOP ;
	return NULL;
}
