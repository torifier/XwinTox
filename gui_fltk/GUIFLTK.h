#ifndef __GUIFLTK__H
#define __GUIFLTK__H

#include <threads.h>
#include <FL/Fl_Widget.H>

#include "AOM/IXWClass.h"
#include "Module/Module.h"
#include "misc.h"
#include "control/xwin.h"

class GUIFLTK : public XWClassT<GUIFLTK>
{
public:
	GUIFLTK(XWF_ObjectParams_t *pobjParams);
	int start();

	void cbAddFriendSend(Fl_Widget* w);
private:
	static int fltkLoop(void*);
	void setFLTKCallbacks();
	XwinTox *Xw_; 
	thrd_t thrdFLTK_;
};

template<class T, class Method, Method m>
static void flThunk(Fl_Widget *w, void *userData)
{
	return ((*static_cast<T *>(userData)).*m)(w);
}

typedef void (*FLTK_Callback_f)(Fl_Widget *w, void *userData);

extern GUIFLTK *pgflCurrent;

#define FLCB(FUNC) reinterpret_cast<FLTK_Callback_f> \
	(flThunk<GUIFLTK, decltype(&GUIFLTK::FUNC), &GUIFLTK::FUNC>), this


#endif
