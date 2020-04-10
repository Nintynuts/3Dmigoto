#include "overlay.h"
#include "log.h"

#include "globals_common.h"

// The active shader will show where we are in each list. / 0 / 0 will mean that we are not 
// actively searching. 

void AppendShaderText(wchar_t* fullLine, wchar_t* type, int pos, size_t size)
{
	if (size == 0)
		return;

	// The position is zero based, so we'll make it +1 for the humans.
	if (++pos == 0)
		size = 0;

	// Format: "VS:1/15"
	wchar_t append[maxstring];
	swprintf_s(append, maxstring, L"%ls:%d/%Iu ", type, pos, size);
	wcscat_s(fullLine, maxstring, append);
}

// Side note: Not really stoked with C++ string handling.  There are like 4 or
// 5 different ways to do things, all partly compatible, none a clear winner in
// terms of simplicity and clarity.  Generally speaking we'd want to use C++
// wstring and string, but there are no good output formatters.  Maybe the 
// newer iostream based pieces, but we'd still need to convert.
//
// The philosophy here and in other files, is to use whatever the API that we
// are using wants.  In this case it's a wchar_t* for DrawString, so we'll not
// do a lot of conversions and different formats, we'll just use wchar_t and its
// formatters.
//
// In particular, we also want to avoid 5 different libraries for string handling,
// Microsoft has way too many variants.  We'll use the regular C library from
// the standard c runtime, but use the _s safe versions.

void ClearNotices()
{
	int level;

	if (notice_cleared_frame == GB->frame_no)
		return;

	EnterCriticalSectionPretty(&notices.lock);

	for (level = 0; level < NUM_LOG_LEVELS; level++)
		notices.notices[level].clear();

	notice_cleared_frame = GB->frame_no;
	has_notice = false;

	LeaveCriticalSection(&notices.lock);
}

void LogOverlayW(LogLevel level, const wchar_t* fmt, ...)
{
	wchar_t msg[maxstring];
	va_list ap;

	va_start(ap, fmt);
	vLogInfoW(fmt, ap);

	// Using _vsnwprintf_s so we don't crash if the message is too long for
	// the buffer, and truncate it instead - unless we can automatically
	// wrap the message, which DirectXTK doesn't appear to support, who
	// cares if it gets cut off somewhere off screen anyway?
	_vsnwprintf_s(msg, maxstring, _TRUNCATE, fmt, ap);

	EnterCriticalSectionPretty(&notices.lock);

	notices.notices[level].emplace_back(msg);
	has_notice = true;

	LeaveCriticalSection(&notices.lock);

	va_end(ap);
}