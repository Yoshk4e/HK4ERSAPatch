## HK4ERSAPatch

A DLL-based patching solution for the HK4ERSA application with a focus on modifying security and network behavior.

## Table of Contents

- Overview
- Features
- Project Structure
- Installation
- Key Components
- Usage
- Compatibility
- Development
- License
- Contact

## Overview

HK4ERSAPatch is designed to modify and extend the functionality of the HK4ERSA application. It utilizes code hooking techniques to intercept and alter application behavior, with a particular focus on security features and network communications.

## Features

- **Function Hooking**: Implements MinHook to intercept application calls
- **Security Modifications**: Custom security implementation and key management
- **Network Interception**: Capture and modify HTTP requests
- **Protocol Protection**: Block unwanted communications
- **Module Management**: Dynamic management of application modules

## Project Structure

```
HK4ERSAPatch/
├── include/
│   └── lib/
│       ├── Marshal.hpp           - Data marshaling utilities
│       ├── MinHook.h             - Function hooking library header
│       ├── ModuleManager.hpp     - Application module management
│       ├── Util.h                - General utility functions
│       ├── ccp_blocker.hpp       - Communication protocol blocking
│       ├── common.h              - Common definitions and structures
│       ├── dllmain.h             - DLL entry point declarations
│       ├── http.hpp              - HTTP protocol handling
│       ├── interceptor.hpp       - Function interception framework
│       ├── misc.hpp              - Miscellaneous helper functions
│       ├── sdk_public_key.h      - SDK authentication key definitions
│       ├── security.hpp          - Security-related functionality
│       └── server_public_key.h   - Server authentication key definitions
│
├── src/
│   ├── Util.cpp                  - Utility function implementations
│   ├── ccp_blocker.cpp           - Protocol blocking implementation
│   ├── dllmain.cpp               - DLL main entry point
│   ├── misc.cpp                  - Miscellaneous function implementations
│   ├── security.cpp              - Security feature implementations
│   └── CMakeLists.txt            - CMake build configuration
```

## Installation

1. Clone the repository:

   ```
   git clone https://github.com/Yoshk4e/HK4ERSAPatch.git
   ```

2. Build the project using CMake:

   ```
   mkdir build
   cd build
   cmake ..
   cmake --build .
   ```

3. Inject the compiled DLL into the HK4ERSA application using your preferred method:

   - DLL injector
   - Application launcher with DLL preloading
   - Direct modification of the target application

## Key Components

### Security Module

The security module (`security.hpp` and `security.cpp`) manages authentication and encryption, likely modifying the original application's security checks or implementing custom security measures.

### Network Interception

The HTTP module (`http.hpp`) and interceptor (`interceptor.hpp`) work together to capture, analyze, and potentially modify network communications between the application and servers.

### Communication Protocol Blocking

The CCP blocker (`ccp_blocker.hpp` and `ccp_blocker.cpp`) appears to prevent certain types of communications, possibly for security or performance reasons.

### Module Management

The ModuleManager (`ModuleManager.hpp`) provides functionality for dynamically loading, unloading, and interacting with application modules.

## Usage

After installation, the patch automatically modifies the target application's behavior according to its implementation. No additional configuration should be necessary unless specified in a configuration file (if implemented).

## Compatibility

This patch is specifically designed for a particular version of the HK4ERSA application. Using it with incompatible versions may cause stability issues or crashes. Please verify your application version before installation.

## Development

To contribute to this project:

1. Fork the repository

2. Create a feature branch:

   ```
   git checkout -b feature/your-feature-name
   ```

3. Make your changes

4. Submit a pull request

## License

This project is provided as-is without any express or implied warranties. Use at your own risk.

## Contact

For issues, feature requests, or questions, please create an issue on the GitHub repository.
