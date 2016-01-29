// Esme Pdu Processing Thread Implementation
#include <Esme.hpp>
#ifdef WITH_THREAD_POOL
void Esme::OnReceivePdu(void){
	Smpp::Uint32 cmdId;
	NetBuffer tmpNetBuf;
	if(!this->receiveQueue.empty()){
		// if not empty
		// Protecting ritical Section , As std::queue not thread safe
		this->m_rcvQueueLock.lock();
		tmpNetBuf = this->receiveQueue.front();
		this->receiveQueue.pop();
		this->m_rcvQueueLock.unlock();
		cmdId = Smpp::get_command_id((Smpp::Uint8 *)tmpNetBuf.GetBuffer());
		APP_LOGGER(CG_MyAppLogger, LOG_INFO, "PROCESSING PDU ID:SEQ:STATUS %08X:%08X:%08X", cmdId, Smpp::get_sequence_number((Smpp::Uint8 *)tmpNetBuf.GetBuffer()), Smpp::get_command_status((Smpp::Uint8 *)tmpNetBuf.GetBuffer()));
		Smpp::Uint32 st = Smpp::get_command_status((Smpp::Uint8 *)tmpNetBuf.GetBuffer());
		if( st != Smpp::CommandStatus::ESME_ROK ){
			APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "PDU ERROR= %08X:%s", st, Smpp::CommandStatus::description(st).c_str());
		}
		switch(cmdId){
			case Smpp::CommandId::BindReceiver:
				{
					// should Not Recive
				}
				break;
			case Smpp::CommandId::BindTransmitter:
				{
					// should not Receive
				}
				break;
			case Smpp::CommandId::BindTransceiver:
				{
					// should not receive
				}
				break;
			case Smpp::CommandId::Unbind:
				{
					// should not receive
				}
				break;
			case Smpp::CommandId::BindReceiverResp:
				{
					// change state 
					APP_LOGGER(CG_MyAppLogger, LOG_DEBUG, "Bind Receiver Response");
					Smpp::BindReceiverResp resp((Smpp::Uint8 *)tmpNetBuf.GetBuffer());
					std::string smscId = resp.system_id();
					APP_LOGGER(CG_MyAppLogger, LOG_INFO, "SMSCID=%s", smscId.c_str());
					// Remove from Map As Sucessfully Response We Got
					this->UnRegisterPdu(tmpNetBuf);
					// check status and change state
					if(resp.command_status() == 0){
						this->m_esmeState = ST_BIND;
					}else{
						this->m_esmeState = ST_BIND_FAIL;
						if( Smpp::CommandStatus::ESME_RALYBND == resp.command_status()){
							APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "Already Binded, Why Again Binding? %s", this->GetEsmeName().c_str());
							this->m_esmeState = ST_BIND;
						}
					}
				}
				break;
			case Smpp::CommandId::BindTransmitterResp:
				{
					// change state
					Smpp::BindTransmitterResp resp((Smpp::Uint8 *)tmpNetBuf.GetBuffer());
					std::string smscId = resp.system_id();
					APP_LOGGER(CG_MyAppLogger, LOG_INFO, "SMSCID=%s", smscId.c_str());
					this->UnRegisterPdu(tmpNetBuf);
					// check status and change state
					if(resp.command_status() == 0){
						this->m_esmeState = ST_BIND;
						APP_LOGGER(CG_MyAppLogger, LOG_DEBUG,"Bind Transmitter Response");
					}else{
						this->m_esmeState = ST_BIND_FAIL;
						APP_LOGGER(CG_MyAppLogger, LOG_ERROR,"Bind Transmitter Response FAIL %d ", resp.command_status());
						if( Smpp::CommandStatus::ESME_RALYBND == resp.command_status()){
							APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "Already Binded, Why Again Binding? %s", this->GetEsmeName().c_str());
							this->m_esmeState = ST_BIND;
						}
					}
				}
				break;
			case Smpp::CommandId::BindTransceiverResp:
				{ // change state
					APP_LOGGER(CG_MyAppLogger, LOG_DEBUG, "Bind TransReceiver Response");
					Smpp::BindTransceiverResp resp((Smpp::Uint8 *)tmpNetBuf.GetBuffer());
					std::string smscId = resp.system_id();
					APP_LOGGER(CG_MyAppLogger, LOG_INFO, "SMSCID=%s", smscId.c_str());
					this->UnRegisterPdu(tmpNetBuf);
					// check status and change state
					if(resp.command_status() == 0){
						this->m_esmeState = ST_BIND;
					}else{
						this->m_esmeState = ST_BIND_FAIL;
						if( Smpp::CommandStatus::ESME_RALYBND == resp.command_status()){
							APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "Already Binded, Why Again Binding? %s", this->GetEsmeName().c_str());
							this->m_esmeState = ST_BIND;
						}
					}
				}
				break;
			case Smpp::CommandId::UnbindResp:
				{
					// Change State
					APP_LOGGER(CG_MyAppLogger, LOG_DEBUG,"Bind Unbind Response");
					this->UnRegisterPdu(tmpNetBuf);
					// check status and change state
					this->m_esmeState = ST_UNBIND;
				}
				break;
			case Smpp::CommandId::EnquireLinkResp:
				{
					APP_LOGGER(CG_MyAppLogger, LOG_DEBUG, "Enquire Link Response" );
					this->UnRegisterPdu(tmpNetBuf);
					// Reset Enquire Link Timer
				}
				break;
			case Smpp::CommandId::GenericNack:
				{
					APP_LOGGER(CG_MyAppLogger, LOG_DEBUG,"Generic NACK");
					// Received Negative Ack for PDU
					// Log Error and Decide to Re-Transmit
				}
				break;
			case Smpp::CommandId::AlertNotification:
				{
					APP_LOGGER(CG_MyAppLogger, LOG_DEBUG, "Alert Notification ");
					// Only Valid in BindReceiver
					// No Resp Associated with this pdu
				}
				break;
			case Smpp::CommandId::SubmitSm:
				{
					APP_LOGGER(CG_MyAppLogger, LOG_DEBUG, "Submit Sm ");
					// To Submit Single SMS
					// Esme Should not Receive This
				}
				break;
			case Smpp::CommandId::SubmitSmResp:
				{//0x80000004
					APP_LOGGER(CG_MyAppLogger, LOG_DEBUG,"Submit Sm Resp " );
					uint32_t seqNum= Smpp::get_sequence_number((Smpp::Uint8 *)tmpNetBuf.GetBuffer());
					Smpp::SubmitSmResp pdu; //((Smpp::Uint8 *)tmpNetBuf.GetBuffer());
					try{
						pdu.decode((Smpp::Uint8 *)tmpNetBuf.GetBuffer());
						// UnRegister the pdu
						this->UnRegisterPdu(tmpNetBuf);
						this->DoDatabaseUpdate(tmpNetBuf); // for All Data Base Related Insert/Update
						if( st == Smpp::CommandStatus::ESME_ROK ){
							this->UnRegisterSmsData(seqNum);
						}
						if(!this->m_isSendSms)
							this->m_isSendSms = true;
					}catch(...){
						APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "DO NOT SUBMIT MORE WAIT FOR SOME TIME: ERROR %d", pdu.command_status());
						this->m_isSendSms = false;
						// Retrive SMS From SMS MAP sendSmsMap
						// TODO: Resend Logic Should Not Be Here
						if(this->sendSmsMap.find(seqNum)!= this->sendSmsMap.end()){
							sms_data_t tmpSms = this->sendSmsMap[seqNum];
							//this->UnRegisterSmsData(seqNum); // TODO: why to unregister, if it is same pdu only
							usleep(RESEND_USLEEP); // Put Some Sleep To Give Server Little Time
							this->SendSubmitSm(seqNum, tmpSms.party_a, (Smpp::Uint8)tmpSms.src_ton, (Smpp::Uint8)tmpSms.src_npi, tmpSms.party_b, (Smpp::Uint8)tmpSms.dest_ton, (Smpp::Uint8)tmpSms.dest_npi, tmpSms.type, (Smpp::Uint8 *)tmpSms.msg, strlen((const char *)tmpSms.msg));
							//this->SendSms(tmpSms);
						}else{
							this->m_isSendSms = true;
							APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "SMS DATA NOT AVAILABLE FOR %u", seqNum);
						}

					}

				}
				break;
			case Smpp::CommandId::DataSm:
				{// Can be used for send SMS
				}
				break;
			case Smpp::CommandId::DataSmResp:
				{// unregister data sm 
				}
				break;
			case Smpp::CommandId::DeliverSm:
				{
					// Delivery Report
					// Received SMS
					Smpp::DeliverSm pdu;
					try{
						pdu.decode((Smpp::Uint8 *)tmpNetBuf.GetBuffer());
					}catch(...){
						APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "ERROR IN DECODING DELIVERSM " );
						break; // error condition break
					}
					Smpp::Uint8 esm = pdu.esm_class();
					if(esm&0x04){
						// Delivery Receipt
						Smpp::DeliverSmResp pduRsp;
						NetBuffer tempBuffer;
						pduRsp.command_status(0);
						pduRsp.sequence_number(pdu.sequence_number());
						tempBuffer.Append(pduRsp.encode(), pduRsp.command_length());
						this->Write(tempBuffer);
						APP_LOGGER(CG_MyAppLogger, LOG_DEBUG,"This is a Delivery Receipt");
						// Update Database Here For Delivery Report
						this->DoDatabaseUpdate(tmpNetBuf); // for All Data Base Related Insert/Update

					}else{
						// Return DeliverySmResp
						APP_LOGGER(CG_MyAppLogger, LOG_DEBUG, "One SMS Received");
						Smpp::DeliverSmResp pduRsp;
						Smpp::String destAddr = pdu.destination_addr().address() ;
						Smpp::String srcAddr = pdu.source_addr().address() ;
						NetBuffer tempBuffer;
						char msgId[65] = {0x00};
						sprintf(msgId, "%s-%s-%ld", destAddr.c_str(), srcAddr.c_str(), time(NULL));
						pduRsp.message_id(msgId);
						pduRsp.command_status(0);
						pduRsp.sequence_number(pdu.sequence_number());
						// write to nw
						tempBuffer.Append(pduRsp.encode(), pduRsp.command_length());
						this->Write(tempBuffer);
					}
				}
				break;
			case Smpp::CommandId::DeliverSmResp:
				{
					// Should not Receive
				}
				break;
			default:
				{
					// Log for Un Handle CMDID
					APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "WHY I RECEIVED IT cmd_id %d", cmdId);
				}
				break;
		}
		APP_LOGGER(CG_MyAppLogger, LOG_INFO, "PROCESSING PDU ID %08X:%08X COMPLETED", cmdId, Smpp::get_sequence_number((Smpp::Uint8 *)tmpNetBuf.GetBuffer()));

	}

}
#else

int Esme::StartPduProcess(void){
	pduProcessThStatus = TH_ST_REQ_RUN;
	if(pthread_create(&pduProcessThId, NULL, &(Esme::ThOnReceivePdu), NULL)){
		//fail to create thread
		return -1;
	}
	return 0;

}

int Esme::StopPduProcess(void){
	if(pduProcessThStatus == TH_ST_STOP){
		return 0; // Already Stop state
	}
	pduProcessThStatus = TH_ST_REQ_STOP;
	while(pduProcessThStatus == TH_ST_REQ_STOP){
		// Waiting for receive thread to stop
		sleep(DFL_SLEEP_VALUE);
	}
	pthread_join(pduProcessThId, NULL);
	return 0;

}

thread_status_t Esme::GetPduProcessThStatus(void){
	return pduProcessThStatus;
}

void *Esme::ThOnReceivePdu(void *arg){
	APP_LOGGER(CG_MyAppLogger, LOG_INFO, "Pdu Processing Thread Started");
	std::map<std::string, Esme *>::iterator l_esmeInstanceItr;
	pduProcessThStatus = TH_ST_RUNNING;
	while(pduProcessThStatus == TH_ST_RUNNING){
		for(l_esmeInstanceItr = esmeInstanceMap.begin(); l_esmeInstanceItr != esmeInstanceMap.end() ; l_esmeInstanceItr++){
			Esme *objAddr = l_esmeInstanceItr->second;
			Smpp::Uint32 cmdId;
			NetBuffer tmpNetBuf;
			if(!objAddr->receiveQueue.empty()){
				// if not empty
				tmpNetBuf = objAddr->receiveQueue.front();
				objAddr->receiveQueue.pop();
				cmdId = Smpp::get_command_id((Smpp::Uint8 *)tmpNetBuf.GetBuffer());
				APP_LOGGER(CG_MyAppLogger, LOG_INFO, "PROCESSING PDU ID:SEQ:STATUS %08X:%08X:%08X", cmdId, Smpp::get_sequence_number((Smpp::Uint8 *)tmpNetBuf.GetBuffer()), Smpp::get_command_status((Smpp::Uint8 *)tmpNetBuf.GetBuffer()));
				Smpp::Uint32 st = Smpp::get_command_status((Smpp::Uint8 *)tmpNetBuf.GetBuffer());
				if( st != Smpp::CommandStatus::ESME_ROK ){
					APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "PDU ERROR= %08X:%s", st, Smpp::CommandStatus::description(st).c_str());
				}
				switch(cmdId){
					case Smpp::CommandId::BindReceiver:
						{
							// should Not Recive
						}
						break;
					case Smpp::CommandId::BindTransmitter:
						{
							// should not Receive
						}
						break;
					case Smpp::CommandId::BindTransceiver:
						{
							// should not receive
						}
						break;
					case Smpp::CommandId::Unbind:
						{
							// should not receive
						}
						break;
					case Smpp::CommandId::BindReceiverResp:
						{
							// change state 
							APP_LOGGER(CG_MyAppLogger, LOG_DEBUG, "Bind Receiver Response");
							Smpp::BindReceiverResp resp((Smpp::Uint8 *)tmpNetBuf.GetBuffer());
							std::string smscId = resp.system_id();
							APP_LOGGER(CG_MyAppLogger, LOG_INFO, "SMSCID=%s", smscId.c_str());
							// Remove from Map As Sucessfully Response We Got
							objAddr->UnRegisterPdu(tmpNetBuf);
							// check status and change state
							if(resp.command_status() == 0){
								objAddr->m_esmeState = ST_BIND;
							}else{
								objAddr->m_esmeState = ST_BIND_FAIL;
								if( Smpp::CommandStatus::ESME_RALYBND == resp.command_status()){
									APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "Already Binded, Why Again Binding? %s", objAddr->GetEsmeName().c_str());
									objAddr->m_esmeState = ST_BIND;
								}
							}
						}
						break;
					case Smpp::CommandId::BindTransmitterResp:
						{
							// change state
							Smpp::BindTransmitterResp resp((Smpp::Uint8 *)tmpNetBuf.GetBuffer());
							std::string smscId = resp.system_id();
							APP_LOGGER(CG_MyAppLogger, LOG_INFO, "SMSCID=%s", smscId.c_str());
							objAddr->UnRegisterPdu(tmpNetBuf);
							// check status and change state
							if(resp.command_status() == 0){
								objAddr->m_esmeState = ST_BIND;
								APP_LOGGER(CG_MyAppLogger, LOG_DEBUG,"Bind Transmitter Response");
							}else{
								objAddr->m_esmeState = ST_BIND_FAIL;
								APP_LOGGER(CG_MyAppLogger, LOG_ERROR,"Bind Transmitter Response FAIL %d ", resp.command_status());
								if( Smpp::CommandStatus::ESME_RALYBND == resp.command_status()){
									APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "Already Binded, Why Again Binding? %s", objAddr->GetEsmeName().c_str());
									objAddr->m_esmeState = ST_BIND;
								}
							}
						}
						break;
					case Smpp::CommandId::BindTransceiverResp:
						{ // change state
							APP_LOGGER(CG_MyAppLogger, LOG_DEBUG, "Bind TransReceiver Response");
							Smpp::BindTransceiverResp resp((Smpp::Uint8 *)tmpNetBuf.GetBuffer());
							std::string smscId = resp.system_id();
							APP_LOGGER(CG_MyAppLogger, LOG_INFO, "SMSCID=%s", smscId.c_str());
							objAddr->UnRegisterPdu(tmpNetBuf);
							// check status and change state
							if(resp.command_status() == 0){
								objAddr->m_esmeState = ST_BIND;
							}else{
								objAddr->m_esmeState = ST_BIND_FAIL;
								if( Smpp::CommandStatus::ESME_RALYBND == resp.command_status()){
									APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "Already Binded, Why Again Binding? %s", objAddr->GetEsmeName().c_str());
									objAddr->m_esmeState = ST_BIND;
								}
							}
						}
						break;
					case Smpp::CommandId::UnbindResp:
						{
							// Change State
							APP_LOGGER(CG_MyAppLogger, LOG_DEBUG,"Bind Unbind Response");
							objAddr->UnRegisterPdu(tmpNetBuf);
							// check status and change state
							objAddr->m_esmeState = ST_UNBIND;
						}
						break;
					case Smpp::CommandId::EnquireLinkResp:
						{
							APP_LOGGER(CG_MyAppLogger, LOG_DEBUG, "Enquire Link Response" );
							objAddr->UnRegisterPdu(tmpNetBuf);
							// Reset Enquire Link Timer
						}
						break;
					case Smpp::CommandId::GenericNack:
						{
							APP_LOGGER(CG_MyAppLogger, LOG_DEBUG,"Generic NACK");
							// Received Negative Ack for PDU
							// Log Error and Decide to Re-Transmit
						}
						break;
					case Smpp::CommandId::AlertNotification:
						{
							APP_LOGGER(CG_MyAppLogger, LOG_DEBUG, "Alert Notification ");
							// Only Valid in BindReceiver
							// No Resp Associated with this pdu
						}
						break;
					case Smpp::CommandId::SubmitSm:
						{
							APP_LOGGER(CG_MyAppLogger, LOG_DEBUG, "Submit Sm ");
							// To Submit Single SMS
							// Esme Should not Receive This
						}
						break;
					case Smpp::CommandId::SubmitSmResp:
						{//0x80000004
							APP_LOGGER(CG_MyAppLogger, LOG_DEBUG,"Submit Sm Resp " );
							uint32_t seqNum= Smpp::get_sequence_number((Smpp::Uint8 *)tmpNetBuf.GetBuffer());
							Smpp::SubmitSmResp pdu; //((Smpp::Uint8 *)tmpNetBuf.GetBuffer());
							try{
								pdu.decode((Smpp::Uint8 *)tmpNetBuf.GetBuffer());
								// UnRegister the pdu
								objAddr->UnRegisterPdu(tmpNetBuf);
								objAddr->DoDatabaseUpdate(tmpNetBuf); // for All Data Base Related Insert/Update
								// Unregister from smsdata
								objAddr->UnRegisterSmsData(seqNum);
								if(!objAddr->m_isSendSms)
									objAddr->m_isSendSms = true;
							}catch(...){
								APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "DO NOT SUBMIT MORE WAIT FOR SOME TIME: ERROR %d", pdu.command_status());
								objAddr->m_isSendSms = false;
								// Retrive SMS From SMS MAP sendSmsMap
								// TODO: Resend Logic Should Not Be Here
								if(objAddr->sendSmsMap.find(seqNum)!= objAddr->sendSmsMap.end()){
									sms_data_t tmpSms = objAddr->sendSmsMap[seqNum];
									usleep(RESEND_USLEEP); // Put Some Sleep To Give Server Little Time
									objAddr->SendSubmitSm(seqNum, tmpSms.party_a, tmpSms.party_b, tmpSms.type, (Smpp::Uint8 *)tmpSms.msg, strlen((const char *)tmpSms.msg));
								}else{
									objAddr->m_isSendSms = true;
									APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "SMS DATA NOT AVAILABLE FOR %u", seqNum);
								}

							}

						}
						break;
					case Smpp::CommandId::DataSm:
						{// Can be used for send SMS
						}
						break;
					case Smpp::CommandId::DataSmResp:
						{// unregister data sm 
						}
						break;
					case Smpp::CommandId::DeliverSm:
						{
							// Delivery Report
							// Received SMS
							Smpp::DeliverSm pdu;
							try{
								pdu.decode((Smpp::Uint8 *)tmpNetBuf.GetBuffer());
							}catch(...){
								APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "ERROR IN DECODING DELIVERSM " );
								break; // error condition break
							}
							Smpp::Uint8 esm = pdu.esm_class();
							if(esm&0x04){
								// Delivery Receipt
								Smpp::DeliverSmResp pduRsp;
								NetBuffer tempBuffer;
								pduRsp.command_status(0);
								pduRsp.sequence_number(pdu.sequence_number());
								tempBuffer.Append(pduRsp.encode(), pduRsp.command_length());
								objAddr->Write(tempBuffer);
								APP_LOGGER(CG_MyAppLogger, LOG_DEBUG,"This is a Delivery Receipt");
								// Update Database Here For Delivery Report
								objAddr->DoDatabaseUpdate(tmpNetBuf); // for All Data Base Related Insert/Update

							}else{
								// Return DeliverySmResp
								APP_LOGGER(CG_MyAppLogger, LOG_DEBUG, "One SMS Received");
								Smpp::DeliverSmResp pduRsp;
								Smpp::String destAddr = pdu.destination_addr().address() ;
								Smpp::String srcAddr = pdu.source_addr().address() ;
								NetBuffer tempBuffer;
								char msgId[65] = {0x00};
								sprintf(msgId, "%s-%s-%ld", destAddr.c_str(), srcAddr.c_str(), time(NULL));
								pduRsp.message_id(msgId);
								pduRsp.command_status(0);
								pduRsp.sequence_number(pdu.sequence_number());
								// write to nw
								tempBuffer.Append(pduRsp.encode(), pduRsp.command_length());
								objAddr->Write(tempBuffer);
							}
						}
						break;
					case Smpp::CommandId::DeliverSmResp:
						{
							// Should not Receive
						}
						break;
					default:
						{
							// Log for Un Handle CMDID
							APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "WHY I RECEIVED IT cmd_id %d", cmdId);
						}
						break;
				}
				APP_LOGGER(CG_MyAppLogger, LOG_INFO, "PROCESSING PDU ID %08X:%08X COMPLETED", cmdId, Smpp::get_sequence_number((Smpp::Uint8 *)tmpNetBuf.GetBuffer()));

			}

		}
		usleep(PDU_PROCESS_USLEEP); // Some Sleep For Give Chance to Other Threads
	}
	APP_LOGGER(CG_MyAppLogger, LOG_WARNING, "PDU PROCESS THREAD EXITING");
	pduProcessThStatus = TH_ST_STOP;
	return NULL;

}
#endif
