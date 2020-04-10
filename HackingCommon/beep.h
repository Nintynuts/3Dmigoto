#pragma once

#include <windows.h>
#include "log.h"

static void BeepSuccess()
{
	// High beep for success
	Beep(1800, 400);
}

static void BeepShort()
{
	// Short High beep
	Beep(1800, 100);
}

static void BeepFailure()
{
	// Bonk sound for failure.
	Beep(200, 150);
}

static void BeepFailure2()
{
	// Brnk, dunk sound for failure.
	Beep(300, 200); Beep(200, 150);
}

static void BeepProfileFail()
{
	// Brnk, du-du-dunk sound to signify the profile failed to install.
	// This is more likely to hit than the others for an end user (e.g. if
	// they denied admin privileges), so use a unique tone to make it
	// easier to identify.
	Beep(300, 300);
	Beep(200, 100);
	Beep(200, 100);
	Beep(200, 200);
}

static DECLSPEC_NORETURN void DoubleBeepExit()
{
	// Fatal error somewhere, known to crash, might as well exit cleanly
	// with some notification.
	BeepFailure2();
	Sleep(500);
	BeepFailure2();
	Sleep(200);
	if (LogFile) {
		// Make sure the log is written out so we see the failure message
		fclose(LogFile);
		LogFile = 0;
	}
	ExitProcess(0xc0000135);
}