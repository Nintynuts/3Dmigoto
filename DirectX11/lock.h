#pragma once

#include <windows.h>
#include <vector>

// This version of EnterCriticalSection will use the function and line number
// in any lock stacks dumped when potential deadlock hazards are detected:
#define EnterCriticalSectionPretty(lock) \
	_EnterCriticalSectionPretty(lock, __FUNCTION__, __LINE__)
void _EnterCriticalSectionPretty(CRITICAL_SECTION *lock, char *function, int line);

// Use this when initialising a critical section in 3DMigoto to give it a nice
// name in lock stack dumps rather than using its address.
//
// **AVOID CALLING THIS FROM GLOBAL CONSTRUCTORS**
// https://yosefk.com/c++fqa/ctors.html#fqa-10.12
#define InitializeCriticalSectionPretty(lock) \
	_InitializeCriticalSectionPretty(lock, #lock)
void _InitializeCriticalSectionPretty(CRITICAL_SECTION *lock, char *lock_name);

void enable_lock_dependency_checks();
struct held_lock_info {
	CRITICAL_SECTION *lock;
	uintptr_t ret;
	size_t stack_hash;
	char *function;
	int line;
};
typedef std::vector<held_lock_info> LockStack;

// Everything in this struct has a unique copy per thread. It would be vastly
// simpler to just use the "thread_local" keyword, but MSDN warns that it can
// interfere with delay loading DLLs (without any detail as to what it means by
// that), so to err on the side of caution I'm using the old Win32 TLS API. We
// are using a structure to ensure we only consume a single TLS slot since they
// are limited, regardless of how many thread local variables we might want in
// the future. Use the below accessor function to get a pointer to this
// structure for the current thread.
struct TLS
{
	// This is set before calling into a DirectX function known to be
	// problematic if hooks are in use that can lead to one of our
	// functions being called unexpectedly if DirectX (or a third party
	// tool sitting between us and DirectX) has implemented the function we
	// are calling in terms of other hooked functions. We check if it is
	// set from any function known to be one called by DirectX and call
	// straight through to the original function if it is set.
	//
	// This is very much a band-aid solution to one of the fundamental
	// problems associated with hooking, but unfortunately hooking is a
	// reality we cannot avoid and in many cases a necessary evil to solve
	// certain problems. This is not a complete solution - it protects
	// against known cases where a function we call can manage to call back
	// into us, but does not protect against unknown cases of the same
	// problem, or cases where we call a function that has been hooked by a
	// third party tool (which we can use other strategies to avoid, such
	// as the unhookable UnhookableCreateDevice).
	bool hooking_quirk_protection;

	LockStack locks_held;

	TLS() :
		hooking_quirk_protection(false)
	{}
};

extern DWORD tls_idx;
static struct TLS* get_tls()
{
	TLS* tls;

	tls = (TLS*)TlsGetValue(tls_idx);
	if (!tls) {
		tls = new TLS();
		TlsSetValue(tls_idx, tls);
	}

	return tls;
}