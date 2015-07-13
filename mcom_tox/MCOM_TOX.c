#include <stdlib.h>

#include "Module/Module.h"
#include "AOM/IMComm.h"
#include "misc.h"

#include "MCOMMTOX.h"

const XWF_Services_t *pSvcs;

int XWF_exit()
{
	return 0;
}

int XWF_init(XWF_Module_t *pmodSelf, const XWF_Services_t *psrvServices)
{
	XWF_Class_t *classIM =malloc(sizeof(XWF_Class_t));

	pmodSelf->pszName ="MComm for Tox IM";
	pmodSelf->fnExit =XWF_exit;

	classIM->pszType ="MESSENGER";
	classIM->pszSubtype ="TOX";
	classIM->enLang =XWF_Lang_C;
	classIM->pmodProvider =pmodSelf;
	classIM->fnCreate =MCommTox_create;
	classIM->fnDestroy =MCommTox_destroy;

	pSvcs =psrvServices;

	psrvServices->fnRegisterClass(classIM);

	return 0;
}