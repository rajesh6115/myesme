#include <Esme.hpp>
// TODO:
// 1. Search For id of esme in esme list
// 2. If Available return the Instance of same
// 3. If Not Available Create One Instance
// 4. Insert In Instance Map
// 5. Return Instance
static Esme * Esme::GetEsmeInstance(std::string host, uint32_t port, std::string id){
	std::map<std::string, Esme*>::iterator esmeItr;
	esmeItr = instanceMap.find(id);
	if(esmeItr == instanceMap.end()){
		return esmeItr->second;
	}else{
		Esme *tempObjp = new Esme(host, port);
		
	}
		
	
}
