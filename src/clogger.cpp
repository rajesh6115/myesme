
/******************************************************************************
** General Logger in C it can enerate File and Log message passes to it      **
** It is having different logging level to catagorise loging                 **
******************************************************************************/

#include "clogger.hpp"
// removed from .h file as it is not exposed to usr
int logger_open(logger_p log); 
int logger_close(logger_p log);

logger_p logger_init(void)
{
	logger_p log = (logger_p) malloc(sizeof(logger_t));
	if(log == NULL)
	{
		return NULL;
	}
	else
	{
		memset(log,0x00,sizeof(logger_t));
		strcpy(log->path ,DFL_PATH );
		strcpy(log->name ,DFL_NAME );
		strcpy(log->tag ,DFL_TAG );
		log->level = DFL_LEVEL ;
		log->interval = DFL_INTERVAL;
		log->create_time = 0;
		log->fp = NULL;
		log->leveltag[0] = "~EMR~";
		log->leveltag[1] = "~ALT~";
		log->leveltag[2] = "~CRT~";
		log->leveltag[3] = "~ERR~";
		log->leveltag[4] = "~WRN~";
		log->leveltag[5] = "~NTC~";
		log->leveltag[6] = "~INF~";
		log->leveltag[7] = "~DBG~";
		log->leveltag[8] = "~DFL~";
		log->leveltag[9] = "~CNT~";
		log->leveltag[10] = "~CNS~";
#ifdef ENABLE_THREADS
		pthread_mutex_init(&log->file_lock, NULL);
#endif
		return log;
	}
}

int logger_cleanup(logger_p log)
{
	if(log)
	{
		logger_close(log);
#ifdef ENABLE_THREADS
		pthread_mutex_destroy(&log->file_lock);
#endif
		free(log);
		return 0;
	}
	else
	{
		return -1;
	}
}

#ifdef ENABLE_THREADS
int logger_lock(logger_p log){
	if(log){
		pthread_mutex_lock(&log->file_lock);
		return 0;
	}else{
		return -1;
	}
}

int logger_unlock(logger_p log){
	if(log){
		pthread_mutex_unlock(&log->file_lock);
		return 0;
	}else{
		return -1;
	}

}

#endif
int logger_apptag(logger_p log, const char *apptag){
	if(log){
		memset(log->tag, 0x00, sizeof(log->tag)); // clear tag buffer
		strcpy(log->tag, apptag);
		return 0;
	}else{
		return -1;
	}
}

int logger_filepath(logger_p log, const char *filepath){
	if(log){
		memset(log->path, 0x00, sizeof(log->path)); // clear tag buffer
		strcpy(log->path, filepath);
		return 0;
	}else{
		return -1;
	}
}
int logger_filename(logger_p log, const char *filename){
	if(log){
		memset(log->name, 0x00, sizeof(log->name)); // clear tag buffer
		strcpy(log->name, filename);
		return 0;
	}else{
		return -1;
	}
}

int logger_level(logger_p log, unsigned int level){
	if(log){
		log->level = level;
		return 0;
	}else{
		return -1;
	}
}

int logger_interval(logger_p log, unsigned int interval){
	if(log){
		log->interval = interval;
		return 0;
	}else{
		return -1;
	}
}


int logger_open(logger_p log)
{
	struct tm cur_time;
	char cmd[512]={0x00};
	char fname[PATH_MAX_SIZE + NAME_MAX_SIZE + 20]={0x00}; //additional 20 for time stamp
	if(log)
	{
		sprintf(cmd, "mkdir -p %s", log->path);
		if(system(cmd)){
			fprintf(stderr, "creating of %s directory fail", log->path);
		}
		time(&log->create_time);
		localtime_r(&log->create_time , &cur_time);
		if(log->interval < 3600)
		{
			sprintf(fname,"%s/%s_%04d%02d%02d%02d%02d.log" ,log->path ,log->name ,cur_time.tm_year+1900,cur_time.tm_mon+1 ,cur_time.tm_mday ,cur_time.tm_hour ,cur_time.tm_min );
		}
		else
		{
			sprintf(fname,"%s/%s_%04d%02d%02d%02d.log" ,log->path ,log->name ,cur_time.tm_year+1900,cur_time.tm_mon+1 ,cur_time.tm_mday ,cur_time.tm_hour );
		}
		// if log->fp is open then close it first
		if(log->fp != NULL)
		{
			logger_close(log);
		}
		log->fp = fopen(fname ,"a+");
		if(log->fp == NULL)
		{
			return -1;
		}
		return 0;
	}
	else
	{
		return -1;
	}
}


int logger_close(logger_p log)
{
	if(log)
	{
		if(log->fp != NULL)
		{
			fflush(log->fp);
			fclose(log->fp);
			log->fp == NULL;
		}
		return 0;
	}
	else
	{
		return -1;
	}
}

int logger_log_header(logger_p log, unsigned int log_level, const char *srcname, unsigned int line_no)
{
	int ret_val = 0;
	struct tm cur_time;
	time_t temp_time;
	char logtag[64]={0x00};
	if(log == NULL)
	{
		return -1;
	}

	if(log_level > log->level)
	{
		log->is_log = 0;
		return 0;
	}
	log->is_log = 1;
	if(log->fp == NULL){
		logger_open(log);  // If Logger is running for first time
	}
	/////check for new file generation condition///////////
	temp_time = time(NULL);
	if((temp_time - log->create_time) > log->interval)
	{
		logger_open(log);
	}
	/////prepare tag//////////
	//time(&temp_time);
	temp_time = time(NULL);
	localtime_r(&temp_time , &cur_time);
	sprintf(logtag ,"[%s|%s|%d|%02d:%02d:%02d|%s]" ,log->tag ,srcname ,line_no ,cur_time.tm_hour ,cur_time.tm_min ,cur_time.tm_sec ,log->leveltag[log_level]);
	/*  to be designed
	if(log_level == DISP_SCREEN)
	{
		fprintf(stdout ,"%s",logtag);
		fflush(stdout);
	}
	*/
	ret_val = fprintf(log->fp ,"%s" ,logtag);
	fflush(log->fp);
	return ret_val;
}

int logger_log_message(logger_p log,const char *format ,...)
{
	int ret_val = 0;
	va_list args;
	if(log == NULL)
        {
                return -1;
        }
	if(log->is_log == 0)
	{
		return 0;
	}
	va_start(args,format);
	ret_val = vfprintf(log->fp ,format ,args);
	fprintf(log->fp,"\n");
	fflush(log->fp);
	va_end(args);
	return ret_val;
}
