#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>

#include "misc.h"

#include "module/manager.h"

ModuleManager_t *pmmManager;

ModuleManager_t *ModuleManager_getInstance()
{
	static ModuleManager_t mmManager;

	return &mmManager;
}

void ModuleManager_init(XWF_Call_f fnAppCall)
{
	pmmManager =ModuleManager_getInstance();

	pmmManager->lstpmodModules =List_new();
	pmmManager->lstpobjWildcards =List_new();
	pmmManager->dictpobjObjects =Dictionary_new();
	pmmManager->psrvServices.uiVersion =1;
	pmmManager->psrvServices.fnRegisterObj =ModuleManager_registerObject;
	pmmManager->psrvServices.fnCall =ModuleManager_call;
	pmmManager->fnAppCall =fnAppCall;
}

void ModuleManager_destroy()
{
	Dictionary_delete(pmmManager->dictpobjObjects);
}

int ModuleManager_loadDynamicModule(const char *pszPath)
{
	XWF_Module_t *pmodNew =malloc(sizeof(XWF_Module_t));
	const char *pszDLError;
	XWF_Init_f fnInit;

	pmodNew->enModtype =XWF_Dynamic;
	pmodNew->hdlLib =dlopen(pszPath, RTLD_LAZY | RTLD_GLOBAL);

	if((pszDLError =dlerror()) != NULL) goto dlerror;

	fnInit =dlsym(pmodNew->hdlLib, "XWF_init");

	if((pszDLError =dlerror()) != NULL) goto dlerror;

	if(ModuleManager_initialiseModule(pmodNew, fnInit) != 0) goto error;

	return 0;

dlerror:
	dbg("Failed to load dynamic module (%s)\n", pszDLError);
error:
	dbg("Unloading dynamic module (%s) due to error.\n", pszPath);
	free(pmodNew);
	return -1;
}

XWF_Object_Handle_t *ModuleManager_createObject(const char *pszType)
{
	XWF_ObjectParams_t obpParams;
	XWF_Object_t *pobjHandler;

	obpParams.pszObjType =pszType;
	obpParams.psrvServices =&pmmManager->psrvServices;

	if((pobjHandler =Dictionary_get_pointer
	                 (pmmManager->dictpobjObjects, pszType)) != 0)
	{
		XWF_Object_Handle_t *pobjhCreated =malloc(sizeof(XWF_Object_Handle_t));
		obpParams.pobjhHandle =pobjhCreated;
		pobjhCreated->pxwoObject =pobjHandler;
		pobjhCreated->pobjObject =pobjHandler->fnCreate(&obpParams);

		if(pobjhCreated->pobjObject) return pobjhCreated;
		else free(pobjhCreated);
	}

	return 0; /* no module can create such an object */
}

int ModuleManager_destroyObject(XWF_Object_Handle_t *pobjhToDelete)
{
	if(!pobjhToDelete) return -1;
	else
	{
		pobjhToDelete->pxwoObject->fnDestroy(pobjhToDelete->pobjObject);
		free(pobjhToDelete);
		return 0;
	}
}

int ModuleManager_initialiseModule(XWF_Module_t *pmodNew, XWF_Init_f fnInit)
{
	int iRet =fnInit(pmodNew, &pmmManager->psrvServices);

	if(!iRet)
	{
		dbg("Module <%s> loaded successfully.\n", pmodNew->pszName);
		return 0;
	}
	else if(iRet == 1)
	{
		dbg("Module <%s> loaded with errors.\n", pmodNew->pszName);
		return 0;
	}
	else
	{
		dbg("Module failed to load.\n");
		return -1;
	}
}

int ModuleManager_registerObject(const XWF_Object_t *pobjRegistered)
{
	const char *pszLang ="Unknown";

	switch(pobjRegistered->enLang)
	{
	case XWF_Lang_C:
		pszLang ="C-language";
		break;

	case XWF_Lang_CXX:
		pszLang ="C++-language";
		break;

	case XWF_Lang_Script:
		pszLang ="Script-language";
		break;
	}

	if(strcmp(pobjRegistered->pszType, "*") == 0)
	{
		dbg("Adding new %s wildcard object provided by <%s>\n", pszLang,
		    pobjRegistered->pmodProvider->pszName);
		List_add(pmmManager->lstpobjWildcards, (void*)pobjRegistered);
		return 0;
	}
	else if(Dictionary_get(pmmManager->dictpobjObjects,
	                       pobjRegistered->pszType))
	{
		dbg("Object already exists: <%s>\n", pobjRegistered->pszType);
		return -1;
	}
	else
	{
		dbg("Adding new %s object <%s> provided by <%s>\n", pszLang,
		    pobjRegistered->pszType, pobjRegistered->pmodProvider->pszName);
		Dictionary_set_pointer(pmmManager->dictpobjObjects,
		                       pobjRegistered->pszType, pobjRegistered);
		return 0;
	}
}

void *ModuleManager_call(const XWF_Object_Handle_t *pobjhSource,
                         const char *pszService,
                         const void *pvParams)
{
	if(strncmp(pszService, "APP", 3) == 0)
	{
		return pmmManager->fnAppCall(pobjhSource, pszService + 4, pvParams);
	}

	return 0;
}
