cmake_minimum_required(VERSION 3.15)
project(hk4eCryptPatch LANGUAGES CXX)

# Static runtime for MSVC
if (MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")  # /MT or /MTd
endif()

# Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Export symbols (DllMain)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Source files
add_library(hk4eCryptPatch SHARED)
file(GLOB HK4E_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
target_sources(hk4eCryptPatch PRIVATE ${HK4E_SOURCES})

# Include directories
target_include_directories(hk4eCryptPatch PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link libraries
target_link_libraries(hk4eCryptPatch PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include/lib/minhook.x64.lib  # Static MinHook
  ws2_32
)

# Compile definitions
target_compile_definitions(hk4eCryptPatch PRIVATE
  MH_STATIC
  _CRT_SECURE_NO_WARNINGS
  _WINSOCKAPI_
  WIN32_LEAN_AND_MEAN
)
