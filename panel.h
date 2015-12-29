#ifndef PANEL_H
#define PANEL_H

#include "window-manager.h"
#include "ui-toolkit.h"
#include "x11-interface.h"

xWindow panelGet(wmSession desktop)
{
	xWindow panel = xCreate(desktop.root, 0, 0, desktop.root.attributes.width, wmTop);
	panel.canvas = uiSet(panel, 0, 0, 0.2, 0, 0.1, 0.9, NULL);

	XSelectInput(panel.display, panel.id, ButtonPressMask);
	XMapWindow(panel.display, panel.id);

	return panel;
}

void panelRedraw(wmSession desktop)
{
	uiDraw(desktop.panel, "*Applications* Desktops Windows	> 3.14pm <	> *Exit* >");
}

wmSession panelEvents(wmSession desktop)
{
	if(XPending(desktop.panel.display) == 0) { return desktop; }
	
	XEvent event;
	XNextEvent(desktop.panel.display, &event);

	switch(event.type)
	{
	}

	return desktop;
}

#endif
