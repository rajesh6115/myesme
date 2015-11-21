#include "appconfig.hpp"

appconfig_p appconfig_init(void){
	appconfig_p myconfig = (appconfig_p ) malloc(sizeof(appconfig_t));
	if(myconfig){
		memset(myconfig, 0x00, sizeof(appconfig_t));
	}
	return myconfig;
}

int appconfig_open(appconfig_p conf, const char *configFile){
	if((conf == NULL)||(configFile == NULL) ){
		return -1;
	}
	strcpy(conf->file, configFile);
	conf->doc = xmlParseFile(conf->file);
        if(conf->doc == NULL){
                fprintf(stderr, "Problem in Opening and Reading File %s \n", conf->file);
                return -1;
	}else{
		conf->cur = xmlDocGetRootElement(conf->doc); 
		if (conf->cur == NULL){
			fprintf(stderr, "Empty Config File\n");
			xmlFree(conf->doc);
			conf->doc = NULL;
			return -1;
		}else{
			conf->root = conf->cur;
		}
	}
	return 0;
}

int appconfig_close(appconfig_p conf){
	if(conf){
		if(conf->doc){
			xmlFree(conf->doc);
			conf->doc = NULL;
		}
		free(conf);
	}
}

int appconfig_module(appconfig_p conf, const char *moduleName, xmlNodePtr *result){
	xmlNodePtr curNode;
	if( conf->root == NULL ){
		fprintf(stderr, "Configuration File Not Opened Properly \n");
		return -1;
	}
	if( moduleName == NULL){
		fprintf(stderr, "Please Provide Module Name To Search\n");
		return -1;
	}
	if( result == NULL){
		fprintf(stderr, "It will Not Return Any Result Node/Element\n");
		return -1;
	}
	curNode = conf->root->xmlChildrenNode;
        while(curNode){
		if(xmlStrcmp(curNode->name, (const xmlChar *) moduleName)== 0){
			if(result != NULL){
				*(result) = curNode;
			}
			break;
		}
                curNode = curNode->next;
        }
	if(curNode == NULL){
		fprintf(stderr, " Module %s Not Found\n", moduleName);
		return -1;
	}
	return 0;
}

int appconfig_element(xmlNodePtr module, const char *elementName, xmlNodePtr *result){
	xmlNodePtr curNode;
	curNode = module->xmlChildrenNode;
	while(curNode){
		if(xmlStrcmp(curNode->name, (const xmlChar *) elementName) == 0){
			if(result != NULL){
				*(result) = curNode;
			}
			break;
		}
		curNode = curNode->next;
	}
	if(curNode == NULL){
		fprintf(stderr, " Element %s Not found\n", elementName);
		return -1;
	}	
	return 0;
}

int appconfig_getvalue(appconfig_p conf, const char *moduleName, const char *elementName, char *result){
	xmlNodePtr reqNode = NULL;
	xmlNodePtr reqEle = NULL;
	xmlChar *reqString = NULL;
	if((conf == NULL) || (moduleName == NULL) || (elementName == NULL)){
		return -1;
	}

	if(appconfig_module(conf, moduleName, &reqNode) == 0){
		if(appconfig_element(reqNode, elementName, &reqEle) ==0){
                	reqString = xmlNodeListGetString(conf->doc, reqEle->xmlChildrenNode, 1);
			if(reqString != NULL){
				strcpy(result,(const char *) reqString);
				return 0;
			}
		}
	}
	return -1;
}
