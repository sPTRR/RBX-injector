#include <windows.h>
#include <vector>
#include <thread>
#include <iostream>

void ModifyFunc(HANDLE process, LPVOID addr, const std::vector<BYTE>& newBytes, std::vector<BYTE>& origBytes) {
  
    SIZE_T bytesRead;
    origBytes.resize(newBytes.size());
    ReadProcessMemory(process, addr, origBytes.data(), origBytes.size(), &bytesRead);

 
    WriteProcessMemory(process, addr, newBytes.data(), newBytes.size(), nullptr);
}

DWORD GetProcId(const char* title) {
    HWND hwnd;
    while (!(hwnd = FindWindowA(nullptr, title))) std::this_thread::sleep_for(std::chrono::seconds(1));
    DWORD pid;
    GetWindowThreadProcessId(hwnd, &pid);
    return pid;
}

int main() {
// credit to https://www.unknowncheats.me/forum/anti-cheat-bypass/682751-updated-roblox-hyperion-bypass-payload-fixed.html#google_vignette forked it just updated for hyp v4.4
    DWORD pid = GetProcId("Roblox");
    HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!process) {
        std::cerr << "Failed to open process!" << std::endl;
        return EXIT_FAILURE;
    }

   
    LPVOID targetAddr = (LPVOID)0x12345678;  // Replace with Roblox base address or entry point voided

 // u can get base address using a driver bridge but dont use a driver rpm/wpm to prevent detections
    std::vector<BYTE> hook = { 0x48, 0x31, 0xC0, 0x59, 0xFF, 0xE1 };
    std::vector<BYTE> orig(6);  

    ModifyFunc(process, targetAddr, hook, orig);  

  
    HMODULE dll = LoadLibraryExA("MessageBox.dll", nullptr, DONT_RESOLVE_DLL_REFERENCES);
    if (!dll) {
        std::cerr << "Failed to load DLL!" << std::endl;
        return EXIT_FAILURE;
    }

    FARPROC Callback = GetProcAddress(dll, "callback");
    if (!Callback) {
        std::cerr << "Failed to find callback function in DLL!" << std::endl;
        return EXIT_FAILURE;
    }

  
    DWORD tid = GetWindowThreadProcessId(FindWindowA(nullptr, "Roblox"), nullptr);
    HHOOK HookHandle = SetWindowsHookExA(WH_GETMESSAGE, (HOOKPROC)Callback, dll, tid);
    if (!HookHandle) {
        std::cerr << "Failed to set hook!" << std::endl;
        return EXIT_FAILURE;
    }


    PostThreadMessageA(tid, WM_NULL, 0, 0);

  
    std::cin.get();

    
    UnhookWindowsHookEx(HookHandle);
    WriteProcessMemory(process, targetAddr, orig.data(), orig.size(), nullptr);

    return EXIT_SUCCESS;
}
