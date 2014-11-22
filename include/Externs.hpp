#ifndef _EXTERNS_HPP_
#define _EXTERNS_HPP_
#include "EsmeConfig.hpp"
#include <clogger.hpp>

// Custom types
typedef enum sms_status{
        SMS_ST_NOT_PICKED=0,
        SMS_ST_PICKED=1,
        SMS_ST_SUBMITTED=2,
        SMS_ST_ACKNOWLEDGED=3,
        SMS_ST_DELIVERED=4,
        SMS_ST_FAIL=99,
}sms_status_t;
typedef enum smstype{
	SMS_TYPE_PROMO_FIXED=0,
	SMS_TYPE_PROMO_CUSTOM=1,
	SMS_TYPE_TRANS_FIXED=3,
	SMS_TYPE_TRANS_CUSTOM=4,
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
        TH_ST_RUNNING=3,
        TH_ST_REQ_STOP=4,
        TH_ST_STOP=5,
}thread_status_t;
// Global Function Declaration
void * CampaignThread(void *arg);
uint32_t IsActiveCampaign(void);
// Global Variables
extern EsmeConfig CG_MyAppConfig;
extern logger_p CG_MyAppLogger;
#endif
