// SecurityModule.cpp
// Implementation file for the SecurityModule

#include "security.hpp"

// Define the static trampoline pointer
SecurityModule::HookFn SecurityModule::origPerformCrypto = nullptr;

// Optional factory export:
// extern "C" IMhyModule* CreateSecurityModule(std::uintptr_t base) {
//     return new SecurityModule(base);
// }
