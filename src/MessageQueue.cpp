#include <MessageQueue.hpp>

SysMessageQueue::SysMessageQueue(){
	mesi_mqState = MQ_ST_UNKNOWN;
	meui_sizePerMsg = 0;
	meui_noOfMsg = 0;
	meui_curNoOfMsg = 0;
	me_mqid = (mqd_t) -1 ;
	memset(&me_mqattr, 0x00, sizeof(me_mqattr));
}

SysMessageQueue::~SysMessageQueue(){
	if ( (mesi_mqState == MQ_ST_OPENED) || (me_mqid != (mqd_t) -1)  ){
                Close();
        }
}

bool SysMessageQueue::IsOpened(void){
	if(mesi_mqState == MQ_ST_OPENED){
		return true;
	}
	return false;
}

size_t	SysMessageQueue::Read(char *buffer, size_t size){
	// size should be more than size of message attribute 
	// TODO ERROR MESSAGE FOR SIZE OF BUFFER 
	if ( (mesi_mqState != MQ_ST_OPENED) || (me_mqid == (mqd_t) -1)  ){
                return MQ_ERR_GENERAL;
        }
	return mq_receive(me_mqid, buffer, size, NULL);
}

size_t	SysMessageQueue::Write(const char *buffer, size_t size){
	if ( (mesi_mqState != MQ_ST_OPENED) || (me_mqid == (mqd_t) -1)  ){
		return MQ_ERR_GENERAL;
	}
	return mq_send(me_mqid, buffer, size, 0); 

}

uint32_t SysMessageQueue::NoOfMessagesInQueue(void){
	if ( (mesi_mqState != MQ_ST_OPENED) || (me_mqid == (mqd_t) -1)  ){
                return MQ_ERR_GENERAL;
        }
	if(-1 == mq_getattr(me_mqid, &me_mqattr)){
		return MQ_ERR_GENERAL;
	}
	meui_curNoOfMsg = me_mqattr.mq_curmsgs;
	return meui_curNoOfMsg;
}

uint32_t SysMessageQueue::MaximumNumberOfMessage(void){
	if ( (mesi_mqState != MQ_ST_OPENED) || (me_mqid == (mqd_t) -1)  ){
                return MQ_ERR_GENERAL;
        }
        if(-1 == mq_getattr(me_mqid, &me_mqattr)){
                return MQ_ERR_GENERAL;
        }
	meui_noOfMsg = me_mqattr.mq_maxmsg;
	return meui_noOfMsg;
}

uint32_t SysMessageQueue::MaximumSizePermessage(void){
	if ( (mesi_mqState != MQ_ST_OPENED) || (me_mqid == (mqd_t) -1)  ){
                return MQ_ERR_GENERAL;
        }
        if(-1 == mq_getattr(me_mqid, &me_mqattr)){
                return MQ_ERR_GENERAL;
        }
	meui_sizePerMsg = me_mqattr.mq_msgsize;
	return meui_sizePerMsg;
}

uint32_t SysMessageQueue::Open(const char *name, uint32_t size_per_msg, uint32_t no_of_msg, int operation){
	if ( !IsQueueExist(name)){
		// Message Queue Not Exist So Try To Create It
		if( MQ_ERR_SUCESS != Create( name, size_per_msg, no_of_msg) ){
			return MQ_ERR_GENERAL;
		}
	}
	if(mesi_mqState == MQ_ST_EXIST || mesi_mqState == MQ_ST_CLOSED){
		me_mqid = mq_open( name, operation);
	}
	if(me_mqid != (mqd_t) -1 ){
		mesi_mqState = MQ_ST_OPENED;
		return MQ_ERR_SUCESS;
	}
	return MQ_ERR_GENERAL;
}

uint32_t SysMessageQueue::Close(void){
	if (me_mqid != (mqd_t) -1){
		mq_close(me_mqid);
	}
	return MQ_ERR_SUCESS;
}

bool SysMessageQueue::IsQueueExist(const char *name){
	mqd_t tempmqid;
	tempmqid = mq_open( name, O_RDONLY);
	if(tempmqid == (mqd_t) -1 ){
		//decide the error
		if(errno == ENOENT){
			return false;
		}
		mesi_mqState = MQ_ST_EXIST;
		return true;
	}else{
		// close the que immediately
		// we can skip error checking here as tempmqid is always valid
		mq_close(tempmqid);
		mesi_mqState = MQ_ST_EXIST;
	}

	return true;	
}

uint32_t SysMessageQueue::Delete(const char *name){
	int ret = mq_unlink(name);
	if (ret = 0){
		return MQ_ERR_SUCESS;
	}else{
	//TODO Set Proper Error Message
		return MQ_ERR_GENERAL;
	}
}

uint32_t SysMessageQueue::Create(const char *name, uint32_t size_per_msg, uint32_t no_of_msg){
	mqd_t mqid;
        struct mq_attr attr;
	attr.mq_flags = 0;
	attr.mq_maxmsg = no_of_msg;
	attr.mq_msgsize = size_per_msg;
	attr.mq_curmsgs = 0;
	mqid = mq_open(name, O_RDWR | O_CREAT, (S_IRWXU | S_IRWXG | S_IRWXO), &attr);
        if (mqid == (mqd_t) -1){
		// Set Appropiate Error Message
		return MQ_ERR_GENERAL;
	}else{
		// change state of message queue
		mq_close(mqid);
		mesi_mqState = MQ_ST_EXIST;
		return MQ_ERR_SUCESS;
	}
}

