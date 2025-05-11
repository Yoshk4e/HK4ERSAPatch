// module_manager.hpp
#pragma once

#include "interceptor.hpp"      // your C++ Interceptor wrapper
#include <unordered_map>
#include <memory>
#include <stdexcept>

// 1) The module types
enum class ModuleType {
    Http,
    Security,
    Misc,
    CcpBlocker,
};

// 2) The interface every module implements
struct IMhyModule {
    virtual ~IMhyModule() = default;

    // Initialize / deinitialize must throw on error
    virtual void init()    = 0;
    virtual void deinit()  = 0;

    // Identify which module this is
    virtual ModuleType getModuleType() const = 0;
};

// 3) The manager that owns & drives modules
class ModuleManager {
public:
    ModuleManager() = default;
    ~ModuleManager() = default;

    // Enable a new module: calls init(), then stores it
    void enable(std::unique_ptr<IMhyModule> module) {
        module->init();
        auto type = module->getModuleType();
        modules_.emplace(type, std::move(module));
    }

    // Disable & remove a module by its type
    void disable(ModuleType type) {
        auto it = modules_.find(type);
        if (it != modules_.end()) {
            it->second->deinit();
            modules_.erase(it);
        }
    }

private:
    // map from ModuleType → owning pointer
    std::unordered_map<ModuleType, std::unique_ptr<IMhyModule>> modules_;
};

// 4) The MhyContext<T> equivalent
template <class T>
struct MhyContext {
    std::size_t    assemblyBase;
    Interceptor    interceptor;

    // You must supply the module’s base address
    explicit MhyContext(std::size_t base)
      : assemblyBase(base)
      , interceptor()
    {}
};

