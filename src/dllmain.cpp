// dllmain.cpp
#include "dllmain.h"
#include <thread> // For std::this_thread::sleep_for

// A single, shared ModuleManager instance
static ModuleManager g_moduleManager;

static void thread_func() {
    // 1) Get base address
    uintptr_t base = reinterpret_cast<uintptr_t>(GetModuleHandleA(nullptr));

    // 2) Bring up the console IMMEDIATELY with focus
    if (AllocConsole()) {
        // Make console window visible and focused
        ShowWindow(GetConsoleWindow(), SW_SHOW);
        SetForegroundWindow(GetConsoleWindow());

        // Redirect standard handles
        FILE* fDummy;
        if (freopen_s(&fDummy, "CONOUT$", "w", stdout) != 0) {
            OutputDebugStringA("Failed to redirect stdout!\n");
        }
        if (freopen_s(&fDummy, "CONOUT$", "w", stderr) != 0) {
            OutputDebugStringA("Failed to redirect stderr!\n");
        }
        
        // Ensure stream buffers are cleared
        std::cout.clear();
        std::cerr.clear();
        std::clog.clear();

        // Direct WinAPI console output as fallback
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hConsole != INVALID_HANDLE_VALUE) {
            const char* initMsg = "[+] Console initialized via WinAPI\n";
            WriteConsoleA(hConsole, initMsg, strlen(initMsg), nullptr, nullptr);
        }
    } else {
        OutputDebugStringA("AllocConsole failed! Using debug output.\n");
    }

    // 3) Always output to both console and debugger
    auto DualOutput = [](const char* message) {
        // Console output
        printf("%s", message);
        std::fflush(stdout);
        
        // Debugger output
        OutputDebugStringA(message);
    };

    DualOutput("[+] Starting initialization...\n");

    try {
        // 4) Enable CcpBlocker
        DualOutput("[+] Enabling CcpBlocker...\n");
        g_moduleManager.enable(std::make_unique<CcpBlocker>(base));

        std::this_thread::sleep_for(std::chrono::seconds(14));

        // 5) Disable VMProtect
        DualOutput("[+] Disabling VMProtect...\n");
        VMPDisable();

        // 6) Enable other modules
        DualOutput("[+] Enabling HttpModule...\n");
        g_moduleManager.enable(std::make_unique<HttpModule>(base));

        DualOutput("[+] Enabling SecurityModule...\n");
        g_moduleManager.enable(std::make_unique<SecurityModule>(base));

        DualOutput("[+] Enabling MiscModule...\n");
        g_moduleManager.enable(std::make_unique<MiscModule>(base));

        DualOutput("[+] Initialization complete!\n");
    }
    catch (const std::exception& e) {
        std::string error = "[!] Exception: " + std::string(e.what()) + "\n";
        OutputDebugStringA(error.c_str());
    }
    catch (...) {
        OutputDebugStringA("[!] Unknown exception occurred!\n");
    }
}
// Standard DLL entry point
BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        // Avoid DLL_THREAD_ATTACH/DLL_THREAD_DETACH notifications –
        // we don't need them and it improves performance.
        DisableThreadLibraryCalls(hinst);

        // Launch our worker thread
        std::thread(thread_func).detach();
    }
    return TRUE;
}
