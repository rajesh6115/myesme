// Sender Thread Implementation
#include <Esme.hpp>
// TODO: sendThStatus Can Not Be Used For Pause Of Sending, It Should Per SMPP Connection
void *Esme::ThSmsSender(void *arg){
	sms_data_t tmpSms;
	APP_LOGGER(CG_MyAppLogger, LOG_INFO, "Sender Thread Started");
	sendThStatus = TH_ST_RUNNING;
	while(sendThStatus != TH_ST_REQ_STOP){
		std::map<std::string, Esme *>::iterator l_esmeInstanceItr;
		for(l_esmeInstanceItr = esmeInstanceMap.begin(); l_esmeInstanceItr != esmeInstanceMap.end() ; l_esmeInstanceItr++){
			if(l_esmeInstanceItr->second->m_esmeType != BIND_RDONLY){
				Esme *objAddr = l_esmeInstanceItr->second;
				if((objAddr->GetEsmeState() == Esme::ST_BIND) && objAddr->IsSendSms()){
					if(!objAddr->sendQueue.empty()){
						// form pdu and Send
						uint32_t pduseq;
						tmpSms = objAddr->sendQueue.front();
						objAddr->sendQueue.pop();
						pduseq = objAddr->GetNewSequenceNumber();
						// Register to smsdata
						objAddr->RegisterSmsData(pduseq, tmpSms);
						objAddr->SendSubmitSm(pduseq, tmpSms.party_a, tmpSms.party_b, tmpSms.type, (Smpp::Uint8 *)tmpSms.msg, strlen((const char *)tmpSms.msg));
					}else{
						// sleep for some second to give chance to other thread run
						usleep(NO_SMS_QUEUE_USLEEP); // currently only designed to thread functionality
						APP_LOGGER(CG_MyAppLogger, LOG_INFO, "No SMS : %s", objAddr->GetEsmeName().c_str());
					}
				}
//else{
//					usleep(SEND_PAUSE_USLEEP); // Sleep For some time mostly pause condition
					//APP_LOGGER(CG_MyAppLogger, LOG_WARNING, "SENDING OF SMS PAUSE STATE OR NOT Binded : %s : %d",objAddr->GetEsmeName().c_str(), objAddr->GetEsmeState());
//				}
			}
		}
		usleep(SEND_PAUSE_USLEEP);
	}
	sendThStatus = TH_ST_STOP;
	APP_LOGGER(CG_MyAppLogger, LOG_WARNING, "Sender Thread EXITING");
	return NULL;
}
