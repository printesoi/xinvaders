/*
Copyright notice:

This is mine.  I'm only letting you use it.  Period.  Feel free to rip off
any of the code you see fit, but have the courtesy to give me credit.
Otherwise great hairy beasties will rip your eyes out and eat your flesh
when you least expect it.

Jonny Goldman <jonathan@think.com>

Wed May  8 1991
*/

/* main.c -- create our windows and initialize things. */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define MAIN
#include "vaders.h"
#include <X11/Xaw/Form.h>
#ifndef X11R3
#include <X11/Xaw/Label.h>
#else
#include <X11/IntrinsicP.h>
#include <X11/Label.h>
#endif

struct windowSize {
	int width, height;
} windowSize;

GameInfo gameInfo;

#ifdef XFILESEARCHPATH
static void AddPathToSearchPath();
#endif

#include <locale.h>

static XrmOptionDescRec table[] = {
    {"-debug",	"*debug",	XrmoptionNoArg,	NULL},
};

static XtResource windowResourses[] = {
	{XtNwidth, XtCWidth, XtRInt, sizeof(int),
		XtOffsetOf(struct windowSize, width), XtRImmediate, (caddr_t)VWIDTH},
	{XtNheight, XtCHeight, XtRInt, sizeof(int),
		XtOffsetOf(struct windowSize, height), XtRImmediate, (caddr_t)VHEIGHT},
};

static XtResource resources[] = {
	{"debug", "Debug", XtRBoolean, sizeof(Boolean),
		XtOffsetOf(GameInfo, debug), XtRString, "off"},
	{"font", "Font", XtRString, sizeof(String),
		XtOffsetOf(GameInfo, vaderfont), XtRString, (String)"9x15"},
	{"scale", "Scale", XtRInt, sizeof(int),
		XtOffsetOf(GameInfo, scale), XtRImmediate, (caddr_t) 2},
	{"basewait", "BaseWait", XtRInt, sizeof(int),
		XtOffsetOf(GameInfo, basewait), XtRImmediate, (caddr_t) 10},
	{"vaderwait", "VaderWait", XtRInt, sizeof(int),
		XtOffsetOf(GameInfo, vaderwait), XtRImmediate, (caddr_t) 300},
	{"spacerwait", "SpacerWait", XtRInt, sizeof(int),
		XtOffsetOf(GameInfo, spacerwait), XtRImmediate, (caddr_t) 50},
	{"shotwait", "ShotWait", XtRInt, sizeof(int),
		XtOffsetOf(GameInfo, shotwait), XtRImmediate, (caddr_t) 10},
	{"vshotwait", "VshotWait", XtRInt, sizeof(int),
		XtOffsetOf(GameInfo, vshotwait), XtRImmediate, (caddr_t) 30},
	{"basecolor", "BaseColor", XtRPixel, sizeof(Pixel),
		XtOffsetOf(GameInfo, basepixel), XtRString, "cyan"},
	{"spacercolor", "SpacerColor", XtRPixel, sizeof(Pixel),
		XtOffsetOf(GameInfo, spacerpixel), XtRString, "gray"},
	{"buildingcolor", "BuildingColor", XtRPixel, sizeof(Pixel),
		XtOffsetOf(GameInfo, buildingpixel), XtRString, "yellow"},
	{"vader1color", "Vader1Color", XtRPixel, sizeof(Pixel),
		XtOffsetOf(GameInfo, vader1pixel), XtRString, "blue"},
	{"vader2color", "Vader2Color", XtRPixel, sizeof(Pixel),
		XtOffsetOf(GameInfo, vader2pixel), XtRString, "green"},
	{"vader3color", "Vader3Color", XtRPixel, sizeof(Pixel),
		XtOffsetOf(GameInfo, vader3pixel), XtRString, "red"},
	{"shotcolor", "ShotColor", XtRPixel, sizeof(Pixel),
		XtOffsetOf(GameInfo, shotpixel), XtRString, "lavender"},
	{"vshotcolor", "VshotColor", XtRPixel, sizeof(Pixel),
		XtOffsetOf(GameInfo, vshotpixel), XtRString, "orange"},
	{"scorecolor", "ScoreColor", XtRPixel, sizeof(Pixel),
		XtOffsetOf(GameInfo, scorepixel), XtRString, "white"},
	{"maxshots", "MaxShots", XtRInt, sizeof(int),
		XtOffsetOf(GameInfo, maxshots), XtRImmediate, (caddr_t) 1},
	{"maxvshots", "MaxVshots", XtRInt, sizeof(int),
		XtOffsetOf(GameInfo, maxvshots), XtRImmediate, (caddr_t) 6},
	{"gameInfo.defaultfore", "DefaultFore", XtRPixel, sizeof(Pixel),
		XtOffsetOf(GameInfo, defaultfore), XtRString, "white"},
	{"defaultback", "DefaultBack", XtRPixel, sizeof(Pixel),
		XtOffsetOf(GameInfo, defaultback), XtRString, "black"},
};


/*ARGSUSED*/
static void CvtStringToFloat(args, num_args, fromVal, toVal)
ArgList args;
Cardinal num_args;
XrmValue    *fromVal;
XrmValue    *toVal;
{
    static float  i;

    if (sscanf((char *)fromVal->addr, "%f", &i) == 1) {
	toVal->size = sizeof(float);
	toVal->addr = (caddr_t) &i;
    } else {
	toVal->size = 0;
	toVal->addr = NULL;
    }
}

static void AddResource(r, p)
char *r;
Pixel *p;
{
    XrmValue value;
    XrmDatabase db = XtDatabase(dpy);
    value.size = sizeof(Pixel);
    value.addr = (caddr_t) p;
    XrmPutResource(&db, r, XtRPixel, &value);
}

Widget
MakeCommandButton(box, name, function, vlink, hlink, data)
Widget box, vlink, hlink;
char *name;
XtCallbackProc function;
caddr_t data;
{
  Widget w;
  Arg args[10];
  Cardinal numargs;

  numargs = 0;
  if(vlink != NULL) {
    XtSetArg(args[numargs], XtNfromVert, vlink); numargs++;
  }
  if(hlink != NULL) {
    XtSetArg(args[numargs], XtNfromHoriz, hlink); numargs++;
  }
  w = XtCreateManagedWidget(name, commandWidgetClass, box, args, numargs);
  if (function != NULL)
    XtAddCallback(w, XtNcallback, function, data);
  return w;
}

int main(Cardinal argc, char **argv)
{
  static Arg args[10];
  int n;
  Widget form, button;
  /* extern WidgetClass labelwidgetclass; */

  setlocale(LC_ALL, "");
  textdomain(PACKAGE);

  me_image = NULL;

  srandom(time(0));
#ifdef XFILESEARCHPATH
  AddPathToSearchPath(XFILESEARCHPATH);
#endif
  toplevel = XtInitialize(argv[0], "Vaders", table, XtNumber(table),
			  &argc, argv);
  dpy = XtDisplay(toplevel);
  XtAddConverter(XtRString, XtRFloat, CvtStringToFloat, NULL, 0);
  XtGetApplicationResources(toplevel, &windowSize, windowResourses, XtNumber(windowResourses), NULL, 0);
  XtGetApplicationResources(toplevel, &gameInfo, resources, XtNumber(resources), NULL, 0);
  AddResource("*background", &gameInfo.defaultback);
  if (DisplayCells(dpy, DefaultScreen(dpy)) <= 2)
    {
      gameInfo.basepixel = gameInfo.defaultfore;
      gameInfo.buildingpixel = gameInfo.defaultfore;
      gameInfo.vader1pixel = gameInfo.defaultfore;
      gameInfo.vader2pixel = gameInfo.defaultfore;
      gameInfo.vader3pixel = gameInfo.defaultfore;
      gameInfo.shotpixel = gameInfo.defaultfore;
      gameInfo.vshotpixel = gameInfo.defaultfore;
      gameInfo.scorepixel = gameInfo.defaultfore;
    }
  if (gameInfo.scale<1) gameInfo.scale = 1;
  if (gameInfo.scale>2) gameInfo.scale = 2;

  windowSize.width = gameInfo.scale*VWIDTH;
  windowSize.height = gameInfo.scale*VHEIGHT;

  form = XtCreateManagedWidget ("form", formWidgetClass,
				toplevel, NULL, 0);

  n = 0;
  XtSetArg (args[n], XtNleft, XtChainLeft); n++;
  XtSetArg (args[n], XtNright, XtChainLeft); n++;
  XtSetArg (args[n], XtNtop, XtChainTop); n++;
  XtSetArg (args[n], XtNbottom, XtChainTop); n++;
  XtSetArg (args[n], XtNwidth, windowSize.width); n++;
  XtSetArg (args[n], XtNheight, windowSize.height); n++;

  gamewidget = (VadersWidget)
    XtCreateManagedWidget("field", vadersWidgetClass, form, args, n);

  XtSetKeyboardFocus (form, (Widget) gamewidget);

  n = 0;
  XtSetArg (args[n], XtNleft, XtChainLeft); n++;
  XtSetArg (args[n], XtNright, XtChainLeft); n++;
  XtSetArg (args[n], XtNtop, XtChainTop); n++;
  XtSetArg (args[n], XtNbottom, XtChainTop); n++;
  XtSetArg (args[n], XtNfromHoriz, gamewidget); n++;
  XtSetArg (args[n], XtNhorizDistance, 5); n++;
  XtSetArg (args[n], XtNwidth, gameInfo.scale*IWIDTH); n++;
  XtSetArg (args[n], XtNheight, windowSize.height/2); n++;

  labelwidget = (VadersWidget)
    XtCreateManagedWidget("label", vadersWidgetClass, form, args, n);

  pausebutton = MakeCommandButton(form, "pause", Pause, labelwidget, gamewidget, NULL);
  XtSetArg(args[0], XtNlabel,_(" Start"));
  XtSetArg(args[1], XtNforeground, gameInfo.defaultfore);
  XtSetArg(args[2], XtNbackground, gameInfo.defaultback);
  XtSetArg(args[3], XtNborderColor, gameInfo.defaultfore);
  XtSetValues(pausebutton, args, 4);
  button = MakeCommandButton(form, "quit", Quit, pausebutton, gamewidget, NULL);
  XtSetArg(args[0], XtNlabel,_(" Quit "));
  XtSetArg(args[1], XtNforeground, gameInfo.defaultfore);
  XtSetArg(args[2], XtNbackground, gameInfo.defaultback);
  XtSetArg(args[3], XtNborderColor, gameInfo.defaultfore);
  XtSetValues(button, args, 4);
  infobutton = MakeCommandButton(form, "info", ShowInfo, button, gamewidget, NULL);
  XtSetArg(args[0], XtNlabel,_(" Info "));
  XtSetArg(args[1], XtNforeground, gameInfo.defaultfore);
  XtSetArg(args[2], XtNbackground, gameInfo.defaultback);
  XtSetArg(args[3], XtNborderColor, gameInfo.defaultfore);
  XtSetValues(infobutton, args, 4);

  XtRealizeWidget(toplevel);
  ResetGame();
  XtMainLoop();
  return 0;
}

#ifdef XFILESEARCHPATH
static void
AddPathToSearchPath(path)
char *path;
{
     char *old, *new;
     extern char *getenv();

     old = getenv("XFILESEARCHPATH");
     if (old) {
#if defined(mips) || defined(hpux) || defined(sun)
	  /* +1 for =, +2 for :, +3 for null */
	  new = XtMalloc((Cardinal) (strlen("XFILESEARCHPATH") +
				     strlen(old) +
				     strlen(path) + 3));
	  (void) strcpy(new, "XFILESEARCHPATH");
	  (void) strcat(new, "=");
	  (void) strcat(new, old);
	  (void) strcat(new, ":");
	  (void) strcat(new, path);
	  putenv(new);
#else
	  /* +1 for colon, +2 for null */
	  new = XtMalloc((Cardinal) (strlen(old) + strlen(path) + 2));
	  (void) strcpy(new, old);
	  (void) strcat(new, ":");
	  (void) strcat(new, path);
	  setenv("XFILESEARCHPATH", new, 1);
#endif
     }
     else {
#if defined(mips) || defined(hpux) || defined(sun)
	  new = XtMalloc((Cardinal) (strlen("XFILESEARCHPATH") +
				     strlen(path) + 2));
	  (void) strcpy(new, "XFILESEARCHPATH");
	  (void) strcat(new, "=");
	  (void) strcat(new, path);
	  putenv(new);
#else
	  setenv("XFILESEARCHPATH", path, 1);
#endif
     }
}
#endif

#ifdef sparc
XShapeCombineMask()
{}

XShapeQueryExtension()
{}
#endif
