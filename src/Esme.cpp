#include "Esme.hpp"
// Static Memebers Of Class
pthread_t Esme::sendThId;
thread_status_t Esme::sendThStatus;
pthread_t Esme::rcvThId;
thread_status_t Esme::rcvThStatus;
pthread_t Esme::linkThId;
thread_status_t Esme::linkThStatus;
pthread_t Esme::pduProcessThId;
thread_status_t Esme::pduProcessThStatus;
// vcMsgId Map
std::map<std::string, uint32_t> Esme::m_vcMsgIdMap;
// For Out oF Sync Msgid Pdus
std::map<std::string, uint8_t> Esme::m_vcMsgIdSyncMap;
// Pdu Register
std::map<uint32_t, NetBuffer> Esme::pduRegister;
pthread_mutex_t Esme::pduRegisterLock = PTHREAD_MUTEX_INITIALIZER;
// Sequence Number
pthread_mutex_t Esme::m_seqNumLock = PTHREAD_MUTEX_INITIALIZER ;
Smpp::Uint32 Esme::m_esmePduSequenceNum = Smpp::SequenceNumber::Min;
// 1. Named And Controlled Instance
std::map<std::string, Esme *> Esme::esmeInstanceMap;
uint32_t Esme::maxEsmeInstance = ESME_MAX_NO_OF_INSTANCE;

Esme * Esme::GetEsmeInstance(std::string name, std::string cfgFile){
	if(esmeInstanceMap.size() >= maxEsmeInstance && name.empty()){
		// 1. Set Error And Fill Error Buffer
		//std::cerr << "Either Name is Empty OR Reached Instance Limit" << std::endl;
		return NULL;
	}
	Esme *tmpObj = NULL;
	std::map<std::string, Esme *>::iterator instanceItr;
	instanceItr = esmeInstanceMap.find(name);
	if(instanceItr == esmeInstanceMap.end() ){
		tmpObj = new Esme(name, cfgFile);
		if(tmpObj){
			// 1. Log Here For Creation Of Instance
			//std::cout << "Instance Created with Id " << name << std::endl;
			esmeInstanceMap.insert(std::pair<std::string, Esme *>(name, tmpObj));
		}else{
			// 1. Log Error For Failing Creation Of Instance
			//std::cerr << " Short In Memory" << std::endl;
			;
		}
	}else{
		// 1. Log Returning Of Existing Object
		//std::cout << "Returning Same Object" << std::endl;
		tmpObj = instanceItr->second;
	}
	return tmpObj;
}

void Esme::RemoveEsmeInstance(std::string name){
	if( !esmeInstanceMap.empty()){
		std::map<std::string, Esme *>::iterator instanceItr;
		instanceItr = esmeInstanceMap.find(name);
		if(instanceItr != esmeInstanceMap.end() ){
			// 1. Log Delition Of Instance	
			delete instanceItr->second;
			esmeInstanceMap.erase(instanceItr);
		}else{
			// 1. Log Error According to Design It Should Not Come Here in Any Condition
			;
		}
	}
}

uint32_t Esme::GetNumberOfEsmeInstance(bind_type_t type){
	uint32_t cnt=0;
	if( !esmeInstanceMap.empty()){
		std::map<std::string, Esme *>::iterator instanceItr;
		for(instanceItr = esmeInstanceMap.begin(); instanceItr != esmeInstanceMap.end(); instanceItr++){
			if(instanceItr->second->GetEsmeType() == type){
				cnt++;
			}
		}
	}
	return cnt;
}

std::string Esme::GetEsmeName(void){
	return m_esmeid;
}

int Esme::GetEsmeSocket(void){
	return m_esmeSocket;
}

Esme::~Esme(void){
	CloseConnection();
#ifdef _MYSQL_UPDATE_
	m_sqlobj.mcfn_Close();
#endif

#ifdef _MESSAGE_QUEUE_UPDATE_
	m_updateQueryMsgQueue.Close();
#endif

}


Esme::Esme(std::string name, std::string cfgFile){
	this->m_esmeid = name;
	this->m_cfgFile = cfgFile;
	this->m_host = DFL_SMPP_URL;
	this->m_port = DFL_SMPP_PORT;
	m_esmeSocket = -1;
	bzero(&m_smscInfo, sizeof(struct sockaddr_in));
	m_esmeState = Esme::ST_IDLE;
	m_esmeType = BIND_RDONLY;
	m_esmeTps = ESME_DFL_TPS;
	m_esmeAccountType = ACCOUNT_PROMOTIONAL_PRIMARY;
	//m_sysId =
	//m_sysPass =
	//m_sysType 
	m_IfVersion = Smpp::InterfaceVersion::V34;
	m_addrRange = "";
	m_esmeTon = Smpp::Ton::Unknown;
	m_esmeNpi = Smpp::Npi::Unknown;
	m_registerDeliveryReport = 0x00;
	m_isSendSms = true;
	m_isLive = false;
	m_smppCfg.Load(m_cfgFile.c_str());
	this->m_updateQueryMsgQueueName	= m_smppCfg.GetUpdateQueryQueueName();
	this->m_sizePerMsgInUpdateQueryMsgQueue = m_smppCfg.GetUpdateQueryQueueMsgSize();
	this->m_noOfMsgsInUpdateQueryMsgQueue = m_smppCfg.GetUpdateQueryQueueNoOfMsg();
#ifdef _MYSQL_UPDATE_
	// TODO:
	// 1. Global Config Should Not Be Used Inside Esme Class
	// 2. Make a connfig Object Per Esme Object
	if(m_sqlobj.mcfn_Open(CG_MyAppConfig.GetMysqlIp().c_str(), CG_MyAppConfig.GetMysqlDbName().c_str(), CG_MyAppConfig.GetMysqlUser().c_str(), CG_MyAppConfig.GetMysqlPassword().c_str()) ){
		APP_LOGGER(CG_MyAppLogger, LOG_DEBUG, "Connected To Database FOR UPDATE ");
	}else{
		APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "NOT ABLE TO CONNECT TO MYSQL FOR UPDATE");
	}
#endif

#ifdef _MESSAGE_QUEUE_UPDATE_
	m_updateQueryMsgQueue.Open(m_updateQueryMsgQueueName.c_str(), m_sizePerMsgInUpdateQueryMsgQueue, m_noOfMsgsInUpdateQueryMsgQueue, O_WRONLY);
	perror("mq");
	if( !m_updateQueryMsgQueue.IsOpened()){
		APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "Failed To Open MsgQue %s", m_updateQueryMsgQueueName.c_str());
	}
#endif


}

bool Esme::IsSendSms(void){
	return m_isSendSms;
}

// TODO : 
// 1. Make This Function Thread Safe By Mutex --> Need To TEST
Smpp::Uint32 Esme::GetNewSequenceNumber(void){
	pthread_mutex_lock(&m_seqNumLock); // Lock Here To Access esmePduSequenceNum
	Smpp::Uint32 tmp = m_esmePduSequenceNum;
	m_esmePduSequenceNum++;
	if(m_esmePduSequenceNum == Smpp::SequenceNumber::Max){
		m_esmePduSequenceNum = Smpp::SequenceNumber::Min;
	}
	pthread_mutex_unlock(&m_seqNumLock); // Allow Other Thread To Get Seq Number
	return tmp;
}

int Esme::OpenConnection(void){
	if(m_esmeSocket == -1){
		m_esmeSocket = socket(PF_INET, SOCK_STREAM, 0);
		if(m_esmeSocket == -1){
			// Log Error
			return -1;
		}
	}
	m_smscInfo.sin_family = PF_INET;
	m_smscInfo.sin_port = htons(this->m_port);
	m_smscInfo.sin_addr.s_addr = inet_addr(this->m_host.c_str());
	if(connect(m_esmeSocket, (struct sockaddr *)&m_smscInfo, sizeof(m_smscInfo))){
		// Log Error
		m_esmeState = ST_NOTCONNECTED;
		return -1;
	}else{
		APP_LOGGER(CG_MyAppLogger, LOG_INFO, "Connected To HOST=%s PORT=%u ", m_host.c_str(), m_port );
		m_esmeState = ST_CONNECTED;
	}
	return 0;
}

int Esme::OpenConnection(std::string host, uint32_t portno){
	// Decide on Some Condition
	this->m_host = host;
	this->m_port = portno;
	return OpenConnection();
}

int Esme::CloseConnection(void){
	if (m_esmeState == ST_IDLE || this->m_esmeSocket == -1 || m_esmeState == ST_NOTCONNECTED){
		return 0; // Already disconnected
	}
	if(shutdown(this->m_esmeSocket, SHUT_RDWR)){
		APP_LOGGER(CG_MyAppLogger, LOG_WARNING, "FAIL TO SHUT DOWN ");
	}
	close(m_esmeSocket);
	m_esmeSocket = -1;
	m_esmeState = ST_IDLE ;

	return 0;
}

Esme::esme_state_t Esme::GetEsmeState(void){
	return m_esmeState;
}

int Esme::RegisterPdu(NetBuffer &tmpNetBuf){
	std::map<uint32_t, NetBuffer>::iterator itr;
	uint32_t seqNo = Smpp::get_sequence_number((Smpp::Uint8 *)tmpNetBuf.GetBuffer());
	itr = pduRegister.find(seqNo);
	if(itr == pduRegister.end()){
		pduRegister.insert(std::pair<uint32_t,NetBuffer>(seqNo, tmpNetBuf));
		return 0;
	}else{
		APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "Reg %08X Fail", seqNo);
		return -1;
	}
}

int Esme::UnRegisterPdu(NetBuffer &tmpNetBuf){
	std::map<uint32_t, NetBuffer>::iterator itr;
	uint32_t seqNo = Smpp::get_sequence_number((Smpp::Uint8 *)tmpNetBuf.GetBuffer());
	itr = pduRegister.find(seqNo);
	if(itr != pduRegister.end()){
		pduRegister.erase(itr);
		return 0;
	}else{
		APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "UnReg %08X Fail", seqNo);
		return -1;
	}
}

int Esme::Write(NetBuffer &robj){
	uint32_t writtenBytes;
	writtenBytes = write(this->m_esmeSocket, robj.GetBuffer(), robj.GetLength());
	if(writtenBytes == -1){
		APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "ERROR IN Writing To SMSC PLEASE RECONNECT" );
		m_esmeState = Esme::ST_ERROR;
		return -1;
	}
	return writtenBytes;
}


int Esme::Read(NetBuffer &robj){
	int32_t readBytes;
	uint8_t readBuffer[MAX_READ_BUFFER_LENGTH];
	memset(readBuffer, 0x00, sizeof(readBuffer));
	readBytes = read(this->m_esmeSocket, readBuffer, MAX_READ_BUFFER_LENGTH);
	if(readBytes == -1){
		APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "READ ERROR -1");
		m_esmeState = Esme::ST_ERROR;
		CloseConnection();
		return -1;
	}else if(readBytes == 0){
		APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "READ ERROR 0 ");
		m_esmeState = Esme::ST_ERROR;
		return 0;
	}else{
		m_readNetBuffer.Append(readBuffer, readBytes);
	}

	uint32_t requireBytes = Smpp::get_command_length((const Smpp::Uint8*)m_readNetBuffer.GetBuffer());
	
	while( (requireBytes>0) && (requireBytes <= m_readNetBuffer.GetLength()) ){
		robj.Erase();
		robj.Append(m_readNetBuffer.GetBuffer(), requireBytes);
		m_readNetBuffer.Erase(0, requireBytes);
		receiveQueue.push(robj);
		if(m_readNetBuffer.GetLength()){
		requireBytes = Smpp::get_command_length((const Smpp::Uint8*)m_readNetBuffer.GetBuffer());
		}else{
			requireBytes = 0;
		}
	}
	return 0;
}

int Esme::Bind(void){
	if(m_esmeState == ST_BIND){
		// Already Binded
		return 0;
	}
	Smpp::Uint8* data=NULL;
	NetBuffer nwData;
	switch(m_esmeType){
		case BIND_RDONLY:
			{
				Smpp::BindReceiver pdu;
				pdu.sequence_number(GetNewSequenceNumber());
				pdu.system_id(m_sysId);
				pdu.password(m_sysPass);
				pdu.system_type(m_sysType);
				pdu.interface_version(m_IfVersion);
				pdu.addr_ton(m_esmeTon);
				pdu.addr_npi(m_esmeNpi);
				pdu.address_range(m_addrRange);
				data = (Smpp::Uint8*) pdu.encode();
				nwData.Append(data, pdu.command_length());

			}
			break;
		case BIND_WRONLY:
			{
				Smpp::BindTransmitter pdu;
				pdu.sequence_number(GetNewSequenceNumber());
				pdu.system_id(m_sysId);
				pdu.password(m_sysPass);
				pdu.system_type(m_sysType);
				pdu.interface_version(m_IfVersion);
				pdu.addr_ton(m_esmeTon);
				pdu.addr_npi(m_esmeNpi);
				pdu.address_range(m_addrRange);
				data = (Smpp::Uint8*) pdu.encode();
				nwData.Append(data, pdu.command_length());
			}
			break;
		case BIND_RDWR:
			{
				Smpp::BindTransceiver pdu;
				pdu.sequence_number(GetNewSequenceNumber());
				pdu.system_id(m_sysId);
				pdu.password(m_sysPass);
				pdu.system_type(m_sysType);
				pdu.interface_version(m_IfVersion);
				pdu.addr_ton(m_esmeTon);
				pdu.addr_npi(m_esmeNpi);
				pdu.address_range(m_addrRange);
				data = (Smpp::Uint8*) pdu.encode();
				nwData.Append(data, pdu.command_length());
			}
			break;
		default:
			{       // Log Error
				return -1;
			}
			break;

	}
	RegisterPdu(nwData);
	m_esmeState = ST_BIND_REQ;
	return Write(nwData);
}
// TODO:
// 1. Global Config Should Not Be Used In Esme Member Functions
// 2. Every Required Things Should Be As Datamambers of class

int Esme::Bind(Smpp::SystemId sysId, Smpp::Password pass, Smpp::SystemType sysType, Smpp::InterfaceVersion ifVer, Smpp::Ton ton, Smpp::Npi npi, Smpp::AddressRange range, bind_type_t bindType){
	this->m_sysId = sysId;
	this->m_sysPass = pass;
	this->m_sysType = sysType;
	this->m_IfVersion = ifVer;
	this->m_addrRange = range;
	this->m_esmeTon = ton;
	this->m_esmeNpi = npi;
	this->m_esmeType = bindType;
	return Bind();
}
// TODO:
// 1. Change The State Of SMPP Connection Status
int Esme::UnBind(void){
	Smpp::Unbind pdu(GetNewSequenceNumber());
	NetBuffer nwData;
	nwData.Append(pdu.encode(),pdu.command_length());
	m_esmeState = Esme::ST_UNBIND_REQ;
	// Reg Before Write
	RegisterPdu(nwData);
	return Write(nwData);
}
//  changing Signature Because iSN require for Performance
// TODO:
// 1. Don't Assume Any thing
// 2. SMSC Ton and Npi Should Be Provided to This Function
// 3. Default Esme Ton Npi Should be Available as Esme Class data member
// 4. Wheather Delivery Report Should Available Should be Taken From EMSE class data member
// 5. Should Support gsm 7bit coding and language shift tables of gsm character set
int Esme::SendSubmitSm(uint32_t seqNo, const Smpp::Char *srcAddr, const Smpp::Char *destAddr, uint8_t type, const Smpp::Uint8 *sms, Smpp::Uint32 length){
	if(m_esmeState != ST_BIND){
		return -1;
	}
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
	if(m_esmeState != ST_BIND){
		return -1;
	}
	Smpp::EnquireLink pdu(GetNewSequenceNumber());
	NetBuffer tmpBuf;
	tmpBuf.Append(pdu.encode(),pdu.command_length());
	RegisterPdu(tmpBuf);
	return Write(tmpBuf);
}

int Esme::StartLinkCheck(void){
	linkThStatus = TH_ST_REQ_RUN;
	if(pthread_create(&linkThId, NULL, &(Esme::LinkCheckThread), NULL)){
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
	pthread_join(linkThId, NULL);
	return 0;
}

thread_status_t Esme::GetEnquireLinkThStatus(void){
	return linkThStatus;
}
// TODO:
// 1. Dose This Thread Requred
// 2. Add Some Timer Logic Here 
// 3. Periodic Sending Enquirelink Logic is Resource Constrain
// 4. Design Dynamic Connection And Disconnection
void *Esme::LinkCheckThread(void *arg){
	linkThStatus = TH_ST_RUNNING;
	APP_LOGGER(CG_MyAppLogger, LOG_DEBUG,"Link Check Thread Started");
	std::map<std::string, Esme *>::iterator l_esmeInstanceItr;
	while(linkThStatus == TH_ST_RUNNING){
		for(l_esmeInstanceItr = esmeInstanceMap.begin(); l_esmeInstanceItr != esmeInstanceMap.end(); l_esmeInstanceItr++){
			if(l_esmeInstanceItr->second){
				switch(l_esmeInstanceItr->second->GetEsmeState()){
					case Esme::ST_ERROR: //8
						{
							APP_LOGGER(CG_MyAppLogger, LOG_WARNING,"Tring To Re Start Connection %s", l_esmeInstanceItr->second->GetEsmeName().c_str());
							l_esmeInstanceItr->second->Stop();
						}
						break;
					case Esme::ST_IDLE: // 0
						{
							if(l_esmeInstanceItr->second->Start() == -1){
								APP_LOGGER(CG_MyAppLogger, LOG_ERROR,"Failed To Re Start Connection %s", l_esmeInstanceItr->second->GetEsmeName().c_str());
							}
						}
						break;
					case Esme::ST_BIND_REQ:
						{
							APP_LOGGER(CG_MyAppLogger, LOG_WARNING," Bind Requested %s", l_esmeInstanceItr->second->GetEsmeName().c_str());
						}
					case Esme::ST_BIND: // 3
						{
							l_esmeInstanceItr->second->SendEnquireLink();
							sleep(DFL_LIVE_SLEEP_VALUE);
						}
						break;
					case Esme::ST_BIND_FAIL: // 4
						{
							APP_LOGGER(CG_MyAppLogger, LOG_ERROR," Trying To Re-Bind %s", l_esmeInstanceItr->second->GetEsmeName().c_str());
							l_esmeInstanceItr->second->Bind();
						}
						break;
					case Esme::ST_CONNECTED: // 1
						{
							l_esmeInstanceItr->second->Bind();
						}
						break;
					case Esme::ST_NOTCONNECTED:
						{
							if(l_esmeInstanceItr->second->OpenConnection() == -1){
								sleep(30); // Give Some Time To Server
							}
						}
					default:
						APP_LOGGER(CG_MyAppLogger, LOG_ERROR," SHOULD NOT BE IN STATE %s: %d", l_esmeInstanceItr->second->GetEsmeName().c_str(), l_esmeInstanceItr->second->GetEsmeState());
						sleep(30);
					break;
				}
			}
		}
		usleep(DFL_USLEEP_VALUE);
	}
	linkThStatus = TH_ST_STOP;
	APP_LOGGER(CG_MyAppLogger, LOG_WARNING, "Link Check Thread Exiting...");
	return NULL;
}

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

int Esme::StartReader(void){
	rcvThStatus = TH_ST_REQ_RUN;
	if(pthread_create(&rcvThId, NULL, &(Esme::ThSmsReader), NULL)){
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
	pthread_join(rcvThId, NULL);
	return 0;
}

thread_status_t Esme::GetRcvThStatus(void){
	return rcvThStatus;
}
// TODO:
// 1. Need to Replace This complete Thread With MultiSocket Read Concept
// 2. Should Able to Watch Multiple sockets For Read Operation
// 3. Should Independent of SMS Connection Type

void *Esme::ThSmsReader(void *arg){
	// 1. Log Starting Of Sms Multi Reader Thread
	//std::cout << "Starting " << __FUNCTION__ << std::endl;
	APP_LOGGER(CG_MyAppLogger, LOG_DEBUG, "SmsReader Started ...");
	rcvThStatus = TH_ST_RUNNING;
	NetBuffer tempSmppPdu;
	while( rcvThStatus == TH_ST_RUNNING ){
		if(esmeInstanceMap.empty()){
			// 1. Log No Live Instance
			sleep(NO_ESME_INSTANCE_SLEEP);
		}else{
			fd_set l_readfds;
			FD_ZERO(&l_readfds);
			int l_max_fd=0;
			std::map<std::string, Esme *>::iterator l_esmeInstanceItr;
			for(l_esmeInstanceItr = esmeInstanceMap.begin(); l_esmeInstanceItr != esmeInstanceMap.end() ; l_esmeInstanceItr++){
				int l_socket = l_esmeInstanceItr->second->GetEsmeSocket();
				// TODO: check Socket Connected OR Not Also
				Esme::esme_state_t l_esmeState = l_esmeInstanceItr->second->GetEsmeState();
				if(l_socket != -1 && (ST_IDLE != l_esmeState) && (ST_ERROR != l_esmeState) && (ST_NOTCONNECTED != l_esmeState)){
					//std::cout << "Adding Socket " << l_socket << std::endl;
					FD_SET( l_socket, &l_readfds);
					if(l_max_fd < l_socket ){
						l_max_fd = l_socket;
					}
				}
			} // Adding Socket Descriptor Ends
			struct timeval l_timeout={ .tv_sec= MULTI_READ_TIME_OUT_SEC , .tv_usec=0};
			int l_ready = -1;
			l_max_fd++;
			l_ready = select(l_max_fd, &l_readfds, NULL, NULL, &l_timeout);
			if(l_ready == -1){
				//perror("select");
				APP_LOGGER(CG_MyAppLogger, LOG_ERROR,"SELECT..." );
			}else if(l_ready == 0){
				APP_LOGGER(CG_MyAppLogger, LOG_DEBUG,"Data Not Available For Reading...TIME OUT");
			}else{
				for(l_esmeInstanceItr = esmeInstanceMap.begin(); l_esmeInstanceItr != esmeInstanceMap.end(); l_esmeInstanceItr++){
					tempSmppPdu.Erase();
					if (l_esmeInstanceItr->second->GetEsmeSocket() > 0){
						if(FD_ISSET(l_esmeInstanceItr->second->GetEsmeSocket(), &l_readfds)){
							if(l_esmeInstanceItr->second->Read(tempSmppPdu) != 0){
								// 1. Log Error
								APP_LOGGER(CG_MyAppLogger, LOG_DEBUG,"Error In Reading PDU ");
								;
							}
						}
					}
				}
			}
		}
	}
	rcvThStatus = TH_ST_STOP;
	APP_LOGGER(CG_MyAppLogger, LOG_DEBUG, "Reader Thread Exiting... ");
	return NULL;
}


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

int Esme::SendSms(sms_data_t sms){
	// Make thread safe pending
	sendQueue.push( sms);
	return 0;
}

uint32_t Esme::NoOfSmsInQueue(void){
	return sendQueue.size();
}

// TODO:
// 1. Don't use std::cout and std::cerr
// 2. Remove Database Dependency
// 3. Develope Logic To Map vcMsgId to iSNo irrespective of SMS Handles
// 4. Implement DBname, DBTablename From SMPP Class or SMS Object
int Esme::DoDatabaseUpdate(NetBuffer &tmpNetBuf){
	std::string myUpdateQuery;
	char tmpBuffer[256]={0x00};
	Smpp::Uint32 commandId=0x00000000;
	Smpp::Uint32 sequenceNumber=0x00000000;
	time_t rawtime;
	struct tm * curtimeinfo;
	time(&rawtime);
	curtimeinfo = localtime(&rawtime);
	strftime(tmpBuffer, sizeof(tmpBuffer),"%F %T", curtimeinfo);
	std::string currentTime = tmpBuffer;
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
						std::map<std::string, uint8_t>::iterator it;
						it = m_vcMsgIdSyncMap.find(msgId);
						if( it != m_vcMsgIdSyncMap.end()){
							
							APP_LOGGER(CG_MyAppLogger, LOG_INFO, "OUTOFSYNC %s:%u, SIZE %u", msgId.c_str(), tmpSms.id, m_vcMsgIdSyncMap.size());
						sprintf(tmpBuffer, "UPDATE SMSMT SET vcMsgId='%s',iStatus=%u,dtSubmitDatetime='%s' Where iSNo=%u " , msgId.c_str(), it->second, currentTime.c_str(), tmpSms.id);
							m_vcMsgIdSyncMap.erase(it);
						}else{
						sprintf(tmpBuffer, "UPDATE SMSMT SET vcMsgId='%s',iStatus=%u,dtSubmitDatetime='%s' Where iSNo=%u " , msgId.c_str(), SMS_ST_ACKNOWLEDGED, currentTime.c_str(), tmpSms.id);
						// Add vcMsgId To vcMsgIdMap m_vcMsgIdMap
						APP_LOGGER(CG_MyAppLogger, LOG_INFO, "Adding %s:%u", msgId.c_str(), tmpSms.id);
						m_vcMsgIdMap.insert(std::pair<std::string, uint32_t>(msgId, tmpSms.id));
						}
					}else{
						sprintf(tmpBuffer, "UPDATE SMSMT SET vcMsgId='error-%u-%u',iStatus=%u,dtSubmitDatetime='%s' Where iSNo=%u" , pdu.command_status(), pdu.sequence_number(), SMS_ST_FAILED, currentTime.c_str(), tmpSms.id);
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
				Smpp::Uint8 esm = pdu.esm_class();
				if (esm & 0x04){ // Delivery Recipt 
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
						// Try To Find iSNo From This MsgId
						std::map<std::string, uint32_t>::iterator l_msgIdItr;
						APP_LOGGER(CG_MyAppLogger, LOG_INFO, "Searching %s", l_msgId);
						l_msgIdItr = m_vcMsgIdMap.find(l_msgId);
						if(l_msgIdItr != m_vcMsgIdMap.end()){
							if(sms_state){
								sprintf(tmpBuffer, " UPDATE SMSMT SET iStatus=%u,dtDeliveryDatetime='%s' Where iSNo='%d'" , sms_state, currentTime.c_str(), l_msgIdItr->second);
							}else{
								sprintf(tmpBuffer, " UPDATE SMSMT SET iStatus=%u,dtDeliveryDatetime='%s' Where iSNo='%d'" , SMS_ST_FAILED, currentTime.c_str(), l_msgIdItr->second);
							}
							// Work Done So Erase It
							m_vcMsgIdMap.erase(l_msgIdItr);
						}else{
								m_vcMsgIdSyncMap.insert(std::pair<std::string, uint8_t>(l_msgId, sms_state));
							if(sms_state){
								sprintf(tmpBuffer, " UPDATE SMSMT SET iStatus=%u,dtDeliveryDatetime='%s' Where vcMsgId='%s' ORDER BY iSNo DESC LIMIT 1" , sms_state, currentTime.c_str(), l_msgId);
							}else{
								sprintf(tmpBuffer, " UPDATE SMSMT SET iStatus=%u,dtDeliveryDatetime='%s' Where vcMsgId='%s' ORDER BY iSNo DESC LIMIT 1" , SMS_ST_FAILED, currentTime.c_str(), l_msgId);
							}
						}
						myUpdateQuery = tmpBuffer;
					}else{
						APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "Delivery Report not having MsgId PDUID=%d", sequenceNumber);
					}
				}else{
					APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "Receiving SMS Not Designed PDUID=%08X", sequenceNumber);
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
	if (!myUpdateQuery.empty()){
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

#ifdef _MESSAGE_QUEUE_UPDATE_
		if(0 == m_updateQueryMsgQueue.Write(myUpdateQuery.c_str(),myUpdateQuery.size())){
			APP_LOGGER(CG_MyAppLogger, LOG_INFO, "ENQUEUE: %s" , myUpdateQuery.c_str() );
		}else{
			APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "ENQUEUE: %s" , myUpdateQuery.c_str() );
		}
#endif
	}
	return 0;
}


int Esme::RegisterSmsData(uint32_t pduSeq, sms_data_t tmpSms){
	std::map<uint32_t, sms_data_t>::iterator itr;
	itr = sendSmsMap.find(pduSeq);
	if(itr == sendSmsMap.end()){
		sendSmsMap.insert(std::pair<uint32_t ,sms_data_t >(pduSeq, tmpSms));
		return 0;
	}else{
		APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "Reg SMS DATA %08X Fail", pduSeq);
		return -1;
	}
}

int Esme::UnRegisterSmsData(uint32_t pduSeq){
	std::map<uint32_t, sms_data_t>::iterator itr;
	itr = sendSmsMap.find(pduSeq);
	if(itr != Esme::sendSmsMap.end()){
		sendSmsMap.erase(itr);
		return 0;
	}else{
		APP_LOGGER(CG_MyAppLogger, LOG_ERROR, "UnReg SMS DATA %08X Fail", pduSeq);
		return -1;
	}
}

/**
 * To Start all necessary steps to start properly esme
 */
// TODO:
// 1. Don't use global Config 
// 2. Use/Put Necessary Infos Regarding SMPP Connection to Datamember Of Class
// 3. Please Remove Global App config
int Esme::Start(void){
	// 1. Load Data From Config File
	this->m_host = m_smppCfg.GetSmscIp();
	this->m_port = m_smppCfg.GetSmscPort();
	this->m_esmeType = m_smppCfg.GetSmscType();
	this->m_esmeTps = m_smppCfg.GetSmscTps();

	this->m_sysId = m_smppCfg.GetSystemId();
	this->m_sysPass = m_smppCfg.GetPassword();
	this->m_sysType = m_smppCfg.GetSystemType();
	this->m_IfVersion = m_smppCfg.GetInterfaceVersion();
	this->m_esmeTon = m_smppCfg.GetTon();
	this->m_esmeNpi = m_smppCfg.GetNpi();
	this->m_addrRange = m_smppCfg.GetAddressRange();
	this->m_isSendSms = true;
	//this->m_registerDeliveryReport = ;
	//2. Open Connection
	if(OpenConnection() != 0){
		std::cerr << "socket Connection fail" << std::endl;
		return -1;
	}
	//3. Bind The Esme
	Bind();
	//4. Wait Till Bind Happen
	while(GetEsmeState() == Esme::ST_BIND_REQ){
		sleep(1);
	}
	//5. Check SMSC Bind Status
	if(GetEsmeState() == Esme::ST_BIND_FAIL){
		CloseConnection();
		return -1;
	}
	return 0;
}

int Esme::Stop(void){
	//3. Unbind From SMSC
	m_isSendSms = false;
	if(m_esmeState == Esme::ST_BIND){
		UnBind(); // TODO change some state in unbind so that force fully termination can happen
		//4. Wait Till Unbind Happen Properly
		while(GetEsmeState() == Esme::ST_UNBIND_REQ ){ // wait till state change
			APP_LOGGER(CG_MyAppLogger, LOG_DEBUG, "CURRENT STATE is %d ", GetEsmeState());
			sleep(1);
		}
	}
	//5. Close Socket Connection So That We can Safely Close Application
	CloseConnection();
	m_esmeState = ST_IDLE;
	return 0;
}
