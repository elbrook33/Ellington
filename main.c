//
// Ellington Desktop
// =================
//
// To do
// -----
// * Panel applets: applications menu, workspace switcher, windows list, clock, status tray, log out.
// * Extra workspace actions (e.g. copy and paste).
// * Titlebar text and close button.
// * Full-screen.
// * Problem cases: Firefox tooltips, xfce4-taskmanager...
//

#include "window-manager.h"

int main(int numArgs, const char** args)
{
	const char* displayID = numArgs > 1? args[1] : ":0";
	
	wmSession desktop = wmGet(displayID);
	while(true) { desktop = wmEvents(desktop); }

	return 0;
}
