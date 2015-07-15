#ifndef GSETTINGS_H_
#define GSETTINGS_H_

#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Tabs.H>

#include "control/garea.h"

class GSettings: public GArea
{
public:
	// Ctor
	GSettings(const XWF_hObj_t* hObj, int S);

	void resize(int X, int Y, int W, int H);
	void draw();

	Fl_Tabs *pages;

	Fl_Group *pagePlugins, *pageIdentity;

	/* pagePlugins */
	//Fl_Browser*tableMods;
	/* end pagePlugins */

	const XWF_hObj_t* hObj_;
};

#endif