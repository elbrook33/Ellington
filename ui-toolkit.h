#ifndef UI_TOOLKIT_H
#define UI_TOOLKIT_H

#include "theme.h"
#include "ui-parser.h"
#include "ui-types.h"


// Headers

uiCanvas uiGet(xWindow window);

#include "x11-interface.h"


// Implementation

float uiMargin = 5;

uiCanvas uiGet(xWindow window)
{
	uiCanvas canvas;

	int glxCount;
	int requests[] = {
		GLX_RENDER_TYPE, GLX_RGBA_BIT,
		GLX_DOUBLEBUFFER, false,
		0
	};

	GLXFBConfig* glxOptions = glXChooseFBConfig(window.display, XDefaultScreen(window.display),
		requests, &glxCount
	);

	canvas.glx = glXCreateNewContext(window.display, glxOptions[0], GLX_RGBA_TYPE, NULL, true);	
	glXMakeCurrent(window.display, window.id, canvas.glx);
	
	if(glewInit() != GLEW_OK) { printf("Failed to start glew\n"); }
	
	canvas.nano = nvgCreateGL2(NVG_ANTIALIAS);
	if (canvas.nano == NULL) { printf("Failed to start NanoVG\n"); }
	
	// Default settings
	canvas.left = uiMargin;
	canvas.right = window.attributes.width - uiMargin;
	canvas.top = uiMargin;
	canvas.bottom = window.attributes.height - uiMargin;
	canvas.fontSize = 13;
	canvas.lineHeight = 15;
	canvas.fgColour = nvgHSL(0, 0, 0);
	canvas.bgColour = nvgHSL(0, 0, 1);
	
	// Default fonts
	if (nvgCreateFont(canvas.nano, "normal", themeFont) == -1)
		{ printf("Failed to create font normal.\n"); }
	if (nvgCreateFont(canvas.nano, "bold", themeFontBold) == -1)
		{ printf("Failed to create font bold.\n"); }
	if (nvgCreateFont(canvas.nano, "italic", themeFontItalic) == -1)
		{ printf("Failed to create font italic.\n"); }
	
	return canvas;
}

uiCanvas uiSet(xWindow window, float fgH, float fgS, float fgL, float bgH, float bgS, float bgL, const char* bgImage)
{
	window.canvas.fgColour = nvgHSL(fgH, fgS, fgL);
	window.canvas.bgColour = nvgHSL(bgH, bgS, bgL);
	window.canvas.bgImage = nvgCreateImage(window.canvas.nano, bgImage, 0);
	window.canvas.bgImagePattern = nvgImagePattern(window.canvas.nano,
		0, 0, window.attributes.width, window.attributes.height, 0,
		window.canvas.bgImage, 1
	);
	return window.canvas;
}

bool uiDrawAction(xWindow window,
	const char* start, const char* end,
	float x, float y, float* bounds,
	int* indices, void* data)
{
	printf("Draw text %s (%i.%i.%i) at %f, %f\n", start, indices[0], indices[1], indices[2], x, y);

	nvgText(window.canvas.nano, x, y, start, end);	
	return true;	// Keep going
}

void uiDraw(xWindow window, const char* markup)
{
	glXMakeCurrent(window.display, window.id, window.canvas.glx);
	nvgBeginFrame(window.canvas.nano, window.attributes.width, window.attributes.height, 1/1);
	
	// Background
	nvgBeginPath(window.canvas.nano);
	if(window.canvas.bgImage)
		{ nvgFillPaint(window.canvas.nano, window.canvas.bgImagePattern); }
	else
		{ nvgFillColor(window.canvas.nano, window.canvas.bgColour); }
	nvgRect(window.canvas.nano, 0, 0, window.attributes.width, window.attributes.height);
	nvgFill(window.canvas.nano);
	
	// Parse markup
	uiParse(window, markup, uiDrawAction, NULL);
	
	nvgEndFrame(window.canvas.nano);
	glFlush();
}

#endif
