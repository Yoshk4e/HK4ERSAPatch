#pragma once

#include <ModuleManager.hpp>
#include <Interceptor.hpp>
#include <Marshal.hpp>
#include <windows.h>
#include <common.h>
#include <vector>
#include <string>
#include <stdexcept>
#include <cstdint>
#include <codecvt>
#include <locale>
#include <cstdlib>
#include <algorithm>
#include <thread>
#include <chrono>
#include <cstdio>

constexpr std::uintptr_t WEB_REQUEST_UTILS_MAKE_INITIAL_URL = 0x103B93B0;
constexpr std::uintptr_t BROWSER_LOAD_URL                   = 0x4F71030;

// IL2CPP structures
typedef uint16_t Il2CppChar;

typedef struct Il2CppObject {
    void* klass;
    void* monitor;
} Il2CppObject;

typedef struct Il2CppString {
    Il2CppObject object;
    int32_t length;
    Il2CppChar chars[1];
} Il2CppString;

class HttpModule : public IMhyModule {
public:
    HttpModule(std::uintptr_t base)
        : assemblyBase(ValidateBaseAddress(base)) {
        printf("[HttpModule] Base: 0x%p\n", (void*)assemblyBase);
    }

    void init() override {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        auto& interceptor = Interceptor::Get();

        // Hook MakeInitialURL
        try {
            origMakeInitialURL = interceptor.Attach<FnMakeInitialURL>(
                assemblyBase + WEB_REQUEST_UTILS_MAKE_INITIAL_URL,
                &HttpModule::on_make_initial_url
            );
            printf("[HttpModule] [SUCCESS] MakeInitialURL hooked, trampoline at %p\n",
                   reinterpret_cast<void*>(origMakeInitialURL));
        }
        catch (const std::exception& e) {
            printf("[HttpModule] [FAIL] Failed to hook MakeInitialURL: %s\n", e.what());
        }

        // Hook BrowserLoadURL
        try {
            origBrowserLoadURL = interceptor.Attach<FnBrowserLoadURL>(
                assemblyBase + BROWSER_LOAD_URL,
                &HttpModule::on_browser_load_url
            );
            printf("[HttpModule] [SUCCESS] BrowserLoadURL hooked, trampoline at %p\n",
                   reinterpret_cast<void*>(origBrowserLoadURL));
        }
        catch (const std::exception& e) {
            printf("[HttpModule] [FAIL] Failed to hook BrowserLoadURL: %s\n", e.what());
        }
    }

    void deinit() override {
        // Managed by Interceptor
    }

    ModuleType getModuleType() const override {
        return ModuleType::Http;
    }

private:
    std::uintptr_t assemblyBase;

    using FnMakeInitialURL = void(__fastcall*)(Registers*, std::uintptr_t);
    using FnBrowserLoadURL = void(__fastcall*)(Registers*, std::uintptr_t);

    static inline FnMakeInitialURL origMakeInitialURL = nullptr;
    static inline FnBrowserLoadURL origBrowserLoadURL = nullptr;

    static std::uintptr_t ValidateBaseAddress(std::uintptr_t base) {
        if (base == 0 || IsBadReadPtr(reinterpret_cast<void*>(base), sizeof(void*))) {
            return reinterpret_cast<std::uintptr_t>(GetModuleHandleA(nullptr));
        }
        return base;
    }

    static std::wstring build_redirect_url(const std::wstring& url, bool is_browser) {
        std::wstring new_base = (url.find(L"/query_region_list") != std::wstring::npos)
            ? L"http://127.0.0.1:21041"
            : L"http://127.0.0.1:21000";

        size_t pos = url.find(L'/');
        for (int i = 0; i < 3 && pos != std::wstring::npos; ++i)
            pos = url.find(L'/', pos + 1);

        if (pos != std::wstring::npos)
            new_base += url.substr(pos);

        return new_base;
    }

    static void process_url(Registers* regs, bool is_browser) {
        if (!regs || IsBadReadPtr(regs, sizeof(Registers))) return;

        const auto ptrReg = is_browser ? regs->rdx : regs->rcx;
        if (IsBadReadPtr(reinterpret_cast<void*>(ptrReg), sizeof(uintptr_t))) return;

        auto length_ptr = reinterpret_cast<uint32_t*>(ptrReg + 16);
        auto string_ptr = reinterpret_cast<wchar_t*>(ptrReg + 20);
        if (!length_ptr || !string_ptr || *length_ptr == 0) return;

        const std::wstring original_url(string_ptr, *length_ptr);
        std::wstring new_url = build_redirect_url(original_url, is_browser);

        std::wstring logMsg = L"[HttpModule] Redirecting from: " +
            original_url + L" to: " + new_url + L"\n";
        OutputDebugStringW(logMsg.c_str());

        if (auto managed_str = Marshal::WideToGameString(new_url)) {
            if (is_browser)
                regs->rdx = reinterpret_cast<uint64_t>(managed_str);
            else
                regs->rcx = reinterpret_cast<uint64_t>(managed_str);
        }
    }

    static void log_game_string(const char* label, Il2CppString* str) {
        if (!str || IsBadReadPtr(str, sizeof(Il2CppString))) {
            printf("[HttpModule] [FAIL] %s string pointer is invalid\n", label);
            return;
        }

        int32_t len = str->length;
        if (len <= 0 || len > 2048) {
            printf("[HttpModule] [FAIL] %s string length is invalid: %d\n", label, len);
            return;
        }

        if (IsBadReadPtr(str->chars, len * sizeof(Il2CppChar))) {
            printf("[HttpModule] [FAIL] %s string chars pointer is invalid\n", label);
            return;
        }

        std::wstring w(str->chars, str->chars + len);
        int size = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (size > 0) {
            std::string utf8(size, 0);
            WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, utf8.data(), size, nullptr, nullptr);
            printf("[HttpModule] [SUCCESS] %s URL = \"%s\"\n", label, utf8.c_str());
        }
    }

    static void __fastcall on_make_initial_url(Registers* regs, std::uintptr_t originalLocalUrl) {
        printf("[HttpModule] [SUCCESS] on_make_initial_url fired (localUrlPtr=%p)\n",
               reinterpret_cast<void*>(originalLocalUrl));

        log_game_string("Original", reinterpret_cast<Il2CppString*>(originalLocalUrl));

        if (origMakeInitialURL) {
            process_url(regs, false);
            origMakeInitialURL(regs, originalLocalUrl);
        }
    }

    static void __fastcall on_browser_load_url(Registers* regs, std::uintptr_t originalLocalUrl) {
        printf("[HttpModule] [SUCCESS] on_browser_load_url fired (localUrlPtr=%p)\n",
               reinterpret_cast<void*>(originalLocalUrl));

        log_game_string("Browser", reinterpret_cast<Il2CppString*>(originalLocalUrl));

        if (origBrowserLoadURL) {
            process_url(regs, true);
            origBrowserLoadURL(regs, originalLocalUrl);
        }
    }
};
