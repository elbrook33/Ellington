#ifndef WALLPAPER_H
#define WALLPAPER_H

#include "window-manager.h"


// Header

typedef struct wallpaperDragState
{
	bool active;
	xWindow window;
	int index;
	int offsetX, offsetY;
} wallpaperDragState;

wallpaperDragState wallpaperDrag;


// Implementation

xWindow wallpaperGet(wmSession desktop)
{
	xWindow wallpaper = xCreate(desktop.root, 0, 0, desktop.root.attributes.width, desktop.root.attributes.height);
	wallpaper.canvas = uiSet(wallpaper, 0, 0, 1, 0, 0.1, 0.5, "/opt/ellington/Resources/Desktop.jpg");

	XSelectInput(wallpaper.display, wallpaper.id,
		PointerMotionMask|ButtonPressMask|ButtonReleaseMask);
	XMapWindow(wallpaper.display, wallpaper.id);

	return wallpaper;
}


// Drawing

void wallpaperRedraw(wmSession desktop)
{
	glXMakeCurrent(desktop.wallpaper.display, desktop.wallpaper.id, desktop.wallpaper.canvas.glx);
	nvgBeginFrame(desktop.wallpaper.canvas.nano,
		desktop.wallpaper.attributes.width, desktop.wallpaper.attributes.height, 1/1
	);
	
	// Background image
	nvgBeginPath(desktop.wallpaper.canvas.nano);
	nvgFillPaint(desktop.wallpaper.canvas.nano, desktop.wallpaper.canvas.bgImagePattern);
	nvgRect(desktop.wallpaper.canvas.nano,
		0, 0, desktop.wallpaper.attributes.width, desktop.wallpaper.attributes.height
	);
	nvgFill(desktop.wallpaper.canvas.nano);
	
	for(int window = 0; window < wmLength(desktop); window++)
	{
		if(window == desktop.activeWindow)
		{
			// Shadow
			if(!wallpaperDrag.active)
			{
				int shadowSize = 20, shadowOffset = 10;
				nvgBeginPath(desktop.wallpaper.canvas.nano);
				nvgFillPaint(desktop.wallpaper.canvas.nano,
					nvgBoxGradient(desktop.wallpaper.canvas.nano,
						wmWorkspace(desktop)[window].attributes.x,
						wmWorkspace(desktop)[window].attributes.y - wmTop + shadowOffset,
						wmWorkspace(desktop)[window].attributes.width,
						wmWorkspace(desktop)[window].attributes.height + wmTop,
						shadowSize / 2,
						2 * shadowSize,
						nvgHSLA(0, 0, 0, 191),
						nvgHSLA(0, 0, 0, 0)
					)
				);
				nvgRect(desktop.wallpaper.canvas.nano,
					wmWorkspace(desktop)[window].attributes.x - shadowSize,
					wmWorkspace(desktop)[window].attributes.y - wmTop - shadowSize + shadowOffset,
					wmWorkspace(desktop)[window].attributes.width + 2 * shadowSize,
					wmWorkspace(desktop)[window].attributes.height + wmTop + 2 * shadowSize
				);
				nvgFill(desktop.wallpaper.canvas.nano);
			}
			
			// Darker titlebar
			nvgBeginPath(desktop.wallpaper.canvas.nano);
			nvgFillColor(desktop.wallpaper.canvas.nano, nvgHSL(0, 0.1, 0.5));
		}
		else
		{
			// Lighter titlebar
			nvgBeginPath(desktop.wallpaper.canvas.nano);
			nvgFillColor(desktop.wallpaper.canvas.nano, nvgHSL(0, 0.1, 0.75));
		}
		nvgRect(desktop.wallpaper.canvas.nano,
			wmWorkspace(desktop)[window].attributes.x,
			wmWorkspace(desktop)[window].attributes.y - wmTop,
			wmWorkspace(desktop)[window].attributes.width,
			wmTop
		);
		nvgFill(desktop.wallpaper.canvas.nano);
	}
	
	nvgEndFrame(desktop.wallpaper.canvas.nano);
	glFlush();
}


// Drag events

int wallpaperWindowAt(wmSession desktop, int x, int y)
{
	int xMid = desktop.root.attributes.width / 2,
		yMid = (desktop.root.attributes.height - wmTop) / 2 + wmTop;
	if(x > xMid && wmLength(desktop) > 1)
	{
		if(y > yMid && wmLength(desktop) > 2)
			{ return 2; }
		else
			{ return 1; }
	}
	else
	{
		if(y > yMid && wmLength(desktop) > 3)
			{ return 3; }
		else
			{ return 0; }
	}
}

wmSession wallpaperEvents(wmSession desktop)
{
	if(XPending(desktop.wallpaper.display) == 0) { return desktop; }
	
	XEvent event;
	XNextEvent(desktop.wallpaper.display, &event);

	int newX, newY;
	int targetIndex;
	
	switch(event.type)
	{
		case EnterNotify:
			if(wallpaperDrag.active) { break; }
			for(int window = 0; window < wmLength(desktop); window++)
			{
				if(wmWorkspace(desktop)[window].id == event.xcrossing.window)
				{
					desktop.activeWindow = window;
					wallpaperRedraw(desktop);
					break;
				}
			}
			break;
		
		case ButtonPress:
			// Start drag
			wallpaperDrag.active = true;
			wallpaperDrag.index = wallpaperWindowAt(desktop, event.xmotion.x_root, event.xmotion.y_root);
			wallpaperDrag.window = wmWorkspace(desktop)[wallpaperDrag.index];
			wallpaperDrag.offsetX = event.xmotion.x_root - wallpaperDrag.window.attributes.x;
			wallpaperDrag.offsetY = event.xmotion.y_root - wallpaperDrag.window.attributes.y;
			break;
		
		case MotionNotify:
			// Handle drag
			if(!wallpaperDrag.active)
			{
				targetIndex = wallpaperWindowAt(desktop, event.xmotion.x_root, event.xmotion.y_root);
				if(targetIndex != desktop.activeWindow)
				{
					desktop.activeWindow = targetIndex;
					wallpaperRedraw(desktop);
				}
				break;
			}
			
			newX = event.xmotion.x_root - wallpaperDrag.offsetX;
			newY = max(event.xmotion.y_root - wallpaperDrag.offsetY, wmTop + wmTop);
			
			// Limit movements
			if(wallpaperDrag.index == 0 || wallpaperDrag.index == 3)
			{
				if(wmLength(desktop) > 1)
					{ newX = min(newX, wmOuter + 2 * wmInner); }
			}
			else
				{ newX = max(newX, desktop.root.attributes.width / 2 - wmInner); }

			if(wallpaperDrag.index <= 1)
			{
				if(wallpaperDrag.index == 0 && wmLength(desktop) > 3
				|| wallpaperDrag.index == 1 && wmLength(desktop) > 2)
					{ newY = min(newY, wmTop + wmOuter + wmTop + 2 * wmInner); }
			}
			else
				{ newY = max(newY, (desktop.root.attributes.height - wmTop) / 2 + wmTop - wmInner + wmTop); }
			
			// Move windows and redraw
			XMoveWindow(desktop.root.display, wallpaperDrag.window.id, newX, newY);
			desktop = wmUpdateWindowAttributes(desktop);
			wallpaperRedraw(desktop);
			
			// Check if overlapping
			// Top-left corner
			targetIndex = wallpaperWindowAt(desktop,
				newX,
				newY - wmTop
			);
			if(targetIndex != wallpaperDrag.index)
				{ desktop.activeWindow = targetIndex; break; }
			
			// Top-right corner
			targetIndex = wallpaperWindowAt(desktop,
				newX + wallpaperDrag.window.attributes.width,
				newY - wmTop
			);
			if(targetIndex != wallpaperDrag.index)
				{ desktop.activeWindow = targetIndex; break; }
			
			// Bottom-right corner
			targetIndex = wallpaperWindowAt(desktop,
				newX + wallpaperDrag.window.attributes.width,
				newY + wallpaperDrag.window.attributes.height
			);
			if(targetIndex != wallpaperDrag.index)
				{ desktop.activeWindow = targetIndex; break; }
			
			// Bottom-left corner
			targetIndex = wallpaperWindowAt(desktop,
				newX,
				newY + wallpaperDrag.window.attributes.height
			);
			if(targetIndex != wallpaperDrag.index)
				{ desktop.activeWindow = targetIndex; break; }

			break;
		
		case ButtonRelease:
			// End drag
			if(wallpaperDrag.active)
			{
				wallpaperDrag.active = false;
				desktop = wmSwapPlaces(desktop, wallpaperDrag.index, desktop.activeWindow);
			}
			break;
	}
	
	return wallpaperEvents(desktop);
}

#endif
