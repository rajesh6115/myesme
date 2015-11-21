#include "Esme.hpp"
Smpp::Uint32 Esme::esmePduSequenceNum = Smpp::SequenceNumber::Min;
Esme::~Esme(void){
}

Esme::Esme(std::string host, uint32_t port){
	this->host = host;
	this->port = port;
	smscSocket = -1;
	bzero(&smscInfo, sizeof(struct sockaddr_in));
	state = ST_IDLE;
	isLive = false;
}

Esme::Esme(const Esme& robj){
}

Esme &Esme::operator=(const Esme &robj){
}

Smpp::Uint32 Esme::GetNewSequenceNumber(void){
	Smpp::Uint32 tmp = esmePduSequenceNum;
	esmePduSequenceNum++;
	if(esmePduSequenceNum == Smpp::SequenceNumber::Max){
		esmePduSequenceNum = Smpp::SequenceNumber::Min;
	}
	return tmp;
}

int Esme::OpenConnection(std::string host, uint32_t portno){
	smscSocket = socket(PF_INET, SOCK_STREAM, 0);
	if(smscSocket == -1){
		// Log Error
		return -1;
	}
	// Decide on Some Condition
	this->host = host;
	this->port = portno; 
	std::cout << "host=" << host << ":port=" << portno << std::endl;
	smscInfo.sin_family = PF_INET;
	smscInfo.sin_port = htons(this->port);
	smscInfo.sin_addr.s_addr = inet_addr(this->host.c_str());
	if(connect(smscSocket, (struct sockaddr *)&smscInfo, sizeof(smscInfo))){
		// Log Error
		return -1;
	}
	state = ST_CONNECTED;
#ifdef _MYSQL_UPDATE_
	if(m_sqlobj.mcfn_Open(CG_MyAppConfig.GetMysqlIp().c_str(), CG_MyAppConfig.GetMysqlDbName().c_str(), CG_MyAppConfig.GetMysqlUser().c_str(), CG_MyAppConfig.GetMysqlPassword().c_str()) ){
                APP_LOGGER(CG_MyAppLogger, LOG_DEBUG, "Connected To Database FOR UPDATE ");
        }else{
                APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "NOT ABLE TO CONNECT TO MYSQL FOR UPDATE");
        }
#endif

	return 0;
}

int Esme::CloseConnection(void){
	if (state == ST_DISCONNECTED){
		return 0; // Already disconnected
	}
	if(shutdown(this->smscSocket, SHUT_RDWR)){
		APP_LOGGER(CG_MyAppLogger, LOG_WARNING, "FAIL TO SHUT DOWN ");
	}
	close(smscSocket);
	state = ST_DISCONNECTED;

#ifdef _MYSQL_UPDATE_
	m_sqlobj.mcfn_Close();
#endif
	return 0;
}

enum Esme::STATE Esme::GetEsmeState(void){
	return state;
}

int Esme::RegisterPdu(NetBuffer &tmpNetBuf){
	std::map<uint32_t, NetBuffer>::iterator itr;
	uint32_t seqNo = Smpp::get_sequence_number((Smpp::Uint8 *)tmpNetBuf.GetBuffer());
	itr = pduRegister.find(seqNo);
	if(itr == pduRegister.end()){
		//  Insert to Register
		pduRegister.insert(std::pair<uint32_t,NetBuffer>(seqNo, tmpNetBuf));
		return 0;
	}else{
		// Log Error
		APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "Reg %u Fail", seqNo);
		return -1;
	}
}

int Esme::UnRegisterPdu(NetBuffer &tmpNetBuf){
	std::map<uint32_t, NetBuffer>::iterator itr;
	uint32_t seqNo = Smpp::get_sequence_number((Smpp::Uint8 *)tmpNetBuf.GetBuffer());
	itr = pduRegister.find(seqNo);
	if(itr != pduRegister.end()){
		// Remove from Register
		pduRegister.erase(itr);
		return 0;
	}else{
		// Log Error
		APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "UnReg %u Fail", seqNo);
		return -1;
	}
}

int Esme::Write(NetBuffer &robj){
	uint32_t writtenBytes;
	writtenBytes = write(this->smscSocket, robj.GetBuffer(), robj.GetLength());
	if(writtenBytes == -1){
		// Log Error
		APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "ERROR IN Writing To SMSC PLEASE RECONNECT" );
		state = Esme::ST_ERROR;
		return -1;
	}
	return writtenBytes;
}

int Esme::Read(NetBuffer &robj){
	int32_t readBytes; // as it may contain -1
	uint32_t requireBytes=4; // reading pdu length is enough for parser
	uint8_t readBuffer[MAX_READ_BUFFER_LENGTH];
	do{
		memset(readBuffer, 0x00, sizeof(readBuffer));
		readBytes = read(this->smscSocket, readBuffer, requireBytes); // requireBytes always less than buffer which is 256 currently
		if(readBytes == -1){
			// Log Error
			APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "READ ERROR -1");
			return -1;
		}else if(readBytes == 0){
			// Not Able to Read log ERROR
			APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "READ ERROR 0");
			return 0;
		}else{
			// find what is the length of PDU and read that much 
			robj.Append(readBuffer, readBytes);
			requireBytes -= readBytes;
		}
	}while(requireBytes > 0);
	requireBytes = Smpp::get_command_length((const Smpp::Uint8*)robj.GetBuffer()); // override requiredBytes as per pdu length parameter
//	APP_LOGGER(CG_MyAppLogger, LOG_DEBUG, "Require Bytes= 0x%08X ", requireBytes);
	if( requireBytes > MAX_PDU_PRACTICAL_LENGTH ){
		APP_LOGGER(CG_MyAppLogger, LOG_WARNING, "Impractical PDU Length");
	}
	if( requireBytes < MIN_PDU_LENGTH ){
		// minimum header should be there other wise some sink issue
		APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "PDU LENGTH LESS THAN MINIMUM LENGTH");
		return -1;
	}
	requireBytes -= 4; // already pdu length added to pdu object 
	do{
		memset(readBuffer, 0x00, sizeof(readBuffer));
		if(requireBytes >= sizeof(readBuffer)){ 
			readBytes = read(this->smscSocket, readBuffer, sizeof(readBuffer));
		}else{
			readBytes = read(this->smscSocket, readBuffer, requireBytes);
		}
		if(readBytes == -1){
			// Log Error
			APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "READ ERROR -1");
			return -1;
		}else if(readBytes == 0){
			// Not Able to Read log ERROR
			APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "READ ERROR 0");
			return 0;
		}else{
			// find what is the length of PDU and read that much 
			robj.Append(readBuffer, readBytes);
			requireBytes -= readBytes;
		}
	}while(requireBytes > 0);
	//robj.PrintHexDump();
//	APP_LOGGER(CG_MyAppLogger, LOG_DEBUG, "READ PDU FRAMER SUCESS");
	return robj.GetLength();
}

int Esme::Bind(Smpp::SystemId sysId, Smpp::Password pass, uint8_t bindType){
	Smpp::Uint8* data=NULL;
	NetBuffer nwData;
	switch(bindType){
		case BIND_RDONLY:
			{
				Smpp::BindReceiver pdu;
				pdu.sequence_number(GetNewSequenceNumber());
				pdu.system_id(sysId);
				pdu.password(pass);
				pdu.system_type(CG_MyAppConfig.GetSystemType());
				pdu.interface_version(CG_MyAppConfig.GetInterfaceVersion());
				pdu.addr_ton(CG_MyAppConfig.GetTon());
				pdu.addr_npi(CG_MyAppConfig.GetNpi());
				pdu.address_range(CG_MyAppConfig.GetAddressRange());
				data = (Smpp::Uint8*) pdu.encode();
				nwData.Append(data, pdu.command_length());

			}
			break;
		case BIND_WRONLY:
			{
				Smpp::BindTransmitter pdu;
				pdu.sequence_number(GetNewSequenceNumber());
				pdu.system_id(sysId);
				pdu.password(pass);
				pdu.system_type(CG_MyAppConfig.GetSystemType());
				pdu.interface_version(CG_MyAppConfig.GetInterfaceVersion());
				pdu.addr_ton(CG_MyAppConfig.GetTon());
				pdu.addr_npi(CG_MyAppConfig.GetNpi());
				//pdu.address_range(CG_MyAppConfig.GetAddressRange());  // address range should be null
				data = (Smpp::Uint8*) pdu.encode();
				nwData.Append(data, pdu.command_length());
			}
			break;
		case BIND_RDWR:
			{
				Smpp::BindTransceiver pdu;
				pdu.sequence_number(GetNewSequenceNumber());
				pdu.system_id(sysId);
				pdu.password(pass);
				pdu.system_type(CG_MyAppConfig.GetSystemType());
				pdu.interface_version(CG_MyAppConfig.GetInterfaceVersion());
				pdu.addr_ton(CG_MyAppConfig.GetTon());
				pdu.addr_npi(CG_MyAppConfig.GetNpi());
				pdu.address_range(CG_MyAppConfig.GetAddressRange());
				data = (Smpp::Uint8*) pdu.encode();
				nwData.Append(data, pdu.command_length());
			}
			break;
		default:
			{	// Log Error
				return -1;
			}
			break;
	}
	// Reg Before Write
	RegisterPdu(nwData);
	return Write(nwData);
}

int Esme::UnBind(void){
	Smpp::Unbind pdu(GetNewSequenceNumber());
	NetBuffer nwData;
	nwData.Append(pdu.encode(),pdu.command_length());
	// Reg Before Write
	RegisterPdu(nwData);
	return Write(nwData);
}
//  changing Signature Because iSN require for Performance
int Esme::SendSubmitSm(uint32_t seqNo, const Smpp::Char *srcAddr, const Smpp::Char *destAddr, uint8_t type, const Smpp::Uint8 *sms, Smpp::Uint32 length){
	NetBuffer nwData;
	Smpp::SubmitSm pdu;
	pdu.sequence_number(seqNo);
	Smpp::Ton srcTon(Smpp::Ton::National);
	Smpp::Npi srcNpi(Smpp::Npi::National);
	Smpp::Address sAddr(srcAddr);
	Smpp::SmeAddress srcAddress(srcTon, srcNpi, sAddr);
	Smpp::Ton destTon(Smpp::Ton::National);
	Smpp::Npi destNpi(Smpp::Npi::National);
	Smpp::Address dAddr(destAddr);
	Smpp::SmeAddress destAddress(destTon, destNpi, dAddr);
	pdu.source_addr(srcAddress);
	pdu.destination_addr(destAddress);

	pdu.registered_delivery(0x01);
	// Change data_coding depending on type 
	// TODO high priority
	switch(type){
		case SMS_TYPE_PROMO_UNICODE:
		case SMS_TYPE_TRANS_UNICODE:
		{
			pdu.data_coding(0x08);
			char ucs2_data[512]={0x00};
			iconv_t utf_to_ucs2 = iconv_open("UCS2", "UTF-8");
			size_t ucs2_length = 512;
			size_t utf8_length = length;
			char *ucs2_data_p = ucs2_data;
			char *utf8_data_p = (char *)sms;
			if (iconv(utf_to_ucs2, &utf8_data_p, &utf8_length, &ucs2_data_p, &ucs2_length) == -1){
                		APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "Problem In Decoding From UTF8 to UCS2 PDUID= %d", seqNo);
			}else{
				ucs2_length = 512 - ucs2_length;
				unsigned int is_littel_endian_data = 1;
				char *is_littel_endian = (char *) &is_littel_endian_data;
				if(*is_littel_endian){
					// littleendian so swap ucs2 bytes of ucs2 char
					for(int i=0; i< ucs2_length; i+=2){
						ucs2_data[i] ^= ucs2_data[i+1];
						ucs2_data[i+1] ^= ucs2_data[i];
						ucs2_data[i] ^= ucs2_data[i+1];
					}
				}
				if(ucs2_length < 256){
					pdu.short_message((Smpp::Uint8 *)ucs2_data, (Smpp::Uint8)ucs2_length);
				}else{
					// Use TLV to Send SMS
					APP_LOGGER(CG_MyAppLogger, LOG_INFO, "Length Is More So Insert with tlv PDUID=%d", seqNo);
					//message_payload
					Smpp::Tlv smscontent(Smpp::Tlv::message_payload, (Smpp::Uint16)ucs2_length, (Smpp::Uint8 *)ucs2_data);
					
					pdu.insert_tlv(smscontent);
				}
			}
			HEX_LOGGER(CG_MyAppLogger, LOG_INFO, sms, length);
			HEX_LOGGER(CG_MyAppLogger, LOG_INFO, ucs2_data, ucs2_length);
			iconv_close(utf_to_ucs2);
		}
		break;
		case SMS_TYPE_PROMO_FLASH:
		case SMS_TYPE_TRANS_FLASH:
		{
			pdu.data_coding(0xF0);
			if(length < 256){
				pdu.short_message(sms, (Smpp::Uint8)length);
			}else{
				// Use TLV to Send SMS
					APP_LOGGER(CG_MyAppLogger, LOG_INFO, "Length Is More So Insert with tlv PDUID=%d", seqNo);
					Smpp::Tlv smscontent(Smpp::Tlv::message_payload, (Smpp::Uint16)length, (Smpp::Uint8 *)sms);
					pdu.insert_tlv(smscontent);
			}
			// Add tlv to specify flash
			unsigned char coding = 0x01;
			Smpp::Tlv additionalcodinginfo(Smpp::Tlv::dest_addr_subunit, 1, &coding);
			pdu.insert_tlv(additionalcodinginfo);

		}
		break;
		default:
		{
			if(length < 256){
				pdu.short_message(sms, (Smpp::Uint8)length);
			}else{
				// Use TLV to Send SMS
					APP_LOGGER(CG_MyAppLogger, LOG_INFO, "Length Is More So Insert with tlv PDUID=%d", seqNo);
					Smpp::Tlv smscontent(Smpp::Tlv::message_payload, (Smpp::Uint16)length, (Smpp::Uint8 *)sms);
					pdu.insert_tlv(smscontent);
					
			}
		}
		break;
	}
	nwData.Append(pdu.encode(), pdu.command_length());
	RegisterPdu(nwData);
	int noOfByteWritten = Write(nwData);
	DoDatabaseUpdate(nwData); // Update Database for PDUID
	return noOfByteWritten;
}

int Esme::SendEnquireLink(void){
	Smpp::EnquireLink pdu(GetNewSequenceNumber());
	NetBuffer tmpBuf;
	tmpBuf.Append(pdu.encode(),pdu.command_length());
	RegisterPdu(tmpBuf);
	return Write(tmpBuf);
}

int Esme::StartLinkCheck(void){
	linkThStatus = TH_ST_REQ_RUN;
	if(pthread_create(&pduProcessThId, NULL, &(Esme::LinkCheckThread),this)){
		//fail to create thread
		return -1;
	}	
	return 0;
}

int Esme::StopLinkCheck(void){
	if(linkThStatus == TH_ST_STOP){
		return 0; // Already Closed State
	}
	linkThStatus = TH_ST_REQ_STOP;
	while(linkThStatus == TH_ST_REQ_STOP){
		// Waiting for receive thread to stop
		sleep(DFL_SLEEP_VALUE);
	}
	return 0;
}

thread_status_t Esme::GetEnquireLinkThStatus(void){
	return linkThStatus;
}

void *Esme::LinkCheckThread(void *arg){
	if(arg == NULL){
		return NULL;
	}
	Esme *objAddr = (Esme *) arg;
	APP_LOGGER(CG_MyAppLogger, LOG_DEBUG,"Link Check Thread Started");
	objAddr->linkThStatus = TH_ST_RUNNING;
	while(objAddr->linkThStatus == TH_ST_RUNNING){
		// And Check Live Timer for time out
		// Start Live Timer
		objAddr->SendEnquireLink();
		sleep(DFL_LIVE_SLEEP_VALUE);
	}
	objAddr->linkThStatus = TH_ST_STOP;
	return NULL;
}

int Esme::StartPduProcess(void){
	pduProcessThStatus = TH_ST_REQ_RUN;
	if(pthread_create(&pduProcessThId, NULL, &(Esme::ThOnReceivePdu),this)){
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
	return 0;

}

thread_status_t Esme::GetPduProcessThStatus(void){
	return pduProcessThStatus;
}


void *Esme::ThOnReceivePdu(void *arg){
	if(arg == NULL){
		return NULL;
	}
	Esme *objAddr = (Esme *) arg;
	APP_LOGGER(CG_MyAppLogger, LOG_DEBUG,"Thread Started");
	APP_LOGGER(CG_MyAppLogger, LOG_INFO, "Pdu Processing Thread Started");
	objAddr->pduProcessThStatus = TH_ST_RUNNING;

	Smpp::Uint32 cmdId;
	NetBuffer tmpNetBuf;
	while(objAddr->pduProcessThStatus == TH_ST_RUNNING){
		// check for any data in queue
		if(!objAddr->receiveQueue.empty()){
			// if not empty
			tmpNetBuf = objAddr->receiveQueue.front();
			objAddr->receiveQueue.pop();
			cmdId = Smpp::get_command_id((Smpp::Uint8 *)tmpNetBuf.GetBuffer());
			APP_LOGGER(CG_MyAppLogger, LOG_INFO, "PROCESSING PDU ID:SEQ:STATUS %08X:%08X:%08X", cmdId, Smpp::get_sequence_number((Smpp::Uint8 *)tmpNetBuf.GetBuffer()), Smpp::get_command_status((Smpp::Uint8 *)tmpNetBuf.GetBuffer()));
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
						std::cout << "Bind Receiver Response" << std::endl;
						Smpp::BindReceiverResp resp((Smpp::Uint8 *)tmpNetBuf.GetBuffer());
						std::string smscId = resp.system_id();
						APP_LOGGER(CG_MyAppLogger, LOG_INFO, "SMSCID=%s", smscId.c_str());
						// Remove from Map As Sucessfully Response We Got
						objAddr->UnRegisterPdu(tmpNetBuf);
						// check status and change state
						if(resp.command_status() == 0){
							objAddr->state = ST_BIND;
						}else{
							objAddr->state = ST_BIND_FAIL;
						}
					}
					break;
				case Smpp::CommandId::BindTransmitterResp:
					{
						// change state
						APP_LOGGER(CG_MyAppLogger, LOG_DEBUG,"Bind Transmitter Response");
						Smpp::BindTransmitterResp resp((Smpp::Uint8 *)tmpNetBuf.GetBuffer());
						std::string smscId = resp.system_id();
						APP_LOGGER(CG_MyAppLogger, LOG_INFO, "SMSCID=%s", smscId.c_str());
						objAddr->UnRegisterPdu(tmpNetBuf);
						// check status and change state
						if(resp.command_status() == 0){
							objAddr->state = ST_BIND;
						}else{
							objAddr->state = ST_BIND_FAIL;
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
							objAddr->state = ST_BIND;
						}else{
							objAddr->state = ST_BIND_FAIL;
						}
					}
					break;
				case Smpp::CommandId::UnbindResp:
					{
						// Change State
						APP_LOGGER(CG_MyAppLogger, LOG_DEBUG,"Bind Unbind Response");
						objAddr->UnRegisterPdu(tmpNetBuf);
						// check status and change state
						objAddr->state = ST_UNBIND;
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
							objAddr->sendThStatus = TH_ST_RUNNING; // If Sucess keep running

						}catch(...){
							APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "DO NOT SUBMIT MORE WAIT FOR SOME TIME: ERROR %d", pdu.command_status());
							if(objAddr->sendThStatus == TH_ST_RUNNING){ // IF RUNNING
								objAddr->sendThStatus = TH_ST_REQ_PAUSE; // PAUSE PUSHING OF THREAD
								while( objAddr->sendThStatus == TH_ST_REQ_PAUSE){
									sleep(1);
								}
							}		
							// Resend The Failure PDU TODO // better re-sending logic has to develope
							// Retrive SMS From SMS MAP sendSmsMap
							if(objAddr->sendSmsMap.find(seqNum)!= objAddr->sendSmsMap.end()){
								sms_data_t tmpSms = objAddr->sendSmsMap[seqNum];
								sleep(1); // Put Some Sleep To Give Server Little Time
								objAddr->SendSubmitSm(seqNum, tmpSms.party_a, tmpSms.party_b, tmpSms.type, (Smpp::Uint8 *)tmpSms.msg, strlen((const char *)tmpSms.msg));
							}else{
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
			
		}else{// if empty 
			// put some sleep as data not available
			//usleep(DFL_USLEEP_VALUE);
			APP_LOGGER(CG_MyAppLogger, LOG_INFO, "NO PDU IN QUEUE");
			sleep(1);
		}
	}
	APP_LOGGER(CG_MyAppLogger, LOG_WARNING, "PDU PROCESS THREAD EXITING");
	objAddr->pduProcessThStatus = TH_ST_STOP;
	return NULL;
}

int Esme::StartReader(void){
	rcvThStatus = TH_ST_REQ_RUN;
	if(pthread_create(&rcvThId, NULL, &(Esme::ThSmsReader),this)){
		//fail to create thread
		return -1;
	}
	return 0;
}

int Esme::StopReader(void){
	if(rcvThStatus == TH_ST_STOP){
		return 0;
	}
	if(rcvThStatus == TH_ST_RUNNING){
		rcvThStatus = TH_ST_REQ_STOP;
		while(rcvThStatus == TH_ST_REQ_STOP){
			// Waiting for receive thread to stop
			sleep(DFL_SLEEP_VALUE);
		}
	}
	return 0;
}

thread_status_t Esme::GetRcvThStatus(void){
	return rcvThStatus;
}

void *Esme::ThSmsReader(void *arg){
	// this thread will read pdus come from smsc and put valid pdus in queue
	NetBuffer tempBuffer;
	if(arg == NULL){
		return NULL;
	}
	Esme *objAddr = (Esme *) arg;
	std::cout << "Thread Started" << std::endl;
	APP_LOGGER(CG_MyAppLogger, LOG_INFO, "SMS READER STARTING");
	objAddr->rcvThStatus = TH_ST_RUNNING;
	while(objAddr->rcvThStatus == TH_ST_RUNNING){// Make it Run with respect to a value start/stop should be implemented
		tempBuffer.Erase();
		// Read Only If it is Open only
		if(objAddr->Read(tempBuffer) > 0){
			APP_LOGGER(CG_MyAppLogger, LOG_DEBUG, "Reading SUCESS");
			objAddr->receiveQueue.push(tempBuffer);
			APP_LOGGER(CG_MyAppLogger, LOG_DEBUG, "PUSH TO RECEIVE QUEUE SUCESS");
			//tempBuffer.PrintHexDump();
			//APP_LOGGER(CG_MyAppLogger, LOG_DEBUG, "%.*s", tempBuffer.GetLength(), tempBuffer.GetBuffer());
		}else{
			APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "Reading ERROR");
			usleep(DFL_USLEEP_VALUE);
			break;
		}
		
	}
	APP_LOGGER(CG_MyAppLogger, LOG_WARNING, "SMS READER EXITING");
	objAddr->rcvThStatus = TH_ST_STOP;
	// TODO 
	// change state of ESME so that All Will Exit no reader means some problem happen to socket connection
	return NULL;
}

int Esme::StartSender(void){
	sendThStatus = TH_ST_REQ_RUN;
	if(pthread_create(&sendThId, NULL, &(Esme::ThSmsSender),this)){
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
	return 0;

}

thread_status_t Esme::GetSenderThStatus(void){
	return sendThStatus;
}

void *Esme::ThSmsSender(void *arg){
	sms_data_t tmpSms;
	if(arg == NULL){
		return NULL;
	}
	Esme *objAddr = (Esme *) arg;
	std::cout << "Thread Started" << std::endl;
	APP_LOGGER(CG_MyAppLogger, LOG_INFO, "Sender Thread Started");
	objAddr->sendThStatus = TH_ST_RUNNING;
	while(objAddr->sendThStatus != TH_ST_REQ_STOP){
		// Test Data Availability in Queue
		//if(available)
		if(objAddr->sendThStatus == TH_ST_RUNNING){
			if((!objAddr->sendQueue.empty())&&(objAddr->GetEsmeState() == ST_BIND)){
				// form pdu and Send
				uint32_t pduseq;
				tmpSms = objAddr->sendQueue.front();
				objAddr->sendQueue.pop();
				pduseq = objAddr->GetNewSequenceNumber();
				// Register to smsdata
				objAddr->RegisterSmsData(pduseq, tmpSms);
				objAddr->SendSubmitSm(pduseq, tmpSms.party_a, tmpSms.party_b, tmpSms.type, (Smpp::Uint8 *)tmpSms.msg, strlen((const char *)tmpSms.msg));
				//usleep(50000); // Give time to server to process
			}else{
				// sleep for some second to give chance to other thread run
				sleep(1); // currently only designed to thread functionality
				APP_LOGGER(CG_MyAppLogger, LOG_INFO, "No SMS or ESME Not BINDED");
			}
		}else if(objAddr->sendThStatus == TH_ST_REQ_PAUSE){
			objAddr->sendThStatus = TH_ST_PAUSED; // Pause the pushing sms
			sleep(1); // Sleep for Some Sec
			APP_LOGGER(CG_MyAppLogger, LOG_INFO, "SENDING OF SMS PAUSED");
		}else{
			sleep(1); // Sleep For some time mostly pause condition
			APP_LOGGER(CG_MyAppLogger, LOG_INFO, "SENDING OF SMS PAUSE STATE");
		}

	}
	objAddr->sendThStatus = TH_ST_STOP;
	APP_LOGGER(CG_MyAppLogger, LOG_WARNING, "Sender Thread EXITING");
	return NULL;
}

int Esme::SendSms(sms_data_t sms){
	// Make thread safe pending
	sendQueue.push( sms);
	return 0;
}

int Esme::DoDatabaseUpdate(NetBuffer &tmpNetBuf){
	std::string myUpdateQuery;
	char tmpBuffer[256]={0x00};
	Smpp::Uint32 commandId=0x00000000;
	Smpp::Uint32 sequenceNumber=0x00000000;
	commandId = Smpp::get_command_id((Smpp::Uint8 *)tmpNetBuf.GetBuffer());
	sequenceNumber =  Smpp::get_sequence_number((Smpp::Uint8 *)tmpNetBuf.GetBuffer());
	switch(commandId){
		case Smpp::CommandId::SubmitSm:
		{
			sms_data_t tmpSms;
			memset(&tmpSms, 0x00, sizeof(tmpSms));
			// get value of smadata from smsdata map
			if(sendSmsMap.find(sequenceNumber)!= sendSmsMap.end()){
				memset(tmpBuffer, 0x00, sizeof(tmpBuffer));
				tmpSms = sendSmsMap[sequenceNumber];
				sprintf(tmpBuffer, "UPDATE SMSMT SET iPduId=%u,iStatus=%u Where iSNo=%u" , sequenceNumber , SMS_ST_SUBMITTED, tmpSms.id);
				myUpdateQuery = tmpBuffer;
			}else{
			// Log Error
				std::cerr << "Error in Finding iSNo of a PDU"<< __LINE__ <<std::endl;
			}
		}
		break;
		case Smpp::CommandId::SubmitSmResp:
		{
			sms_data_t tmpSms;
			Smpp::String msgId;
			memset(&tmpSms, 0x00, sizeof(tmpSms));
			// get value of smadata from smsdata map
			if(sendSmsMap.find(sequenceNumber)!= sendSmsMap.end()){
				memset(tmpBuffer, 0x00, sizeof(tmpBuffer));
				tmpSms = sendSmsMap[sequenceNumber];
				Smpp::SubmitSmResp pdu((Smpp::Uint8 *)tmpNetBuf.GetBuffer());
				msgId = pdu.message_id() ;
				memset(tmpBuffer, 0x00, sizeof(tmpBuffer));
				if(pdu.command_status() == 0){
				sprintf(tmpBuffer, "UPDATE SMSMT SET vcMsgId='%s',iStatus=%u,dtSubmitDatetime=NOW() Where iSNo=%u AND iPduId=%u" , msgId.c_str(), SMS_ST_ACKNOWLEDGED, tmpSms.id, sequenceNumber);
				}else{
					sprintf(tmpBuffer, "UPDATE SMSMT SET vcMsgId='error-%u-%u',iStatus=%u,dtSubmitDatetime=NOW() Where iSNo=%u AND iPduId=%u" , pdu.command_status(), pdu.sequence_number(), SMS_ST_FAILED, tmpSms.id, sequenceNumber);
				}
				myUpdateQuery = tmpBuffer;
				
			}else{
				// Log Error
				std::cerr << "Error in Finding iSNo of a PDU "<< sequenceNumber <<std::endl;
			}			
		}
		break;
		case Smpp::CommandId::DeliverSm:
		{
			//TODO 
			// 1. Parse short_message for delivery report
			// 2. Find Error code if fail to deliver
			// 3. If Possible Error MSG from error code
			// message_state - tlv for final state of original msg
			// parse date time from pdu content and update database accordingly
			Smpp::DeliverSm pdu((Smpp::Uint8 *)tmpNetBuf.GetBuffer());
			const Smpp::Tlv *p_msgidTlv = NULL;
			const Smpp::Tlv *p_msgstTlv = NULL;
			p_msgstTlv = pdu.find_tlv(Smpp::Tlv::message_state);
			p_msgidTlv = pdu.find_tlv(Smpp::Tlv::receipted_message_id);
			if(p_msgidTlv){
				uint8_t sms_state=0;
				if(p_msgstTlv){
					memcpy(&sms_state, (const char *)p_msgstTlv->value(), p_msgstTlv->length());
				}else{
					APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "Delivery Report not message_state PDUID=%d", sequenceNumber);
				}
				char l_msgId[64]={0x00};
				memset(tmpBuffer, 0x00, sizeof(tmpBuffer));
				strncpy(l_msgId, (const char *)p_msgidTlv->value(), p_msgidTlv->length());
				if(sms_state){
				sprintf(tmpBuffer, " UPDATE SMSMT SET iStatus=%u,dtDeliveryDatetime=NOW() Where vcMsgId='%s'" , sms_state, l_msgId); // replace 1 with exact msdId Found
				}else{
					sprintf(tmpBuffer, " UPDATE SMSMT SET iStatus=%u,dtDeliveryDatetime=NOW() Where vcMsgId='%s'" , SMS_ST_FAILED, l_msgId);
				}
				myUpdateQuery = tmpBuffer;
			}else{
				APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "Delivery Report not having MsgId PDUID=%d", sequenceNumber);
			}
		}
		break;
		default:
		{
			APP_LOGGER(CG_MyAppLogger, LOG_WARNING, "DataBase Interaction Not Available For %08x ", commandId);
		}
		break;
	}
	// Execute Prepared Query
#ifdef _MYSQL_UPDATE_
	int errNo;
	char errMsg[256]={0x00};
	if(!m_sqlobj.mcfb_isConnectionAlive()){
		m_sqlobj.mcfn_reconnect();
		APP_LOGGER(CG_MyAppLogger, LOG_WARNING, "RECONNECTING MYSQL");
	}
	if(m_sqlobj.mcfn_Execute(myUpdateQuery.c_str(), errNo, errMsg)){
		APP_LOGGER(CG_MyAppLogger, LOG_INFO, "%s", myUpdateQuery.c_str());
	}else{
		APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "%s", myUpdateQuery.c_str());
	}
#endif
	return 0;
}



int Esme::RegisterSmsData(uint32_t pduSeq, sms_data_t tmpSms){
	sendSmsMap.insert(std::pair<uint32_t ,sms_data_t >(pduSeq, tmpSms));
}

int Esme::UnRegisterSmsData(uint32_t pduSeq){
	// TODO BUG: check for valid data then only try to erase
	sendSmsMap.erase(pduSeq);
}

/**
* To Start all necessary steps to start properly esme
*/
int Esme::Start(void){
	//1. Open Connection
	if(OpenConnection(CG_MyAppConfig.GetSmscIp(), CG_MyAppConfig.GetSmscPort()) != 0){
		std::cerr << "socket Connection fail" << std::endl;
		return -1;
	}
	//2. Start Reader As SMSC Response We Have to Read
	StartReader();
	//3. Wait Till Readed Thread Start Properly
	while(GetRcvThStatus() != TH_ST_RUNNING){
		sleep(1);
	}
	//4. Start Pdu Process Thread to Process main core engine
	StartPduProcess();
	//5. Wait tille Pdu Process Thread start Properly
	while(GetPduProcessThStatus() != TH_ST_RUNNING){
		sleep(1);
	}
	//6. Bind The Esme
	Smpp::SystemId sysId=CG_MyAppConfig.GetSystemId();
	Smpp::Password pass=CG_MyAppConfig.GetPassword();
	unsigned int type = CG_MyAppConfig.GetSmscType();
	Bind( sysId, pass, type);
	//7. Wait Till Bind Happen
	while(GetEsmeState() != Esme::ST_BIND){
		sleep(1);
	}
	//8. Check SMSC Bind Status
	if(GetEsmeState() == Esme::ST_BIND_FAIL){
		CloseConnection();
		return -1;
	}
	//9. Start Link check Thread/ Live thread 
	StartLinkCheck();
	//10. Wait till Live Thread Start
	while(GetEnquireLinkThStatus() != TH_ST_RUNNING){
		sleep(1);
	}
	//11. Start Sender Thread if Tx is Type For esme
	if((CG_MyAppConfig.GetSmscType() == Esme::BIND_WRONLY) || (CG_MyAppConfig.GetSmscType()==Esme::BIND_RDWR)){
		StartSender(); //StartSender
		while(GetSenderThStatus() != TH_ST_RUNNING){
			sleep(1);
		}
	}
	return 0;
}

int Esme::Stop(void){
	//1. Stop Link / Live Thread
	StopLinkCheck();
	//2. If Sender Running Then Stop Sender
	if(GetSenderThStatus() == TH_ST_RUNNING){
		StopSender();
	}	
	//3. Unbind From SMSC
	if(state == Esme::ST_BIND){
		UnBind(); // TODO change some state in unbind so that force fully termination can happen
		//4. Wait Till Unbind Happen Properly
		while(GetEsmeState() == Esme::ST_BIND ){ // wait till state change
			APP_LOGGER(CG_MyAppLogger, LOG_DEBUG, "CURRENT STATE is %d ", GetEsmeState());
			sleep(1);
		}
	}
	//5. Close Socket Connection So That We can Safely Close Application
	CloseConnection();
	//6. Stop Reader Thread
	StopReader();
	//7. Stop Pdu Processing
	StopPduProcess();
	state = ST_IDLE;
	return 0;
}
