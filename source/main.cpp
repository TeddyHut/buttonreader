#include <3ds.h>
#include <base/eg_engine.h>
#include "../include/main/main_controller.h"

int main()
{
	gfxInitDefault();
	//gfxSet3D(true); // uncomment if using stereoscopic 3D
	// Main loop
	Main::Controller_c main_controller;
	Main::Rundata rundata;
	PrintConsole top;
	PrintConsole bottom;
	consoleInit(GFX_TOP, &top);
	consoleInit(GFX_BOTTOM, &bottom);
	consoleSelect(&bottom);
	rundata.top = &top;
	rundata.bottom = &bottom;
	rundata.stop = false;
	svcCreateMutex(&(rundata.console_mutex), false);
	while (aptMainLoop())
	{
		gspWaitForVBlank();
		hidScanInput();

		// Your code goes here
		main_controller.runover_controller(&rundata);
		main_controller.runover_dataManipulator(&rundata);
		main_controller.runover_input(&rundata);
		main_controller.runover_model(&rundata);
		main_controller.runover_view(&rundata);

		if (rundata.stop)
			break;

		// Flush and swap framebuffers
		gfxFlushBuffers();
		gfxSwapBuffers();
	}
	svcCloseHandle(rundata.console_mutex);
	gfxExit();
	return 0;
}