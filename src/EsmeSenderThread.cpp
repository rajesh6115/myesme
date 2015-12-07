// Sender Thread Implementation
#include <Esme.hpp>
#ifdef WITH_THREAD_POOL
void Esme::SendOneSms(void){
	sms_data_t tmpSms;
	if(this->IsSendSms()){
		if(!this->sendQueue.empty()){
			uint32_t pduseq;
			this->m_sendQueueLock.lock();
			tmpSms = this->sendQueue.front();
			this->sendQueue.pop();
			this->m_sendQueueLock.unlock();
			pduseq = this->GetNewSequenceNumber();
			// Register to smsdata
			this->RegisterSmsData(pduseq, tmpSms);
			this->SendSubmitSm(pduseq, tmpSms.party_a, tmpSms.party_b, tmpSms.type, (Smpp::Uint8 *)tmpSms.msg, strlen((const char *)tmpSms.msg));
		}else{
			APP_LOGGER(CG_MyAppLogger, LOG_WARNING, "No SMS FOR : %s", this->GetEsmeName().c_str());
		}
	}else{
		APP_LOGGER(CG_MyAppLogger, LOG_WARNING, " PAUSE OF SENDING: %s : %u ", this->GetEsmeName().c_str(), this->GetEsmeState());
		if(this->GetEsmeState() == ST_ERROR){
			usleep(10000); // delay by 10ms
		}
		g_threadPool.enqueue(&Esme::SendOneSms, this); // Enque missed event
	}
}

#else

int Esme::StartSender(void){
        sendThStatus = TH_ST_REQ_RUN;
        if(pthread_create(&sendThId, NULL, &(Esme::ThSmsSender), NULL)){
                //fail to create thread
                return -1;
        }
        return 0;
}

int Esme::StopSender(void){
        sendThStatus = TH_ST_REQ_STOP;
        while(sendThStatus == TH_ST_REQ_STOP){
                // Waiting for send thread to stop
                sleep(DFL_SLEEP_VALUE);
        }
        pthread_join(sendThId, NULL);
        return 0;

}

thread_status_t Esme::GetSenderThStatus(void){
        return sendThStatus;
}

// TODO: sendThStatus Can Not Be Used For Pause Of Sending, It Should Per SMPP Connection
void *Esme::ThSmsSender(void *arg){
	unsigned int noSmsThCnt=1;
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
						objAddr->m_sendQueueLock.lock();
						tmpSms = objAddr->sendQueue.front();
						objAddr->sendQueue.pop();
						objAddr->m_sendQueueLock.unlock();
						pduseq = objAddr->GetNewSequenceNumber();
						// Register to smsdata
						objAddr->RegisterSmsData(pduseq, tmpSms);
						objAddr->SendSubmitSm(pduseq, tmpSms.party_a, tmpSms.party_b, tmpSms.type, (Smpp::Uint8 *)tmpSms.msg, strlen((const char *)tmpSms.msg));
						noSmsThCnt = 0;
					}else{
						// sleep for some second to give chance to other thread run
						usleep(NO_SMS_QUEUE_USLEEP); // currently only designed to thread functionality
						APP_LOGGER(CG_MyAppLogger, LOG_INFO, "No SMS : %s", objAddr->GetEsmeName().c_str());
						noSmsThCnt++;
					}
				}
			}
		}
		if(noSmsThCnt)
			usleep(SEND_PAUSE_USLEEP);
	}
	sendThStatus = TH_ST_STOP;
	APP_LOGGER(CG_MyAppLogger, LOG_WARNING, "Sender Thread EXITING");
	return NULL;
}
#endif
