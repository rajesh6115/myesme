#include "Esme.hpp"
Smpp::Uint32 Esme::esmePduSequenceNum = Smpp::SequenceNumber::Min;
Esme::~Esme(void){
}

Esme::Esme(std::string host, uint32_t port){
	this->host = host;
	this->port = port;
	smscSocket = -1;
	bzero(&smscInfo, sizeof(struct sockaddr_in));
	state = IDLE;
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
	smscInfo.sin_family = PF_INET;
	smscInfo.sin_port = htons(this->port);
	smscInfo.sin_addr.s_addr = inet_addr(this->host.c_str());
	if(connect(smscSocket, (struct sockaddr *)&smscInfo, sizeof(smscInfo))){
		// Log Error
		return -1;
	}
	return 0;
}

int Esme::CloseConnection(void){
	if(shutdown(this->smscSocket, SHUT_RDWR)){
		//Log Error
		// Even this fail we can close socket descriptor
	}
	close(smscSocket);
	return 0;
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
		return -1;
	}
	return writtenBytes;
}

int Esme::Read(NetBuffer &robj){
	uint32_t readBytes,requireBytes=16;
	uint8_t readBuffer[256];
	memset(readBuffer, 0x00, sizeof(readBuffer));
	readBytes = read(this->smscSocket, readBuffer, 16); // read the header of smpp first
	if(readBytes >= 4){ 
	// read require bytes and re-initise requireBytes
		requireBytes = Smpp::get_command_length(readBuffer);
		std::cout << "Require Bytes= " << requireBytes << std::endl;
	}
	while(requireBytes>0){
		if(readBytes == -1){
			// Log Error
			return -1;
		}else if(readBytes == 0){
			// Not Able to Read log ERROR
			return 0;
		}else{
			// find what is the length of PDU and read that much 
			robj.Append(readBuffer, readBytes);
			requireBytes -= readBytes;
			if(requireBytes >0){
				memset(readBuffer, 0x00, sizeof(readBuffer));
				readBytes = read(this->smscSocket, readBuffer, sizeof(readBuffer));
			}
		}
	}
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
		//	pdu.system_type(CG_MyAppConfig.getSystemType());
		//	pdu.interface_version(CG_MyAppConfig.getSmppVersion());
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
		//	pdu.system_type(CG_MyAppConfig.getSystemType());
		//	pdu.interface_version(CG_MyAppConfig.getSmppVersion());
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
		//	pdu.system_type(CG_MyAppConfig.getSystemType());
		//	pdu.interface_version(CG_MyAppConfig.getSmppVersion());
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

int Esme::SendSubmitSm(const Smpp::Char *srcAddr, const Smpp::Char *destAddr, uint8_t type, const Smpp::Uint8 *sms, Smpp::Uint32 length){
	NetBuffer nwData;
	Smpp::SubmitSm pdu;
	pdu.sequence_number(GetNewSequenceNumber());
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
	if(length < 256){
		pdu.short_message(sms, (Smpp::Uint8)length);
	}else{
		// Use TLV to Send SMS
		std::cout << "Length Is More So Insert with tlv" << std::endl;
	}
	pdu.registered_delivery(0x03);
	nwData.Append(pdu.encode(), pdu.command_length());
	RegisterPdu(nwData);
	return Write(nwData);
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
	linkThStatus = TH_ST_REQ_STOP;
	while(linkThStatus == TH_ST_REQ_STOP){
		// Waiting for receive thread to stop
		sleep(DFL_SLEEP_VALUE);
	}
	return 0;
}

void *Esme::LinkCheckThread(void *arg){
	if(arg == NULL){
		return NULL;
	}
	Esme *objAddr = (Esme *) arg;
	std::cout << "Link Check Thread Started" << std::endl;
	objAddr->linkThStatus = TH_ST_RUNNING;
	while(objAddr->linkThStatus == TH_ST_RUNNING){
		// Send Enquire Link in Regular Interval
		// And Check Live Timer for time out
		// Start Live Timer
		objAddr->SendEnquireLink();
		sleep(DFL_SLEEP_VALUE);
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
	std::cout << "Thread Started" << std::endl;
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
					}
					break;
				case Smpp::CommandId::BindTransmitterResp:
					{
						// change state
						std::cout << "Bind Transmitter Response" << std::endl;
						Smpp::BindTransmitterResp resp((Smpp::Uint8 *)tmpNetBuf.GetBuffer());
						std::string smscId = resp.system_id();
						APP_LOGGER(CG_MyAppLogger, LOG_INFO, "SMSCID=%s", smscId.c_str());
						objAddr->UnRegisterPdu(tmpNetBuf);
					}
					break;
				case Smpp::CommandId::BindTransceiverResp:
					{ // change state
						std::cout << "Bind TransReceiver Response" << std::endl;
						Smpp::BindTransceiverResp resp((Smpp::Uint8 *)tmpNetBuf.GetBuffer());
						std::string smscId = resp.system_id();
						APP_LOGGER(CG_MyAppLogger, LOG_INFO, "SMSCID=%s", smscId.c_str());
						objAddr->UnRegisterPdu(tmpNetBuf);
					}
					break;
				case Smpp::CommandId::UnbindResp:
					{
						// Change State
						std::cout << "Bind Unbind Response" << std::endl;
						objAddr->UnRegisterPdu(tmpNetBuf);
					}
					break;
				case Smpp::CommandId::EnquireLinkResp:
					{
						std::cout << "Enquire Link Response" << std::endl;
						objAddr->UnRegisterPdu(tmpNetBuf);
						// Reset Enquire Link Timer
					}
					break;
				case Smpp::CommandId::GenericNack:
					{
						std::cout << "Generic NACK" << std::endl;
						// Received Negative Ack for PDU
						// Log Error and Decide to Re-Transmit
					}
					break;
				case Smpp::CommandId::AlertNotification:
					{
						std::cout << "Alert Notification " << std::endl;
						// Only Valid in BindReceiver
						// No Resp Associated with this pdu
					}
					break;
				case Smpp::CommandId::SubmitSm:
					{
						std::cout << "Submit Sm " << std::endl;
						// To Submit Single SMS
						// Esme Should not Receive This
					}
					break;
				case Smpp::CommandId::SubmitSmResp:
					{
						std::cout << "Submit Sm Resp " << std::endl;
						// UnRegister the pdu
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
				default:
					{
						// Log for Un Handle CMDID
						std::cerr << "Un Handelled PDU ID = " << cmdId << std::endl;
					}
					break;
			}
		}else{// if empty 
			// put some sleep as data not available
			usleep(DFL_USLEEP_VALUE);
		}
	}
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
	rcvThStatus = TH_ST_REQ_STOP;
	while(rcvThStatus == TH_ST_REQ_STOP){
		// Waiting for receive thread to stop
		sleep(DFL_SLEEP_VALUE);
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
	objAddr->rcvThStatus = TH_ST_RUNNING;
	while(objAddr->rcvThStatus == TH_ST_RUNNING){// Make it Run with respect to a value start/stop should be implemented
		tempBuffer.Erase();
		if(objAddr->Read(tempBuffer) > 0){
			objAddr->receiveQueue.push(tempBuffer);
			tempBuffer.PrintHexDump();
			std::cout << std::endl;
		}else{
			usleep(DFL_USLEEP_VALUE);
		}
	}
	objAddr->rcvThStatus = TH_ST_STOP;
	return NULL;
}
/*
void *Esme::ThSmsWriter(void *arg){
}*/
