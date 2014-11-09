#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <string.h>
#include <cstring>
#include <string.h>

using namespace std;


int main(int argc,char *argv[])
{
        if(argc < 2)
        {
                printf("Usage : <exe> <Q-Name>\n");
                exit(0);
        }
        mqd_t mqd1;
        struct mq_attr attr;
	attr.mq_flags = 0;
        attr.mq_maxmsg = 30000;
        attr.mq_msgsize= 1024;
	attr.mq_curmsgs = 0;  
        mqd1 = mq_open(argv[1], O_RDWR | O_CREAT, 0666, &attr);
        if (mqd1 == (mqd_t) -1)
                perror("mq_open");

        printf(" QUEUE created successfully");

}
