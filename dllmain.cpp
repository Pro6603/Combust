// this file is responsible for hooking the target windows wndproc and setting up the opengl hook (the injection basically)

#include <windows.h>
#include "globals.h"
#include "opengl_hook.h"
#include "select_window.h"
#include <imgui.h>


LRESULT CALLBACK HookedWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//globals
HWND g_targetWindow = nullptr;
WNDPROC g_originalWndProc = nullptr;
extern bool g_showUI;
extern bool g_unloadRequested;

LRESULT CALLBACK HookedWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (g_showUI)
    {
        if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
            return true;

        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureMouse || io.WantCaptureKeyboard)
            return true;
    }

    return CallWindowProc(g_originalWndProc, hwnd, uMsg, wParam, lParam);
}

DWORD WINAPI MainThread(LPVOID lpParam)
{
    HMODULE hModule = (HMODULE)lpParam;

    //g_targetWindow = SelectWindow(); // select window via a cmd windoww
	g_targetWindow = FindWindow(nullptr, L"NoRiskClient 1.21-2.0.36 (20250204) | Minecraft 1.21");


    if (!g_targetWindow)
    {
        MessageBox(nullptr, L"Failed to find target window!", L"Error", MB_OK);
        return 1;
    }

    //hook wnd proc
    g_originalWndProc = (WNDPROC)SetWindowLongPtr(g_targetWindow, GWLP_WNDPROC, (LONG_PTR)HookedWndProc);
    if (g_originalWndProc == nullptr)
    {
        DWORD err = GetLastError();
        wchar_t errMsg[256];
        wsprintf(errMsg, L"Failed to hook WndProc! Error: %lu", err);
        MessageBox(nullptr, errMsg, L"Error", MB_OK);
        return 1;
    }

    //hook setup
    SetupHook();

    while (!g_unloadRequested)
    {
        Sleep(100);
    }

    if (g_originalWndProc)
        SetWindowLongPtr(g_targetWindow, GWLP_WNDPROC, (LONG_PTR)g_originalWndProc);

    RemoveHook();

    FreeLibraryAndExitThread(hModule, 0);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);  //avoid deadlocks
        CreateThread(nullptr, 0, MainThread, hModule, 0, nullptr);
        break;

    case DLL_PROCESS_DETACH:
        RemoveHook();
        break;
    }
    return TRUE;
}
