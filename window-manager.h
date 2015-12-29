//
// Window manager
//
// This is the main starting point for the app.
// It starts following new windows and sets up hot keys.
// It also launches the desktop wallpaper (which handles shadows and window movements) and panel.
//

#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include "ui-toolkit.h"
#include "helpers.h"


// Some default dimensions
const int
	wmTop = 22,
	wmOuter = 15,
	wmInner = 10;


// Headers (used by panel and wallpaper)
#define wmWorkspace(desktop) desktop.workspaces[desktop.activeWorkspace]
#define wmLength(desktop) desktop.workspaceLengths[desktop.activeWorkspace]

wmSession wmLayoutWorkspace(wmSession desktop);
wmSession wmSwapPlaces(wmSession desktop, int indexA, int indexB);
wmSession wmUpdateWindowAttributes(wmSession desktop);
int wmErrorHandler(Display* display, XErrorEvent* error);

#include "panel.h"
#include "wallpaper.h"


// Functions
wmSession wmGet(const char* displayID)
{
	xWindow root = xGetRoot(displayID);
	
//	XCompositeRedirectSubwindows(root.display, root.id, CompositeRedirectAutomatic);
	XSelectInput(root.display, root.id,
		SubstructureRedirectMask|SubstructureNotifyMask
	);
	
	// 0-9
	XGrabKey(root.display, 10, Mod4Mask, root.id, true, GrabModeAsync, GrabModeAsync);
	XGrabKey(root.display, 11, Mod4Mask, root.id, true, GrabModeAsync, GrabModeAsync);
	XGrabKey(root.display, 12, Mod4Mask, root.id, true, GrabModeAsync, GrabModeAsync);
	XGrabKey(root.display, 13, Mod4Mask, root.id, true, GrabModeAsync, GrabModeAsync);
	XGrabKey(root.display, 14, Mod4Mask, root.id, true, GrabModeAsync, GrabModeAsync);
	XGrabKey(root.display, 15, Mod4Mask, root.id, true, GrabModeAsync, GrabModeAsync);
	XGrabKey(root.display, 16, Mod4Mask, root.id, true, GrabModeAsync, GrabModeAsync);
	XGrabKey(root.display, 17, Mod4Mask, root.id, true, GrabModeAsync, GrabModeAsync);
	XGrabKey(root.display, 18, Mod4Mask, root.id, true, GrabModeAsync, GrabModeAsync);
	XGrabKey(root.display, 19, Mod4Mask, root.id, true, GrabModeAsync, GrabModeAsync);
	
	XGrabKey(root.display, 27, Mod4Mask, root.id, true, GrabModeAsync, GrabModeAsync);	// R
	XGrabKey(root.display, 28, Mod4Mask, root.id, true, GrabModeAsync, GrabModeAsync);	// T
	
	// Create components
	wmSession desktop = {0};
	desktop.root = root;
	
	desktop.wallpaper = wallpaperGet(desktop);
	wallpaperRedraw(desktop);
	
	desktop.panel = panelGet(desktop);
	panelRedraw(desktop);
	
	XSetErrorHandler(wmErrorHandler);
	
	return desktop;
}

wmSession wmUpdateWindowAttributes(wmSession desktop)
{
	for(int window = 0; window < wmLength(desktop); window++)
	{
		wmWorkspace(desktop)[window] = xUpdateAttributes(wmWorkspace(desktop)[window]);
	}
	return desktop;
}

wmSession wmLayoutWorkspace(wmSession desktop)
{
	printf("Layout %i x %i\n", desktop.activeWorkspace, wmLength(desktop));
	
	const int
		topY = wmTop + wmOuter + wmTop,
		leftX = wmOuter,
		
		fullW = desktop.root.attributes.width - leftX - wmOuter,
		halfW = fullW / 2 - wmInner,
		
		fullH = desktop.root.attributes.height - topY - wmOuter,
		halfH = (wmTop + fullH) / 2 - wmTop - wmInner,
		
		rightX = leftX + halfW + wmInner + wmInner,
		bottomY = topY + halfH + wmInner + wmInner + wmTop;
	
	switch(wmLength(desktop))
	{
		case 1:
			XMoveResizeWindow(desktop.root.display, wmWorkspace(desktop)[0].id,
				leftX, topY, fullW, fullH
			);
			break;
		case 2:
			XMoveResizeWindow(desktop.root.display, wmWorkspace(desktop)[0].id,
				leftX, topY, halfW, fullH
			);
			XMoveResizeWindow(desktop.root.display, wmWorkspace(desktop)[1].id,
				rightX, topY, halfW, fullH
			);
			break;
		case 3:
			XMoveResizeWindow(desktop.root.display, wmWorkspace(desktop)[0].id,
				leftX, topY, halfW, fullH
			);
			XMoveResizeWindow(desktop.root.display, wmWorkspace(desktop)[1].id,
				rightX, topY, halfW, halfH
			);
			XMoveResizeWindow(desktop.root.display, wmWorkspace(desktop)[2].id,
				rightX, bottomY, halfW, halfH
			);
			break;
		case 4:
			XMoveResizeWindow(desktop.root.display, wmWorkspace(desktop)[0].id,
				leftX, topY, halfW, halfH
			);
			XMoveResizeWindow(desktop.root.display, wmWorkspace(desktop)[1].id,
				rightX, topY, halfW, halfH
			);
			XMoveResizeWindow(desktop.root.display, wmWorkspace(desktop)[2].id,
				rightX, bottomY, halfW, halfH
			);
			XMoveResizeWindow(desktop.root.display, wmWorkspace(desktop)[3].id,
				leftX, bottomY, halfW, halfH
			);
			break;
	}
	
	desktop = wmUpdateWindowAttributes(desktop);
	wallpaperRedraw(desktop);
	
	return desktop;
}

wmSession wmSwitchToWorkspace(wmSession desktop, int workspace)
{
	if(workspace == desktop.activeWorkspace) { return desktop; }

	for(int window = 0; window < wmLength(desktop); window++)
	{
		XUnmapWindow(desktop.root.display, wmWorkspace(desktop)[window].id);
	}
	
	desktop.activeWorkspace = workspace;
	desktop = wmLayoutWorkspace(desktop);
	
	for(int window = 0; window < wmLength(desktop); window++)
	{
		XMapWindow(desktop.root.display, wmWorkspace(desktop)[window].id);
	}
	return desktop;
}

wmSession wmSwapPlaces(wmSession desktop, int indexA, int indexB)
{
	xWindow placeholder = wmWorkspace(desktop)[indexA];
	wmWorkspace(desktop)[indexA] = wmWorkspace(desktop)[indexB];
	wmWorkspace(desktop)[indexB] = placeholder;
	
	desktop = wmLayoutWorkspace(desktop);
	return desktop;
}

wmSession wmRemoveByID(wmSession desktop, int workspace, Window id)
{
	for(int window = 0; window < desktop.workspaceLengths[workspace]; window++)
	{
		// Find matching window in each workspace
		if(desktop.workspaces[workspace][window].id == id)
		{
			// Shift other windows into its place
			for(int shift = window + 1; shift < desktop.workspaceLengths[workspace]; shift++)
			{
				desktop.workspaces[workspace][shift - 1] = desktop.workspaces[workspace][shift];
			}
			desktop.workspaceLengths[workspace] -= 1;
			break;
		}
	}
	return desktop;
}

wmSession wmRemoveEverywhereByID(wmSession desktop, Window id)
{
	for(int workspace = 0; workspace < 10; workspace++)
	{
		desktop = wmRemoveByID(desktop, workspace, id);
	}
	return desktop;
}

wmSession wmGlobalEvents(wmSession desktop)
{
	if(XPending(desktop.root.display) == 0)
		return desktop;
	
	XEvent event;
	XNextEvent(desktop.root.display, &event);

	xWindow targetedWindow;
	switch(event.type)
	{
		case MapRequest:
			printf("wmGlobalEvent MapRequest\n");
			
			targetedWindow = xGetWindow(desktop.root, event.xmaprequest.window);
			if(xIsNormal(targetedWindow))
			{
				// Listen for mouse entering the new window (in wallpaper.h)
				XSelectInput(desktop.wallpaper.display, targetedWindow.id, EnterWindowMask);
				
				// New top-level application windows are placed at the end of the workspace
				if(wmLength(desktop) < 4)
					wmLength(desktop) += 1;
				else
					XUnmapWindow(desktop.root.display,
						wmWorkspace(desktop)[3].id);

				wmWorkspace(desktop)
					[wmLength(desktop) - 1]
						= targetedWindow;
				
				XSetWindowBorderWidth(desktop.root.display, targetedWindow.id, 0);
				desktop = wmLayoutWorkspace(desktop);
			}
			XMapWindow(desktop.root.display, targetedWindow.id);
			break;
		
		case ConfigureRequest:
			printf("wmGlobalEvent ConfigureRequest %li\n", event.xconfigurerequest.window);
			
			// Sign-off on all changes to dialogs, popups, etc.
			XWindowChanges changes;
			changes.x = event.xconfigurerequest.x;
			changes.y = event.xconfigurerequest.y;
			changes.width = event.xconfigurerequest.width;
			changes.height = event.xconfigurerequest.height;
			changes.border_width = event.xconfigurerequest.border_width;
			changes.sibling = event.xconfigurerequest.above;
			changes.stack_mode = event.xconfigurerequest.detail;
			XConfigureWindow(desktop.root.display, event.xconfigurerequest.window,
				event.xconfigurerequest.value_mask, &changes);
			
			// Fix layout for top-level application windows
			targetedWindow = xGetWindow(desktop.root, event.xconfigurerequest.window);			
			if(xIsNormal(targetedWindow))
			{
				desktop = wmLayoutWorkspace(desktop);
			}
			break;
		
		case UnmapNotify:
			printf("wmGlobalEvent UnmapNotify\n");
			
			desktop = wmRemoveByID(desktop, desktop.activeWorkspace, event.xunmap.window);
			desktop = wmLayoutWorkspace(desktop);
			break;
		
		case DestroyNotify:
			printf("wmGlobalEvent DestroyNotify\n");

			desktop = wmRemoveEverywhereByID(desktop, event.xdestroywindow.window);
			desktop = wmLayoutWorkspace(desktop);
			break;
		
		case KeyPress:
			switch(event.xkey.keycode)
			{
				// 0-9
				case 10: desktop = wmSwitchToWorkspace(desktop, 0); break;
				case 11: desktop = wmSwitchToWorkspace(desktop, 1); break;
				case 12: desktop = wmSwitchToWorkspace(desktop, 2); break;
				case 13: desktop = wmSwitchToWorkspace(desktop, 3); break;
				case 14: desktop = wmSwitchToWorkspace(desktop, 4); break;
				case 15: desktop = wmSwitchToWorkspace(desktop, 5); break;
				case 16: desktop = wmSwitchToWorkspace(desktop, 6); break;
				case 17: desktop = wmSwitchToWorkspace(desktop, 7); break;
				case 18: desktop = wmSwitchToWorkspace(desktop, 8); break;
				case 19: desktop = wmSwitchToWorkspace(desktop, 9); break;
				
				// Launchers
				case 27:	system("xfce4-appfinder --disable-server &"); break;	// R
				case 28:	system("x-terminal-emulator &"); break;	// T
				
				// Cmd-X
				// Cmd-C
				// Cmd-V
				// Cmd-D
				// Cmd-(Left|Right)
			}
			break;
	}
	
	// Go again
	return wmGlobalEvents(desktop);
}

// Keep track of BadWindows (errors for destroyed windows), to remove them from layouts later
struct wmRemovals
{
	void* sessionID;
	Window queue[40];
	int length;
};
struct wmRemovals wmRemovals = {0};

wmSession wmEvents(wmSession desktop)
{
	// Handle events
	desktop = wmGlobalEvents(desktop);
	desktop = panelEvents(desktop);
	desktop = wallpaperEvents(desktop);
	
	// Remove failed windows
	for(int i = 0; i < wmRemovals.length; i++)
	{
		desktop = wmRemoveEverywhereByID(desktop, wmRemovals.queue[i]);
	}
	wmRemovals.length = 0;
	
	// Set up polling (by select) - i.e. waiting till the next event
	int selectMax = 0, connection;
	fd_set selectMask;
	FD_ZERO(&selectMask);
	
	connection = XConnectionNumber(desktop.root.display);
	selectMax = max(selectMax, connection);
	FD_SET(connection, &selectMask);
	
	connection = XConnectionNumber(desktop.panel.display);
	selectMax = max(selectMax, connection);
	FD_SET(connection, &selectMask);
	
	connection = XConnectionNumber(desktop.wallpaper.display);
	selectMax = max(selectMax, connection);
	FD_SET(connection, &selectMask);
	
	struct timeval timeout;
	timeout.tv_sec = 60;
	timeout.tv_usec = 0;
	
	select(selectMax, &selectMask, NULL, NULL, &timeout);
	
	return desktop;
}

// Replacement X11 error handler - note BadWindows
int wmErrorHandler(Display* display, XErrorEvent* error) {
	printf("X11 error. Request %i, code %i.%i, resource %li\n",
		error->request_code, error->error_code, error->minor_code, error->resourceid);
	
	char errorText[100];
	XGetErrorText(display, error->error_code, errorText, 100);
	printf("Error code: %s\n", errorText);
	
	XGetErrorText(display, error->minor_code, errorText, 100);
	printf("Minor code: %s\n", errorText);
	
	if(error->error_code == BadWindow)
	{
		wmRemovals.queue[wmRemovals.length] = error->resourceid;
		wmRemovals.length += 1;
	}
	
	return error->error_code;
}

#endif
