//
// Routines for creating windows and getting their attributes
//

#ifndef X11_INTERFACE_H
#define X11_INTERFACE_H

#include "ui-toolkit.h"
#include "ui-types.h"

xWindow xUpdateAttributes(xWindow window)
{
	XGetWindowAttributes(window.display, window.id, &window.attributes);
	XGetClassHint(window.display, window.id, &window.name);
	return window;
}

xWindow xGetRoot(const char* displayID)
{
	xWindow root;

	root.display = XOpenDisplay(displayID);
	root.id = XDefaultRootWindow(root.display);
	root = xUpdateAttributes(root);

	return root;
}

xWindow xGetWindow(xWindow root, Window id)
{
	xWindow newWindow;
	
	newWindow.display = root.display;
	newWindow.id = id;
	newWindow = xUpdateAttributes(newWindow);
	
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
	Atom flag_dialogWindow =
		XInternAtom(window.display, "_NET_WM_WINDOW_TYPE_DIALOG", false);
	
	return window.attributes.override_redirect == false
		&& (windowType == flag_normalWindow
			|| windowType == flag_dialogWindow
			|| windowType == 0);
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
	
	newWindow = xUpdateAttributes(newWindow);
	newWindow.canvas = uiGet(newWindow);
	
	return newWindow;
}

#endif
