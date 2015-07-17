#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <FL/Fl.H>

#include "Module/Module.h"
#include "dictionary.h"
#include "misc.h"
#include "postbox.h"

#include "GUIFLTK.h"
#include "control/xwin.h"

GUIFLTK *pgflCurrent;

GUIFLTK::GUIFLTK(XWF_ObjectParams_t *prmObj)
{
	dbg("Initialising a new GUI for FLTK\n");
}

int GUIFLTK::start()
{
	Xw_ =new XwinTox(this, 640, 480, "XwinTox", 1);//1);
	Xw_->show();

	thrd_create(&thrdFLTK_, (thrd_start_t)fltkLoop, 0);

	setFLTKCallbacks();
	return 0;
}

int GUIFLTK::fltkLoop(void *)
{
	while(1)
	{
		void *msg =0;
		Fl::lock();
		Fl::wait();
		msg = Fl::thread_message();
		if (msg) handle(msg);
		Fl::unlock();
	}
	return 0;
}
