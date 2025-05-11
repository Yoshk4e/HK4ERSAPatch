// SecurityModule.cpp

#include "security.hpp"

// Define the static trampoline pointer
SecurityModule::HookFn SecurityModule::origPerformCrypto = nullptr;
