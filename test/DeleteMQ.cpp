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
        mqd_t mqid;
	mqid = mq_unlink(argv[1]);
        if (mqid == (mqd_t) -1)
                perror("mq_unlink");
	else
        	printf(" QUEUE destroid successfully");

}
