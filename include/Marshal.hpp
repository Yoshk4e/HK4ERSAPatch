#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <cstdint>

class Marshal {
public:
    static const uint8_t* WideToGameString(const std::wstring& ws) {
        // Convert to ANSI using game's expected encoding
        int size = WideCharToMultiByte(
            CP_ACP, 0,
            ws.c_str(), -1,
            nullptr, 0,
            nullptr, nullptr
        );
        
        std::vector<char> buffer(size);
        WideCharToMultiByte(
            CP_ACP, 0,
            ws.c_str(), -1,
            buffer.data(), size,
            nullptr, nullptr
        );

        return PtrToStringAnsi(buffer.data());
    }

private:
    static const uint8_t* PtrToStringAnsi(const char* content) {
        static auto func = reinterpret_cast<const uint8_t*(__fastcall*)(const uint8_t*)>(
            GetBaseAddress() + 0xFA8CBD0
        );
        return func(reinterpret_cast<const uint8_t*>(content));
    }

    static uintptr_t GetBaseAddress() {
        return reinterpret_cast<uintptr_t>(GetModuleHandleA(nullptr));
    }
};