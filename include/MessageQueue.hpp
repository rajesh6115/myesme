#ifndef __SYS_MESSAGE_QUEUE_H__
#define __SYS_MESSAGE_QUEUE_H__

#include <cstdint>
#include <iostream>
#include <cstring>
#ifdef __cplusplus
extern "C" {
#endif

#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <mqueue.h>
#ifdef __cplusplus
}
#endif

class SysMessageQueue{
	typedef enum MQ_STATE{
		MQ_ST_UNKNOWN,
		MQ_ST_EXIST,
		MQ_ST_OPENED,
		MQ_ST_CLOSED,
		MQ_ST_DELETED
	}mq_state_t;
	typedef enum MQ_ERR_CODE{
		MQ_ERR_SUCESS,
		MQ_ERR_GENERAL,
	}mq_err_t;
	private:
	mq_state_t mesi_mqState;
	uint32_t meui_sizePerMsg;
	uint32_t meui_noOfMsg;
	uint32_t meui_curNoOfMsg;
	std::string mes_name;
	mqd_t me_mqid;
	struct mq_attr me_mqattr;
	uint32_t Create(const char *name, uint32_t size_per_msg, uint32_t no_of_msg);
	uint32_t Delete(const char *name);
	bool IsQueueExist(const char *name);
	public:
	SysMessageQueue();
	~SysMessageQueue();
	bool IsOpened(void);
	size_t Read(char *buffer, size_t size);
	size_t Write(const char *buffer, size_t size);
	uint32_t NoOfMessagesInQueue(void);
	uint32_t MaximumNumberOfMessage(void);
	uint32_t MaximumSizePermessage(void);
	uint32_t Open(const char *name, uint32_t size_per_msg, uint32_t no_of_msg, int operation);
	uint32_t Close(void);
};
#endif
