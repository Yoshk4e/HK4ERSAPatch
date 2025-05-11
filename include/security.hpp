#pragma once

#include <ModuleManager.hpp>
#include <interceptor.hpp>
#include <common.h>            
#include <server_public_key.h>
#include <sdk_public_key.h>    
#include <windows.h>
#include <vector>
#include <string>
#include <stdexcept>
#include <cstdint>
#include <cstring>
#include <cstdlib>

static constexpr std::uintptr_t MHYRSA_PERFORM_CRYPTO_ACTION = 0x4B76F0;
static constexpr std::uintptr_t KEY_SIGN_CHECK               = 0x6E8A2E;
static constexpr std::uintptr_t SDK_UTIL_RSA_ENCRYPT         = 0xF9B01B0;

static constexpr size_t KEY_SIZE = 268;

class SecurityModule : public IMhyModule {
public:
    explicit SecurityModule(std::uintptr_t base)
      : assemblyBase(base)
    {}

    ~SecurityModule() override = default;

    void init() override {
        // Hook #1: MhyRSA.PerformCryptoAction
        try {
            hookPerform = assemblyBase + MHYRSA_PERFORM_CRYPTO_ACTION;
            origPerformCrypto = Interceptor::Get().Attach<HookFn>(
                hookPerform, &SecurityModule::on_mhy_rsa
            );
            printf("[SecurityModule] [SUCCESS] Hooked PerformCryptoAction at 0x%p\n", (void*)hookPerform);
        } catch (const std::exception& e) {
            printf("[SecurityModule] [FAIL] Failed to hook PerformCryptoAction: %s\n", e.what());
        }

        // Hook #2: KeySignCheck
        try {
            hookKeySign = assemblyBase + KEY_SIGN_CHECK;
            Interceptor::Get().Attach<HookFn>(
                hookKeySign, &SecurityModule::after_key_sign_check
            );
            printf("[SecurityModule] [SUCCESS] Hooked KeySignCheck at 0x%p\n", (void*)hookKeySign);
        } catch (const std::exception& e) {
            printf("[SecurityModule] [FAIL] Failed to hook KeySignCheck: %s\n", e.what());
        }

        // Hook #3: SDK.Util.RsaEncrypt
        try {
            hookEncrypt = assemblyBase + SDK_UTIL_RSA_ENCRYPT;
            Interceptor::Get().Attach<HookFn>(
                hookEncrypt, &SecurityModule::on_sdk_util_rsa_encrypt
            );
            printf("[SecurityModule] [SUCCESS] Hooked SDK.RsaEncrypt at 0x%p\n", (void*)hookEncrypt);
        } catch (const std::exception& e) {
            printf("[SecurityModule] [FAIL] Failed to hook SDK.RsaEncrypt: %s\n", e.what());
        }
    }

    void deinit() override {
        for (auto addr : { hookPerform, hookKeySign, hookEncrypt }) {
            MH_DisableHook(reinterpret_cast<LPVOID>(addr));
            MH_RemoveHook(reinterpret_cast<LPVOID>(addr));
        }
    }

    ModuleType getModuleType() const override {
        return ModuleType::Security;
    }

private:
    std::uintptr_t assemblyBase;
    std::uintptr_t hookPerform = 0;
    std::uintptr_t hookKeySign = 0;
    std::uintptr_t hookEncrypt = 0;

    using HookFn = void(__fastcall*)(Registers*, std::uintptr_t);
    static HookFn origPerformCrypto;

    static void __fastcall on_mhy_rsa(Registers* regs, std::uintptr_t) {
        printf("[SecurityModule] on_mhy_rsa triggered\n");

        if (regs->r8 == KEY_SIZE) {
            std::memcpy(
                reinterpret_cast<void*>(regs->rdx),
                server_public_key_bin,
                server_public_key_bin_len
            );
            printf("[SecurityModule] SERVER public key replaced (size: %llu bytes)\n", (unsigned long long)server_public_key_bin_len);
        } else {
            printf("[SecurityModule] r8 size mismatch (got %llu, expected %llu)\n", (unsigned long long)regs->r8, (unsigned long long)KEY_SIZE);
        }

        origPerformCrypto(regs, 0);
    }

    static void __fastcall after_key_sign_check(Registers* regs, std::uintptr_t) {
        printf("[SecurityModule] after_key_sign_check triggered\n");
        regs->rax = 1;
        printf("[SecurityModule] Key sign check overridden to succeed\n");
    }

    static void __fastcall on_sdk_util_rsa_encrypt(Registers* regs, std::uintptr_t) {
        printf("[SecurityModule] on_sdk_util_rsa_encrypt triggered\n");

        char* buf = static_cast<char*>(std::malloc(std::strlen(SDK_PUBLIC_KEY_XML) + 1));
        std::memcpy(buf, SDK_PUBLIC_KEY_XML, std::strlen(SDK_PUBLIC_KEY_XML) + 1);
        regs->rcx = reinterpret_cast<uint64_t>(buf);

        printf("[SecurityModule] SDK RSA public key replaced with XML (%zu bytes)\n", std::strlen(SDK_PUBLIC_KEY_XML));
    }
};
