// MiscModule.hpp
#pragma once

#include <ModuleManager.hpp>   // IMhyModule, ModuleType
#include <interceptor.hpp>
#include <common.h>            // your Registers definition
#include <windows.h>
#include <cstdint>

// RVA of Unity’s SetCustomPropertyFloat thunk
static constexpr std::uintptr_t SET_CUSTOM_PROPERTY_FLOAT = 0xFD3D20;

class MiscModule : public IMhyModule {
public:
    explicit MiscModule(std::uintptr_t base)
      : assemblyBase(base)
    {}

    ~MiscModule() override = default;

    void init() override {
        // Compute the target address and hook it so it always returns 0
        hookAddr = assemblyBase + SET_CUSTOM_PROPERTY_FLOAT;
        using HookFn = std::uintptr_t(__fastcall*)(Registers*, std::uintptr_t, std::uintptr_t);
        Interceptor::Get().Attach<HookFn>(
            hookAddr,
            &MiscModule::set_custom_property_float_replacement
        );
    }

    void deinit() override {
        // Clean up our single hook
        MH_DisableHook(reinterpret_cast<LPVOID>(hookAddr));
        MH_RemoveHook(reinterpret_cast<LPVOID>(hookAddr));
    }

    ModuleType getModuleType() const override {
        return ModuleType::Misc;
    }

private:
    std::uintptr_t assemblyBase;
    std::uintptr_t hookAddr = 0;

    // Matches Rust’s `extern "win64" fn(*mut Registers, usize, usize) -> usize`
    static std::uintptr_t __fastcall set_custom_property_float_replacement(
        Registers* /*regs*/,
        std::uintptr_t /*arg1*/,
        std::uintptr_t /*arg2*/
    ) {
        return 0;
    }
};
