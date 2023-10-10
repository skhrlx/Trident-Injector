#include <Windows.h>
#include <iostream>
#include <Shellapi.h>

int main() {
    // Path to your DLL file
    const wchar_t* dllPath = L"C:\\Users\\a\\source\\repos\\Trident-GO-Reborn\\x64\\Release\\Trident.dll";

    // Start notepad.exe as an administrator
    SHELLEXECUTEINFO sei = { sizeof(SHELLEXECUTEINFO) };
    sei.lpVerb = L"runas"; // This will request elevation to admin
    sei.lpFile = L"notepad.exe";
    sei.nShow = SW_SHOWNORMAL;
    if (!ShellExecuteEx(&sei)) {
        std::cout << "Failed to start notepad.exe as an administrator." << std::endl;
        return 1;
    }

    // Wait for notepad.exe to initialize
    Sleep(2000);

    // Get a handle to the target process (notepad.exe)
    HWND targetWnd = FindWindow(NULL, L"Untitled - Notepad");
    DWORD targetProcessId;
    GetWindowThreadProcessId(targetWnd, &targetProcessId);

    if (targetProcessId == 0) {
        std::cout << "Notepad.exe not found. Make sure it's running." << std::endl;
        return 1;
    }

    // Open the target process
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, targetProcessId);
    if (!hProcess) {
        std::cout << "Failed to open the target process." << std::endl;
        return 1;
    }

    // Allocate memory in the target process for the DLL path
    LPVOID dllPathAddr = VirtualAllocEx(hProcess, NULL, wcslen(dllPath) * sizeof(wchar_t), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    // Write the DLL path into the target process
    WriteProcessMemory(hProcess, dllPathAddr, dllPath, wcslen(dllPath) * sizeof(wchar_t), NULL);

    // Get the address of the LoadLibraryW function
    LPVOID loadLibraryAddr = (LPVOID)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryW");

    // Create a remote thread in the target process to load the DLL
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)loadLibraryAddr, dllPathAddr, 0, NULL);
    if (!hThread) {
        std::cout << "Failed to create a remote thread." << std::endl;
        return 1;
    }

    // Wait for the remote thread to finish
    WaitForSingleObject(hThread, INFINITE);

    // Cleanup
    VirtualFreeEx(hProcess, dllPathAddr, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);

    std::cout << "DLL injected successfully!" << std::endl;
    return 0;
}
