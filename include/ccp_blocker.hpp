#pragma once

#include <windows.h>
#include <cstring>
#include <string>
#include <ws2tcpip.h>
#include <stdexcept>
#include <cstdint>
#include <ModuleManager.hpp>
#include <interceptor.hpp>

// CCP DNS blocker module
class CcpBlocker : public IMhyModule {
public:
    explicit CcpBlocker(std::size_t base)
        : assemblyBase(base)
    {}

    ~CcpBlocker() override = default;

    void init() override {
        // 1) Get Ws2_32.dll handle
        HMODULE ws2 = ::GetModuleHandleA("Ws2_32.dll");
        if (!ws2) {
            throw std::runtime_error("GetModuleHandleA(Ws2_32.dll) failed");
        }

        // 2) Resolve getaddrinfo
        auto pRaw = ::GetProcAddress(ws2, "getaddrinfo");
        if (!pRaw) {
            throw std::runtime_error("GetProcAddress(getaddrinfo) failed");
        }
        targetAddr = reinterpret_cast<std::uintptr_t>(pRaw);

        // 3) Define function‐pointer type
        using FnGetAddrInfo = int (__stdcall*)(
            const char*, const char*, const addrinfo*, addrinfo**);

        // 4) Attach the hook via the MinHook singleton
        originalGetAddrInfo = Interceptor::Get().Attach<FnGetAddrInfo>(
            targetAddr,
            &CcpBlocker::on_getaddrinfo
        );
    }

    void deinit() override {
        // Disable & remove our specific hook
        MH_DisableHook(reinterpret_cast<LPVOID>(targetAddr));
        MH_RemoveHook(reinterpret_cast<LPVOID>(targetAddr));
    }

    ModuleType getModuleType() const override {
        return ModuleType::CcpBlocker;
    }

private:
    std::size_t    assemblyBase;
    std::uintptr_t targetAddr = 0;

    using FnGetAddrInfo = int (__stdcall*)(
        const char*, const char*, const addrinfo*, addrinfo**);
    static FnGetAddrInfo originalGetAddrInfo;

    // Detour callback
    static int __stdcall on_getaddrinfo(
        const char* node,
        const char* service,
        const addrinfo* hints,
        addrinfo** res
    ) {
        if (node) {
            std::string host(node);
            if (host == "dispatchosglobal.yuanshen.com" ||
                host == "dispatchcnglobal.yuanshen.com")
            {
                // Overwrite hostname buffer with "0.0.0.0"
                std::memcpy(const_cast<char*>(node), "0.0.0.0", 8);
            }
        }
        return originalGetAddrInfo(node, service, hints, res);
    }
};
