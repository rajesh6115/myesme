#ifdef _MSG_QUE_MSGID_MAP_
// TODO: Once Message Que Class Become Complete
// Design Error Part
// This May Block Applicatio So Be sure Before Writing to Queue

#include <Esme.hpp>

int Esme::WriteToMsgIdPersistenceQue(std::string vcMsgId, uint32_t iSNo){
	if(!m_msgIdQueue.IsOpened()){
		// Not Opened
		return -1;
	}
	vcMsgId += ":";
	vcMsgId += std::to_string(iSNo);
	m_msgIdQueue.Write(vcMsgId.c_str(), vcMsgId.length());
	return 0;
}

// TODO : Decide Size of buffer
// And It Should be Greater Than Writing Buffer
int Esme::ReadFromMsgIdPersistenceQue(void){
	if(!m_msgIdQueue.IsOpened()){
                // Not Opened
                return -1;
        }
	char buffer[512]={0x00};
	char *vcMsgId = NULL;
	char *iSNo = NULL;
	std::map<std::string, std::string>::iterator msgIdMapIterator;
	while(m_msgIdQueue.NoOfMessagesInQueue()){
		memset(buffer, 0x00, sizeof(buffer));
		m_msgIdQueue.Read(buffer, sizeof(buffer));
		vcMsgId = strtok(buffer, ":");
		iSNo = strtok(NULL, ":");
		if(vcMsgId && iSNo){
			msgIdMapIterator = m_msgIdMap.find(vcMsgId);
			if(msgIdMapIterator == m_msgIdMap.end()){
				// Insert To Map Key Is Not There
				m_msgIdMap.insert(std::pair<std::string, std::string>(vcMsgId, iSNo));
			}else{
				// Log Repeat Of VcMsgId Error
				;
			}
		}
	}
	return 0;
}

#endif
