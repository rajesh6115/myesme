#include <iostream>
#include <MessageQueue.hpp>

int main(int argc, char *argv[]){
	SysMessageQueue smqObj;
	smqObj.Open(argv[1], 256, 20, O_WRONLY);
	if(smqObj.IsOpened()){
		std::cout << "No Of Available Message is " << smqObj.NoOfMessagesInQueue() << std::endl;
		char msg[256]={0x00};
		std::cin.getline(msg, 255);
		smqObj.Write(msg, strlen(msg));
		std::cout << "Message Written To Queue is " << msg << std::endl;
		smqObj.Close();
	}else{
		perror("mq:");
	}	
	return 0;
}
