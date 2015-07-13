#include <ctype.h>

#include <FL/Fl.H>
#include <FL/Fl_Multiline_Input.H>
#include <FL/fl_draw.H>

#include "misc.h"
#include "postbox.h"

#include "control/xwin.h"
#include "control/gaddfrnd.h"

void af_sendrequest(const char* id, const char* msg)
{
	/*	Event_t *e1 =Ev_new();
		Event_t *e2 =Ev_new();
		Event_t *e3 =Ev_new();

		e1->T =Comm_SendFriendRequest;
		e1->S1 =strdup(id);
		e1->S2 =strdup(msg);

		e2->T =Comm_SaveData;

		e3->T =Comm_GetFriendList;

		List_add(APP->Comm->WorkQueue, (void*)e1);
		List_add(APP->Comm->WorkQueue, (void*)e2);
		CommWork();*/
}

void af_post(int mtype, PBMessage_t* msg, void* custom)
{
	/*GAddFriend *g =(GAddFriend*)custom;
	dbg("Post: %d\n", mtype);

	if(mtype == PB_DNSResolved)
	{
		af_sendrequest(msg->S1, g->message->value());
		g->id->value("");
	}*/
}

void removespaces(char * s)
{
	char *p =s;
	int l =strlen(p);

	while(isspace(p[l - 1])) p[--l] =0;

	while(*p && isspace(*p)) ++p, --l;

	memmove(s, p, l + 1);
}

void af_pressed(Fl_Widget *w, void *custom)
{
	GAddFriend *g =(GAddFriend*)custom;
	char *id =strdup(g->id->value());
	const char *msg =g->message->value();

	removespaces(id);

	/* try to identify what type of request this is */
	if((strlen(id) == 76) && !strchr(id, '@'))
	{
		/* a regular tox key */
		dbg("Regular Tox key\n");
		af_sendrequest(id, msg);
		g->id->value("");
		free(id);
		return;
	}
	else if(strchr(id, '@'))
	{
		dbg("ToxDNS ID\n");
		/*	Call_t *call =(Call_t*)calloc(1, sizeof(Call_t));
			call->Func =R_DNSResolve;
			call->S1 =id;
			List_add(APP->Resolv->Calls, call);
			ResolvAddWork(1);*/
		return;
	}
	else
	{
		dbg("ID unrecognised");
		free(id);
	}

}

GAddFriend::GAddFriend(const XWF_hObj_t* hObj, int S) :
	GArea(hObj, S, "Add Friends")
{
	scale = S;

	id =new Fl_Input(x() + (10 * S), y() + (90 * S),
	                 (x() + w() - (224 * S) - (20 * S)),
	                 24 * S);
	message =new Fl_Multiline_Input(x() + (10 * S), y() + (140 * S),
	                                (x() + w() - (224 * S) - (20 * S)),
	                                84 * S);

	send =new Fl_Button(parent()->w() - 62 * scale, y() + 234 * scale,
	                    52 * scale, 20 * scale, "Add");


	id->textsize(12 * S);

	message->textsize(12 * S);
	message->value("Please accept this friend request.");

	send->color(fl_rgb_color(107, 194, 96));
	send->labelcolor(255);
	send->labelsize(14 * S);
	send->callback(af_pressed, this);

	//PB_Register(APP->events, PB_DNSResolved, this, af_post);

	end();
}

void GAddFriend::resize(int X, int Y, int W, int H)
{
	Fl_Group::resize(X, Y, W, H);
	id->resize(x() + (10 * scale), y() + (90 * scale),
	           (x() + w() - (224 * scale) - (20 * scale)),
	           24 * scale);
	message->resize(x() + (10 * scale), y() + (140 * scale),
	                (x() + w() - (224 * scale) - (20 * scale)),
	                84 * scale);
	send->resize(parent()->w() - 62 * scale, y() + 234 * scale, 52 * scale,
	             20 * scale);
}

void GAddFriend::draw()
{
	GArea::draw();

	fl_color(0);
	fl_font(FL_HELVETICA, 12 * scale);
	fl_draw("Tox ID", x() + (10 * scale), y() + 84 * scale);
	fl_draw("Message", x() + (10 * scale), y() + 134 * scale);
}