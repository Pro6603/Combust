#include "select_window.h"
#include <vector>
#include <string>
#include <iostream>
#include <TlHelp32.h>

// Helper struct
struct WindowInfo {
    HWND hwnd;
    std::wstring title;
};

// Check if process name matches
bool IsJavawProcess(DWORD procId) {
    bool result = false;
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE)
        return false;

    PROCESSENTRY32W entry;
    entry.dwSize = sizeof(PROCESSENTRY32W);

    if (Process32FirstW(snap, &entry)) {
        do {
            if (entry.th32ProcessID == procId) {
                std::wstring name(entry.szExeFile);
                if (_wcsicmp(name.c_str(), L"javaw.exe") == 0)
                    result = true;
                break;
            }
        } while (Process32NextW(snap, &entry));
    }
    CloseHandle(snap);
    return result;
}

// EnumWindows callback to gather Java windows
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    DWORD procId = 0;
    GetWindowThreadProcessId(hwnd, &procId);
    if (IsJavawProcess(procId) && IsWindowVisible(hwnd)) {
        int length = GetWindowTextLengthW(hwnd);
        if (length > 0) {
            std::wstring title(length + 1, L'\0');
            GetWindowTextW(hwnd, &title[0], length + 1);
            title.resize(length);
            if (!title.empty()) {
                std::vector<WindowInfo>* pList = (std::vector<WindowInfo>*)lParam;
                pList->push_back({ hwnd, title });
            }
        }
    }
    return TRUE;
}

HWND SelectWindow()
{
    AllocConsole();
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONIN$", "r", stdin);

    std::vector<WindowInfo> windows;
    EnumWindows(EnumWindowsProc, (LPARAM)&windows);

    std::wcout << L"Select window to hook:\n";
    for (size_t i = 0; i < windows.size(); i++) {
        std::wcout << i + 1 << L") " << windows[i].title << L"\n";
    }

    std::wcout << L"Enter choice: ";
    int choice = 0;
    std::wcin >> choice;

    HWND selectedHwnd = NULL;
    if (choice > 0 && choice <= (int)windows.size()) {
        selectedHwnd = windows[choice - 1].hwnd;
        std::wcout << L"Selected: " << windows[choice - 1].title << L"\n";
    }
    else {
        std::wcout << L"Invalid choice.\n";
    }

    Sleep(1000); // pause so user can read confirmation

    fclose(fp);
    FreeConsole(); // close console immediately after selection

    return selectedHwnd;
}


