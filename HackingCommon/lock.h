#pragma once

#include <windows.h>
#include <vector>

// This version of EnterCriticalSection will use the function and line number
// in any lock stacks dumped when potential deadlock hazards are detected:
#define EnterCriticalSectionPretty(lock) \
	_EnterCriticalSectionPretty(lock, __FUNCTION__, __LINE__)
void _EnterCriticalSectionPretty(CRITICAL_SECTION *lock, const char *function, int line);

// Use this when initialising a critical section in 3DMigoto to give it a nice
// name in lock stack dumps rather than using its address.
//
// **AVOID CALLING THIS FROM GLOBAL CONSTRUCTORS**
// https://yosefk.com/c++fqa/ctors.html#fqa-10.12
#define InitializeCriticalSectionPretty(lock) \
	_InitializeCriticalSectionPretty(lock, #lock)
void _InitializeCriticalSectionPretty(CRITICAL_SECTION *lock, const char *lock_name);

#define LockResourceCreationMode() \
	EnterCriticalSectionPretty(&resource_creation_mode_lock)

#define UnlockResourceCreationMode() \
	LeaveCriticalSection(&resource_creation_mode_lock)

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

void enable_lock_dependency_checks();
struct held_lock_info {
	CRITICAL_SECTION *lock;
	uintptr_t ret;
	size_t stack_hash;
	const char *function;
	int line;
};
typedef std::vector<held_lock_info> LockStack;
