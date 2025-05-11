#include <Util.h>
#include <Windows.h>
#include <cstdint>
#include <iostream>

HMODULE ntdllAddr = nullptr;

// Check if running under Wine
static bool isWine() {
    if (!ntdllAddr) {
        ntdllAddr = GetModuleHandleW(L"ntdll.dll");
    }
    return GetProcAddress(ntdllAddr, "wine_get_version") != nullptr;
}

// Correctly reconstruct syscall stub from Rust implementation
void VMPDisable() {
    std::printf("[VMPDisable] Starting...\n");

    // Get ntdll handle (using global variable)
    ntdllAddr = GetModuleHandleW(L"ntdll.dll");
    if (!ntdllAddr) {
        std::printf("[VMPDisable] Failed to get ntdll handle.\n");
        return;
    }
    std::printf("[VMPDisable] ntdll handle: %p\n", ntdllAddr);

    // Get NtProtectVirtualMemory address
    auto procAddr = GetProcAddress(ntdllAddr, "NtProtectVirtualMemory");
    if (!procAddr) {
        std::printf("[VMPDisable] Failed to get NtProtectVirtualMemory address.\n");
        return;
    }
    std::printf("[VMPDisable] NtProtectVirtualMemory address: %p\n", procAddr);

    // Choose correct syscall routine
    const char* targetRoutine = isWine() ? "NtPulseEvent" : "NtQuerySection";
    auto g_routine = GetProcAddress(ntdllAddr, targetRoutine);
    if (!g_routine) {
        std::printf("[VMPDisable] Failed to get %s address.\n", targetRoutine);
        return;
    }
    std::printf("[VMPDisable] %s address: %p\n", targetRoutine, g_routine);

    // Unprotect memory for patching
    DWORD oldProtect;
    if (!VirtualProtect(procAddr, 8, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        std::printf("[VMPDisable] VirtualProtect failed (error: %lu).\n", GetLastError());
        return;
    }
    std::printf("[VMPDisable] Memory unprotected (oldProtect: 0x%X).\n", oldProtect);

    // Reconstruct syscall stub as in Rust
    uint32_t lower_bits = *reinterpret_cast<uint32_t*>(g_routine);
    uint32_t upper_bits_offset = *reinterpret_cast<uint32_t*>(
        reinterpret_cast<uintptr_t>(g_routine) + 4
    );
    uint64_t upper_bits = (static_cast<uint64_t>(upper_bits_offset) - 1) << 32;
    uint64_t syscall_stub = static_cast<uint64_t>(lower_bits) | upper_bits;

    std::printf("[VMPDisable] Syscall stub value: 0x%llX\n", syscall_stub);
    
    // Apply patch
    *reinterpret_cast<uint64_t*>(procAddr) = syscall_stub;

    // Restore protection
    VirtualProtect(procAddr, 8, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), procAddr, 8);

    std::printf("[VMPDisable] Completed successfully.\n");
}
