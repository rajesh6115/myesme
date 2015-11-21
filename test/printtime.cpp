#include <iostream>
#include <ctime>

int main(int argc, char *argv[]){
	// size_t strftime (char* ptr, size_t maxsize, const char* format, const struct tm* timeptr );
	std::time_t curtime;
	struct tm * timeinfo;
	char buffer[64];
	std::time(&curtime);
	timeinfo = std::localtime(&curtime);
	
	strftime (buffer, sizeof(buffer),"%F %T",timeinfo);
	std::cout << "NOW TIME IS " << buffer << std::endl;	
	return 0;
}
