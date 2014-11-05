#ifndef _APPCONFIG_H_
#define _APPCONFIG_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/parser.h>
#ifdef __cplusplus
extern "C" {
#endif
#define MAX_NAME 256

typedef struct appconfig{
	char file[MAX_NAME];
	xmlDocPtr doc;
	xmlNodePtr root;
	xmlNodePtr cur; 
}appconfig_t;

typedef appconfig_t * appconfig_p;

appconfig_p appconfig_init(void);
int appconfig_open(appconfig_p conf, const char *configFile);
int appconfig_close(appconfig_p conf);
int appconfig_getvalue(appconfig_p conf, const char *moduleName, const char *elementName, char *result);
#ifdef __cplusplus
}
#endif
#endif
