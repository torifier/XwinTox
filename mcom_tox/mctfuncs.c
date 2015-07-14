#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "AOM/IMComm.h"
#include "MCOMMTOX.h"
#include "xwintox.h"
#include "callback.h"
#include "mctfuncs.h"
#include "hexstring.h"
#include "misc.h"

/* Signal handlers */

void savedata(XWF_hObj_t *hobjSelf)
{
	PREP
	unsigned int wLen =tox_get_savedata_size(TOXINST);
	unsigned char *bData =calloc(wLen+1, sizeof(unsigned char));
	FILE *hfSave;

	DLOCK
	tox_get_savedata(TOXINST, bData);
	DUNLOCK

	unlink(DPRIVATE->szToxSave);
	hfSave =fopen(DPRIVATE->szToxSave, "wb");

	if(hfSave == NULL)
	{
		dbg("Failed to open savefile %s\n", DPRIVATE->szToxSave);
		return;
	}

	if(fwrite(bData, wLen, 1, hfSave) != 1)
	{
		dbg("Failed to save data to savefile %s\n", DPRIVATE->szToxSave);
		fclose(hfSave);
		return;
	}

	dbg("Saved Tox data of length %d to %s\n", wLen, DPRIVATE->szToxSave);

	fclose(hfSave);
}

void frsendmessage(XWF_hObj_t *hobjSelf, unsigned int wNum, const char *pszMsg)
{
	PREP
	dbg("Send message to %d\n", wNum);
	DLOCK
	tox_friend_send_message(TOXINST, wNum, TOX_MESSAGE_TYPE_NORMAL,
	                        (uint8_t*) pszMsg, strlen(pszMsg), 0);
	DUNLOCK

}

void frdelete(XWF_hObj_t *hobjSelf, unsigned int wNum)
{
	PREP
	dbg("Delete contact %d\n", wNum);
	DLOCK
	tox_friend_delete(TOXINST, wNum, 0);
	DUNLOCK
}

void MCT_recv(int iType, PBMessage_t* msg, void* custom)
{
	switch(iType)
	{
	case clSaveData:
		savedata(custom);
		break;

	case frSendMsg:
		frsendmessage(custom, msg->I1, msg->S1);
		break;

	case frDelete:
		frdelete(custom, msg->I1);
		savedata(custom);
		break;

	default:
		dbg("Unhandled request: %d\n", iType);
	}
}

/* Object functions */

int MCT_Connect(XWF_Object_Handle_t *hobjSelf)
{
	PREP
	TOX_ERR_NEW txnerrTnerr;
	TOX_ERR_BOOTSTRAP txberrTberr;
	struct Tox_Options *ptxoTopts =tox_options_new(0);
	PBMessage_t *msgContacts =PB_New_Message();
	uint8_t mypubkey[TOX_ADDRESS_SIZE];

	LOCK(pimcSelf)

	ptxoTopts->ipv6_enabled =false;

	if(PRIVATE(pimcSelf)->datToxSave.wLength)
	{
		ptxoTopts->savedata_type =TOX_SAVEDATA_TYPE_TOX_SAVE;
		ptxoTopts->savedata_data =(const uint8_t*)
		                          PRIVATE(pimcSelf)->datToxSave.pbData;
		ptxoTopts->savedata_length =PRIVATE(pimcSelf)->datToxSave.wLength;
		dbg("Savedata length: %d\n", ptxoTopts->savedata_length);
	}

	TOXINST =tox_new(ptxoTopts, &txnerrTnerr);

	if(txnerrTnerr != TOX_ERR_NEW_OK)
	{
		const char *pszErrcode;

		if(txnerrTnerr == TOX_ERR_NEW_LOAD_BAD_FORMAT) pszErrcode =
			    "Bad save format";
		else pszErrcode ="Unknown";

		dbg("Failed to create new Tox: %s\n", pszErrcode);
		return -1;
	}

	tox_self_set_name(TOXINST, (uint8_t*)pimcSelf->pszName,
	                  strlen(pimcSelf->pszName), NULL);
	tox_self_set_status_message(TOXINST, (uint8_t*)pimcSelf->pszStatus,
	                            strlen((char*)(pimcSelf->pszStatus)),
	                            NULL);

	tox_self_set_status(TOXINST, TOX_USER_STATUS_NONE);

	tox_bootstrap(TOXINST,
	              Dictionary_get(pimcSelf->dictConfig, "Tox.BootstrapIP"),
	              strtol(
	                  Dictionary_get(pimcSelf->dictConfig, "Tox.BootstrapPort"),
	                  0, 10),
	              hex_string_to_bin(
	                  Dictionary_get(pimcSelf->dictConfig, "Tox.BootstrapKey")),
	              &txberrTberr);

	if(txberrTberr != TOX_ERR_BOOTSTRAP_OK)
	{
		dbg("Failed to bootstrap\n");
	}

	tox_self_get_address(TOXINST, mypubkey);
	dbg("Tox ID: %s\n",bin_to_hex_string(mypubkey, TOX_ADDRESS_SIZE));

	register_callbacks(hobjSelf);
	getfriends(PRIVATE(pimcSelf)->ptoxTox, pimcSelf->lstContacts);

	thrd_create(&PRIVATE(pimcSelf)->thrdTox, toxthread, hobjSelf);

	SUBSCRIBE(clSaveData, hobjSelf, MCT_recv);

	SUBSCRIBE(frSendMsg, hobjSelf, MCT_recv);
	SUBSCRIBE(frDelete, hobjSelf, MCT_recv);

	msgContacts->V =pimcSelf->lstContacts;
	DISPATCH(clContacts, msgContacts);

	UNLOCK(pimcSelf)
	return 0;
}
