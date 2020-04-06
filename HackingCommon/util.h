#pragma once

#include <Windows.h>
#include <ctype.h>
#include <wchar.h>
#include <string>
#include <vector>
#include <typeinfo>

#include "log.h"

// -----------------------------------------------------------------------------------------------

// This critical section must be held to avoid race conditions when creating
// any resource. The nvapi functions used to set the resource creation mode
// affect global state, so if multiple threads are creating resources
// simultaneously it is possible for a StereoMode override or stereo/mono copy
// on one thread to affect another. This should be taken before setting the
// surface creation mode and released only after it has been restored. If the
// creation mode is not being set it should still be taken around the actual
// CreateXXX call.
//
// The actual variable definition is in the DX11 project to remind anyone using
// this from another project that they need to InitializeCriticalSection[Pretty]
extern CRITICAL_SECTION resource_creation_mode_lock;

// Use the pretty lock debugging version if lock.h is included first, otherwise
// use the regular EnterCriticalSection:
#ifdef EnterCriticalSectionPretty
#define LockResourceCreationMode() \
	EnterCriticalSectionPretty(&resource_creation_mode_lock)
#else
#define LockResourceCreationMode() \
	EnterCriticalSection(&resource_creation_mode_lock)
#endif

#define UnlockResourceCreationMode() \
	LeaveCriticalSection(&resource_creation_mode_lock)

// -----------------------------------------------------------------------------------------------

// Strip spaces from the right of a string.
// Returns a pointer to the last non-NULL character of the truncated string.
static char* RightStripA(char* buf)
{
	char* end = buf + strlen(buf) - 1;
	while (end > buf && isspace(*end))
		end--;
	*(end + 1) = 0;
	return end;
}
static wchar_t* RightStripW(wchar_t* buf)
{
	wchar_t* end = buf + wcslen(buf) - 1;
	while (end > buf && iswspace(*end))
		end--;
	*(end + 1) = 0;
	return end;
}

static char* readStringParameter(wchar_t* val)
{
	static char buf[MAX_PATH];
	wcstombs(buf, val, MAX_PATH);
	RightStripA(buf);
	char* start = buf; while (isspace(*start)) start++;
	return start;
}

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


// -----------------------------------------------------------------------------------------------

// Primary hash calculation for all shader file names.

// 64 bit magic FNV-0 and FNV-1 prime
#define FNV_64_PRIME ((UINT64)0x100000001b3ULL)
static UINT64 fnv_64_buf(const void* buf, size_t len)
{
	UINT64 hval = 0;
	unsigned const char* bp = (unsigned const char*)buf;	/* start of buffer */
	unsigned const char* be = bp + len;		/* beyond end of buffer */

	// FNV-1 hash each octet of the buffer
	while (bp < be)
	{
		// multiply by the 64 bit FNV magic prime mod 2^64 */
		hval *= FNV_64_PRIME;
		// xor the bottom with the current octet
		hval ^= (UINT64)*bp++;
	}
	return hval;
}