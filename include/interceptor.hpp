// Interceptor.hpp
#pragma once

#include <MinHook.h>
#include <vector>
#include <stdexcept>
#include <cstdint>
#include <sstream>
#include <mutex>

struct HookPoint {
    LPVOID target;
    LPVOID detour;
    LPVOID original;
};

class Interceptor {
public:
    static Interceptor& Get() {
        static Interceptor instance;
        return instance;
    }

    template<typename FuncPtr>
    FuncPtr Attach(std::uintptr_t addr, FuncPtr detour) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        LPVOID original = nullptr;
        MH_STATUS status = MH_CreateHook(
            reinterpret_cast<LPVOID>(addr),
            reinterpret_cast<LPVOID>(detour),
            &original
        );

        if (status != MH_OK) {
            throw std::runtime_error(
                "CreateHook failed: " + std::string(MH_StatusToString(status)) +
                " at 0x" + Hex(addr)
            );
        }

        status = MH_EnableHook(reinterpret_cast<LPVOID>(addr));
        if (status != MH_OK) {
            MH_RemoveHook(reinterpret_cast<LPVOID>(addr));
            throw std::runtime_error(
                "EnableHook failed: " + std::string(MH_StatusToString(status)) +
                " at 0x" + Hex(addr)
            );
        }

        hooks_.push_back({reinterpret_cast<LPVOID>(addr), 
                         reinterpret_cast<LPVOID>(detour), 
                         original});
        return reinterpret_cast<FuncPtr>(original);
    }

    void RemoveAll() {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& hp : hooks_) {
            MH_DisableHook(hp.target);
            MH_RemoveHook(hp.target);
        }
        hooks_.clear();
    }

    Interceptor(const Interceptor&) = delete;
    Interceptor& operator=(const Interceptor&) = delete;

private:
    std::vector<HookPoint> hooks_;
    std::mutex mutex_;

    Interceptor() {
        MH_STATUS status = MH_Initialize();
        if (status != MH_OK) {
            throw std::runtime_error(
                "MinHook init failed: " + std::string(MH_StatusToString(status))
            );
        }
    }
    friend class CcpBlocker;

    ~Interceptor() = default;

    static std::string Hex(std::uintptr_t v) {
        std::ostringstream ss;
        ss << std::hex << v;
        return ss.str();
    }
};