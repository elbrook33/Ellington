#ifndef X11_INTERFACE_H
#define X11_INTERFACE_H

#include <GL/glew.h>
#include <GL/glx.h>
#include <GL/gl.h>
#define NANOVG_GLEW
#define NANOVG_GL2_IMPLEMENTATION
#include "nanovg.h"
#include "nanovg_gl.h"
#include <X11/Xlib.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/Xatom.h>	// For WINDOW_TYPE
#include <X11/Xutil.h>	// For titles and app names
#include <stdio.h>
#include <stdbool.h>


// Header

// Define uiCanvas here, since it's needed for xWindow
typedef struct uiCanvas
{
	GLXContext glx;
	NVGcontext* nano;
	
	float left, right, top, bottom,
		fontSize, lineHeight;
	
	NVGcolor fgColour, bgColour;
	int bgImage;
	NVGpaint bgImagePattern;
} uiCanvas;

typedef struct xWindow
{
	Display* display;
	Window id;
	XWindowAttributes attributes;
	XClassHint name;
	uiCanvas canvas;
} xWindow;


// Implementation

#include "ui-toolkit.h"

xWindow xGetRoot(const char* displayID)
{
	xWindow root;

	root.display = XOpenDisplay(displayID);
	root.id = XDefaultRootWindow(root.display);
	XGetWindowAttributes(root.display, root.id, &root.attributes);

	return root;
}

xWindow xGetWindow(xWindow root, Window id)
{
	xWindow newWindow;
	
	newWindow.display = root.display;
	newWindow.id = id;
	XGetWindowAttributes(root.display, id, &newWindow.attributes);
	
	return newWindow;
}

Atom xGetWindowType(xWindow window)
{
	Atom flag_windowType =
		XInternAtom(window.display, "_NET_WM_WINDOW_TYPE", false );
	
	Atom typeReturned;
	int formatReturned;
	unsigned long numAtomsReturned, unreadBytes;
	unsigned char* rawData;
	
	XGetWindowProperty(window.display, window.id, flag_windowType,
		0L, 1L, false, XA_ATOM, &typeReturned, &formatReturned,
		&numAtomsReturned, &unreadBytes, &rawData);
	
	Atom type = numAtomsReturned > 0?
		((Atom*)rawData)[0] : 0;
	
	XFree(rawData);
	return type;
}

bool xIsNormal(xWindow window)
{
	Atom windowType = xGetWindowType(window);
	Atom flag_normalWindow =
		XInternAtom(window.display, "_NET_WM_WINDOW_TYPE_NORMAL", false);
	
	return window.attributes.override_redirect == false
		&& (windowType == flag_normalWindow
			|| windowType == 0);
}

xWindow xUpdateAttributes(xWindow window)
{
	XGetWindowAttributes(window.display, window.id, &window.attributes);
	XGetClassHint(window.display, window.id, &window.name);
	return window;
}

const char* xGetAppName(xWindow window)
{
}
const char* xGetTitle(xWindow window)
{
}

xWindow xCreate(xWindow root, int x, int y, int width, int height)
{
	xWindow newWindow;
	newWindow.display = XOpenDisplay(XDisplayString(root.display));
	
	XSetWindowAttributes attributes;
	attributes.override_redirect = true;
	
	newWindow.id = XCreateWindow(newWindow.display, root.id,
		x, y, width, height, 0,
		root.attributes.depth, CopyFromParent, CopyFromParent,
		CWOverrideRedirect, &attributes
	);
	XGetWindowAttributes(newWindow.display, newWindow.id, &newWindow.attributes);
	
	newWindow.canvas = uiGet(newWindow);
	
	return newWindow;
}

#endif
