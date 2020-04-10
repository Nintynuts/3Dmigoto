#pragma once
#include <Windows.h>
#include <stdio.h>
#include <string>

BOOL CreateDirectoryEnsuringAccess(LPCWSTR path);
errno_t wfopen_ensuring_access(FILE** pFile, const wchar_t *filename, const wchar_t *mode);
void set_file_last_write_time(wchar_t *path, FILETIME* ftWrite, DWORD flags = 0);
void touch_file(wchar_t *path, DWORD flags = 0);
#define touch_dir(path) touch_file(path, FILE_FLAG_BACKUP_SEMANTICS)