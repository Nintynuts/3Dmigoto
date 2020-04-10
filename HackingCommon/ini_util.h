#pragma once

#include <Windows.h>
#include <wchar.h>
#include <string>

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

// Parses the name of one of the IniParam constants: x, y, z, w, x1, y1, ..., z7, w7
static bool ParseIniParamName(const wchar_t* name, int* idx, float DirectX::XMFLOAT4::** component)
{
	int ret, len1, len2;
	wchar_t component_chr;
	size_t length = wcslen(name);

	ret = swscanf_s(name, L"%lc%n%u%n", &component_chr, 1, &len1, idx, &len2);

	// May or may not have matched index. Make sure entire string was
	// matched either way and check index is valid if it was matched:
	if (ret == 1 && len1 == length) {
		*idx = 0;
	}
	else if (ret == 2 && len2 == length) {
#if MIGOTO_DX == 9
		// Added gating for this DX9 specific limitation that we definitely do
		// not want to enforce in DX11 as that would break a bunch of mods -DSS
		if (*idx >= 225)
			return false;
#endif // MIGOTO_DX == 9
	}
	else {
		return false;
	}

	switch (towlower(component_chr)) {
	case L'x':
		*component = &DirectX::XMFLOAT4::x;
		return true;
	case L'y':
		*component = &DirectX::XMFLOAT4::y;
		return true;
	case L'z':
		*component = &DirectX::XMFLOAT4::z;
		return true;
	case L'w':
		*component = &DirectX::XMFLOAT4::w;
		return true;
	}

	return false;
}