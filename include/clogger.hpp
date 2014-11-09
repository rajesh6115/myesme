
/******************************************************************************
** General Logger in C it can enerate File and Log message passes to it      **
** It is having different logging level to catagorise loging                 **
******************************************************************************/

#ifndef _LOGGER_C_H_
#define _LOGGER_C_H_
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#ifdef __cplusplus
extern "C" {
#endif
#include "config.h"
#ifdef ENABLE_THREADS
#include <pthread.h>
#endif

#define PATH_MAX_SIZE 256
#define NAME_MAX_SIZE 64
#define TAG_MAX_SIZE 16

#define LOG_EMERG 0 
#define LOG_ALERT 1
#define LOG_CRITICAL 2
#define LOG_ERROR 3
#define LOG_WARNING 4
#define LOG_NOTICE 5
#define LOG_INFO 6
#define LOG_DEBUG 7
#define LOG_DEFAULT 8
#define LOG_CONT 9
#define LOG_CONSOL 10
#define APP_LOGGER(OBJECT, LOG_LEVEL, LOGDATA, ...) {\
	logger_lock(OBJECT);\
	logger_log_header(OBJECT, LOG_LEVEL, __FILE__, __LINE__);\
	logger_log_message(OBJECT, LOGDATA, __VA_ARGS__);\
	logger_unlock(OBJECT);\
	}

#define DFL_NAME "united"
#define DFL_PATH "."
#define DFL_TAG "LOG"
#define DFL_INTERVAL 300
#define DFL_LEVEL 8

typedef struct logger{
	FILE *fp;  /* File pointer to hold file*/
	unsigned int level; /* Level Of Logging*/
	unsigned int is_log; /* Should Log or Not*/
	time_t create_time; /* File Creation Time*/
	time_t interval; /* New File Creation Interval*/
	char path[PATH_MAX_SIZE]; /* Log file path*/
	char name[NAME_MAX_SIZE]; /*Log file name*/
	char tag[TAG_MAX_SIZE]; /*APPLICATION TAG*/
	const char *leveltag[11]; /*Log Level Tags Strings*/
#ifdef ENABLE_THREADS
	pthread_mutex_t file_lock;
#endif
}logger_t;
typedef logger_t * logger_p;

logger_p logger_init(void);
int logger_cleanup(logger_p);
int logger_apptag(logger_p, const char *apptag );
int logger_filepath(logger_p, const char *filepath);
int logger_filename(logger_p, const char *filename);
int logger_level(logger_p, unsigned int level);
int logger_interval(logger_p, unsigned int interval);
#ifdef ENABLE_THREADS
int logger_lock(logger_p);
int logger_unlock(logger_p);
#endif
int logger_set(logger_p, unsigned int option, const char *);
int logger_log_header(logger_p log, unsigned int log_level, const char *srcname, unsigned int line_no);
int logger_log_message(logger_p ,const char *format ,...);
#ifdef __cplusplus
}
#endif
#endif


