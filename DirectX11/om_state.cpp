#include "om_state.h"

#include <log.h>
#include <lock.h>

#include <DbgHelp.h>
#include <ctime>

void save_om_state(ID3D11DeviceContext* context, struct OMState* state)
{
	int i;

	// OMGetRenderTargetAndUnorderedAccessViews is a poorly designed API as
	// to use it properly to get all RTVs and UAVs we need to pass it some
	// information that we don't know. So, we have to do a few extra steps
	// to find that info.

	context->OMGetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, state->rtvs, &state->dsv);

	state->NumRTVs = 0;
	for (i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; i++) {
		if (state->rtvs[i])
			state->NumRTVs = i + 1;
	}

	state->UAVStartSlot = state->NumRTVs;
	// Set NumUAVs to the max to retrieve them all now, and so that later
	// when rebinding them we will unbind any others that the command list
	// bound in the meantime
	state->NumUAVs = D3D11_PS_CS_UAV_REGISTER_COUNT - state->UAVStartSlot;

	// Finally get all the UAVs. Since we already retrieved the RTVs and
	// DSV we can skip getting them:
	context->OMGetRenderTargetsAndUnorderedAccessViews(0, NULL, NULL, state->UAVStartSlot, state->NumUAVs, state->uavs);
}

void restore_om_state(ID3D11DeviceContext* context, struct OMState* state)
{
	static const UINT uav_counts[D3D11_PS_CS_UAV_REGISTER_COUNT] = { (UINT)-1, (UINT)-1, (UINT)-1, (UINT)-1, (UINT)-1, (UINT)-1, (UINT)-1, (UINT)-1 };
	UINT i;

	context->OMSetRenderTargetsAndUnorderedAccessViews(state->NumRTVs, state->rtvs, state->dsv,
		state->UAVStartSlot, state->NumUAVs, state->uavs, uav_counts);

	for (i = 0; i < state->NumRTVs; i++) {
		if (state->rtvs[i])
			state->rtvs[i]->Release();
	}

	if (state->dsv)
		state->dsv->Release();

	for (i = 0; i < state->NumUAVs; i++) {
		if (state->uavs[i])
			state->uavs[i]->Release();
	}
}

static IDXGISwapChain* last_fullscreen_swap_chain;
static CRITICAL_SECTION crash_handler_lock;
static int crash_handler_level;

static DWORD WINAPI crash_handler_switch_to_window(_In_ LPVOID lpParameter)
{
	// Debugging is a pain in exclusive full screen, especially without a
	// second monitor attached (and even with one if you don't know about
	// the win+arrow or alt+space shortcuts you may be stuck - alt+tab to
	// the debugger, either win+down or alt+space and choose "Restore",
	// either win+left/right several times to move it to the other monitor
	// or alt+space again and choose "Move", press any arrow key to start
	// moving and *then* you can use the mouse to move the window to the
	// other monitor)... Try to switch to windowed mode to make our lives a
	// lot easier, but depending on the crash this might just hang (DirectX
	// might get stuck waiting on a lock or the window message queue might
	// not be pumping), so we do this in a new thread to allow the main
	// crash handler to continue responding to other keys:
	//
	// TODO: See if we can find a way to make this more reliable
	//
	if (last_fullscreen_swap_chain) {
		LogInfo("Attempting emergency switch to windowed mode on swap chain %p\n",
			last_fullscreen_swap_chain);

		last_fullscreen_swap_chain->SetFullscreenState(FALSE, NULL);
		//last_fullscreen_swap_chain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
	}

	if (LogFile)
		fflush(LogFile);

	return 0;
}

static LONG WINAPI migoto_exception_filter(_In_ struct _EXCEPTION_POINTERS* ExceptionInfo)
{
	wchar_t path[MAX_PATH];
	tm timestruct;
	time_t ltime;
	LONG ret = EXCEPTION_CONTINUE_EXECUTION;

	// SOS
	Beep(250, 100); Beep(250, 100); Beep(250, 100);
	Beep(200, 300); Beep(200, 200); Beep(200, 200);
	Beep(250, 100); Beep(250, 100); Beep(250, 100);

	// Before anything else, flush the log file and log exception info

	if (LogFile) {
		fflush(LogFile);

		LogInfo("\n\n ######################################\n"
			" ### 3DMigoto Crash Handler Invoked ###\n");

		int i = 0;
		for (auto record = ExceptionInfo->ExceptionRecord; record; record = record->ExceptionRecord, i++) {
			LogInfo(" ######################################\n"
				" ### Exception Record %i\n"
				" ###    ExceptionCode: 0x%08x\n"
				" ###   ExceptionFlags: 0x%08x\n"
				" ### ExceptionAddress: 0x%p\n"
				" ### NumberParameters: 0x%u\n"
				" ###",
				i,
				record->ExceptionCode,
				record->ExceptionFlags,
				record->ExceptionAddress,
				record->NumberParameters);
			for (unsigned j = 0; j < record->NumberParameters; j++)
				LogInfo(" %08Ix", record->ExceptionInformation[j]);
			LogInfo("\n");
		}

		fflush(LogFile);
	}

	// Next, write a minidump file so we can examine this in a debugger
	// later. Note that if the stack is corrupt there is some possibility
	// this could fail - if we really want a robust crash handler we could
	// bring in something like breakpad

	ltime = time(NULL);
	localtime_s(&timestruct, &ltime);
	wcsftime(path, MAX_PATH, L"3DM-%Y%m%d%H%M%S.dmp", &timestruct);

	// If multiple threads crash only allow one to write the crash dump and
	// the rest stop here:
	EnterCriticalSectionPretty(&crash_handler_lock);

	auto fp = CreateFile(path, GENERIC_WRITE, FILE_SHARE_READ,
		0, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fp != INVALID_HANDLE_VALUE) {
		LogInfo("Writing minidump to %S...\n", path);

		MINIDUMP_EXCEPTION_INFORMATION dump_info =
		{ GetCurrentThreadId(), ExceptionInfo, FALSE };

		if (MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
			fp, MiniDumpWithHandleData, &dump_info, NULL, NULL))
			LogInfo("Succeeded\n");
		else
			LogInfo("Failed :(\n");

		CloseHandle(fp);
	}
	else
		LogInfo("Error creating minidump file \"%S\": %d\n", path, GetLastError());

	if (LogFile)
		fflush(LogFile);

	// If crash is set to 2 instead of continuing we will stop and start
	// responding to various key bindings, sounding a reminder tone every
	// 5 seconds. All key bindings in this mode are prefixed with Ctrl+Alt
	// to prevent them being accidentally triggered.
	if (crash_handler_level == 2) {
		if (LogFile) {
			LogInfo("3DMigoto interactive crash handler invoked:\n");
			LogInfo(" Ctrl+Alt+Q: Quit (execute exception handler)\n");
			LogInfo(" Ctrl+Alt+K: Kill process\n");
			LogInfo(" Ctrl+Alt+C: Continue execution\n");
			LogInfo(" Ctrl+Alt+B: Break into the debugger (make sure one is attached)\n");
			LogInfo(" Ctrl+Alt+W: Attempt to switch to Windowed mode\n");
			LogInfo("\n");
			fflush(LogFile);
		}
		while (1) {
			Beep(500, 100);
			for (int i = 0; i < 50; i++) {
				Sleep(100);
				if (GetAsyncKeyState(VK_CONTROL) < 0 &&
					GetAsyncKeyState(VK_MENU) < 0) {
					if (GetAsyncKeyState('C') < 0) {
						LogInfo("Attempting to continue...\n"); fflush(LogFile); Beep(1000, 100);
						ret = EXCEPTION_CONTINUE_EXECUTION;
						goto unlock;
					}

					if (GetAsyncKeyState('Q') < 0) {
						LogInfo("Executing exception handler...\n"); fflush(LogFile); Beep(1000, 100);
						ret = EXCEPTION_EXECUTE_HANDLER;
						goto unlock;
					}

					if (GetAsyncKeyState('K') < 0) {
						LogInfo("Killing process...\n"); fflush(LogFile); Beep(1000, 100);
						ExitProcess(0x3D819070);
					}

					// TODO:
					// S = Suspend all other threads
					// R = Resume all other threads

					if (GetAsyncKeyState('B') < 0) {
						LogInfo("Dropping to debugger...\n"); fflush(LogFile); Beep(1000, 100);
						__debugbreak();
						goto unlock;
					}

					if (GetAsyncKeyState('W') < 0) {
						LogInfo("Attempting to switch to windowed mode...\n"); fflush(LogFile); Beep(1000, 100);
						CreateThread(NULL, 0, crash_handler_switch_to_window, NULL, 0, NULL);
						Sleep(1000);
					}
				}
			}
		}
	}

unlock:
	LeaveCriticalSection(&crash_handler_lock);

	return ret;
}

static DWORD WINAPI exception_keyboard_monitor(_In_ LPVOID lpParameter)
{
	while (1) {
		Sleep(1000);
		if (GetAsyncKeyState(VK_CONTROL) < 0 &&
			GetAsyncKeyState(VK_MENU) < 0 &&
			GetAsyncKeyState(VK_F11) < 0) {
			// User must be really committed to this to invoke the
			// crash handler, and this is a simple measure against
			// accidentally invoking it multiple times in a row:
			Sleep(3000);
			if (GetAsyncKeyState(VK_CONTROL) < 0 &&
				GetAsyncKeyState(VK_MENU) < 0 &&
				GetAsyncKeyState(VK_F11) < 0) {
				// Make sure 3DMigoto's exception handler is
				// still installed and trigger it:
				SetUnhandledExceptionFilter(migoto_exception_filter);
				RaiseException(0x3D819070, 0, 0, NULL);
			}
		}
	}

}

void install_crash_handler(int level)
{
	LPTOP_LEVEL_EXCEPTION_FILTER old_handler;
	UINT old_mode;

	crash_handler_level = level;

	old_handler = SetUnhandledExceptionFilter(migoto_exception_filter);
	// TODO: Call set_terminate() on every thread to catch unhandled C++
	// exceptions as well

	if (old_handler == migoto_exception_filter) {
		LogInfo("  > 3DMigoto crash handler already installed\n");
		return;
	}

	InitializeCriticalSectionPretty(&crash_handler_lock);

	old_mode = SetErrorMode(SEM_FAILCRITICALERRORS);

	LogInfo("  > Installed 3DMigoto crash handler, previous exception filter: %p, previous error mode: %x\n",
		old_handler, old_mode);

	// Spawn a thread to monitor for a keyboard salute to trigger the
	// exception handler in the event of a hang/deadlock:
	CreateThread(NULL, 0, exception_keyboard_monitor, NULL, 0, NULL);
}