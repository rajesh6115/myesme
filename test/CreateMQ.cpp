/*
*******************************************************************************
*                 * /etc/sysctl.conf                                          *
*                 * kernel.msgmni=16384 ( MINIMUM NUMBER OF MSGs PER QUEUE )  *
*                 * kernel.msgmax=65536 ( MAXIMUM NUMBER OF MSGs PER QUEUE )  *
*                 * kernel.msgmnb=10485760 (MAXIMUN MESSAGE QUEUE SIZE SYSTEM *
*                 *                        -WIDE)                             *
*                 *                                                           *
*                 * /etc/security/limits.conf                                 *
*                 * (*              -       msgqueue        unlimited )       *
*                 *                                                           *
*                 *                                                           *
*******************************************************************************
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>

int main(int argc,char *argv[])
{
        if(argc < 2)
        {
                printf("Usage : <exe> <Q-Name>\n");
                exit(0);
        }
        mqd_t mqid;
        struct mq_attr attr;
	// int getrlimit(int resource, struct rlimit *rlim);
	struct rlimit rlim;
	memset(&rlim, 0x00, sizeof(rlim));
	getrlimit(RLIMIT_MSGQUEUE , &rlim);
	printf("Limit is %lu : %lu \n", rlim.rlim_cur, rlim.rlim_max);
	attr.mq_flags = 0;
	// Maximum Number Of Message
        attr.mq_maxmsg = 65000;
	// Maximum Size Of Each Message
        attr.mq_msgsize= 256;
	attr.mq_curmsgs = 0;  
	//	S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH
        mqid = mq_open(argv[1], O_RDWR | O_CREAT, (S_IRWXU | S_IRWXG | S_IRWXO), &attr);
        if (mqid == (mqd_t) -1){
                perror("mq_open");
		printf("%s Message Queue Creation Failed\n", argv[1]);
	}
	else{
        	printf(" QUEUE created successfully %s ", argv[1]);
		mq_close(mqid);
	}

}
