#pragma once

#include "lock.h"

#include <Windows.h>
#include <DirectXMath.h>
#include <string>
#include <memory>

static bool has_notice = false;
static unsigned notice_cleared_frame = 0;

// Max expected on-screen string size, used for buffer safe calls.
const int maxstring = 1024;

enum LogLevel {
	LOG_DIRE,
	LOG_WARNING,
	LOG_WARNING_MONOSPACE,
	LOG_NOTICE,
	LOG_INFO,

	NUM_LOG_LEVELS
};

class OverlayNotice {
public:
	std::wstring message;
	DWORD timestamp;

	OverlayNotice(std::wstring message);
};

#define MAX_SIMULTANEOUS_NOTICES 10

static class Notices
{
public:
	std::vector<OverlayNotice> notices[NUM_LOG_LEVELS];
	CRITICAL_SECTION lock;

	Notices()
	{
		InitializeCriticalSectionPretty(&lock);
	}

	~Notices()
	{
		DeleteCriticalSection(&lock);
	}
} notices;

template<typename FONT>
class OverlayBase {

protected:
	DirectX::XMUINT2 mResolution;

	virtual void SaveState() = 0;
	virtual void RestoreState() = 0;

	virtual void DrawShaderInfoLines(float* y) = 0;
	virtual void DrawNotices(float* y) = 0;
	virtual void DrawProfiling(float* y) = 0;
	virtual void DrawRectangle(float x, float y, float w, float h, float r, float g, float b, float opacity) = 0;

public:
	std::unique_ptr<FONT> mFont;
	std::unique_ptr<FONT> mFontNotifications;
	std::unique_ptr<FONT> mFontProfiling;
};

template<typename COLOUR, typename FONT>
struct LogLevelParams {
	COLOUR colour;
	DWORD duration;
	bool hide_in_release;
	std::unique_ptr<FONT> OverlayBase<FONT>::* font;
};

void ClearNotices();
void LogOverlayW(LogLevel level, const wchar_t* fmt, ...);
void LogOverlay(LogLevel level, const char* fmt, ...);
void AppendShaderText(wchar_t* fullLine, wchar_t* type, int pos, size_t size);