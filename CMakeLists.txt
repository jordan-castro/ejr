cmake_minimum_required(VERSION 3.15)
project(EasyJSRuntime LANGUAGES C CXX)

# ------------------------------
# Set C and C++ flags
# ------------------------------
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -Wall")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall")

# ------------------------------
# Include directories
# ------------------------------
include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/js
)

# ------------------------------
# Source files
# ------------------------------
file(GLOB C_SOURCES "lib/*.c")
file(GLOB CPP_SOURCES
    "stdlib/*.cpp"
    "src/*.cpp"
)

# ------------------------------
# Add executable
# ------------------------------
add_executable(ejr ${C_SOURCES} ${CPP_SOURCES})

# ------------------------------
# Compile-time definitions
# ------------------------------
# Only __STDC_FORMAT_MACROS is needed now
target_compile_definitions(ejr PRIVATE __STDC_FORMAT_MACROS)

# ------------------------------
# Platform-specific libraries
# ------------------------------
if(WIN32)
    target_link_libraries(ejr m)
elseif(UNIX)
    target_link_libraries(ejr m dl pthread)
endif()
