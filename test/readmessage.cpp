#include <iostream>
#include <MessageQueue.hpp>

int main(int argc, char *argv[]){
	SysMessageQueue smqObj;
	smqObj.Open(argv[1], 256, 20, O_RDONLY);
	if(smqObj.IsOpened()){
		std::cout << "No Of Available Message is " << smqObj.NoOfMessagesInQueue() << std::endl;
		char msg[512]={0x00};
		smqObj.Read(msg, 511);
		perror("mq:");
		std::cout << "1st Message In Queue is " << msg << std::endl;
		smqObj.Close();
	}
	return 0;
}
