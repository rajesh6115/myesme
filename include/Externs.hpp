#ifndef _EXTERNS_HPP_
#define _EXTERNS_HPP_
#include "EsmeConfig.hpp"
#include <clogger.hpp>
#include <map>
// Custom types
typedef enum sms_status{
        SMS_ST_SCHEDULED=0,
	SMS_ST_ENROUTE=1,
        SMS_ST_DELIVERED=2,
	SMS_ST_EXPIRED=3,
	SMS_ST_DELETED=4,
	SMS_ST_UNDELIVERABLE=5,
	SMS_ST_ACCEPTED=6,
	SMS_ST_UNKNOWN=7,
	SMS_ST_REJECTED=8,
	SMS_ST_SKIPPED=9,
        SMS_ST_PICKED=51,
        SMS_ST_SUBMITTED=52,
        SMS_ST_ACKNOWLEDGED=53,
        SMS_ST_FAILED=99,
}sms_status_t;
typedef enum smstype{
	SMS_TYPE_PROMO_FIXED=0,
	SMS_TYPE_PROMO_CUSTOM=1,
	SMS_TYPE_TRANS_FIXED=3,
	SMS_TYPE_TRANS_CUSTOM=4,
	SMS_TYPE_PROMO_FLASH=5,
	SMS_TYPE_TRANS_FLASH=6,
	SMS_TYPE_PROMO_UNICODE=7,
	SMS_TYPE_TRANS_UNICODE=8,
	SMS_TYPE_USSD=9,
}sms_type_t;
typedef struct smsdata{
        uint32_t id;
        uint32_t status;
        char party_a[16];
        char party_b[16];
	uint8_t msg[512];
        uint8_t type;
}sms_data_t;
typedef enum thread_status{
        TH_ST_IDLE=0,
        TH_ST_REQ_RUN=1,
        TH_ST_RUNNING=2,
	TH_ST_REQ_PAUSE=3,
	TH_ST_PAUSED=4,
        TH_ST_REQ_STOP=5,
        TH_ST_STOP=6,
}thread_status_t;

typedef std::map<uint32_t, pthread_t> campaign_thread_map_t;
// Global Function Declaration
void * CampaignThread(void *arg);
uint32_t IsActiveCampaign(void);
// Global Variables
extern EsmeConfig CG_MyAppConfig;
extern logger_p CG_MyAppLogger;
#endif
