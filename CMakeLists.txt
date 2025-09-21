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
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/
)

# ------------------------------
# Source files
# ------------------------------
file(GLOB C_SOURCES "lib/*.c")
file(GLOB CPP_SOURCES "src/*.cpp")

# ------------------------------
# 1. C++ Executable
# ------------------------------
add_executable(ejr ${C_SOURCES} ${CPP_SOURCES})
target_compile_definitions(ejr PRIVATE __STDC_FORMAT_MACROS)

if(WIN32)
    target_link_libraries(ejr m)
elseif(UNIX)
    target_link_libraries(ejr m dl pthread)
endif()

# ------------------------------
# 2. C++ Libraries
# ------------------------------
# C++ static library (full runtime)
add_library(ejr_cpp_static STATIC ${C_SOURCES} ${CPP_SOURCES})
target_include_directories(ejr_cpp_static PUBLIC ${PROJECT_SOURCE_DIR}/include)

# C++ shared library
add_library(ejr_cpp_shared SHARED ${C_SOURCES} ${CPP_SOURCES})
target_include_directories(ejr_cpp_shared PUBLIC ${PROJECT_SOURCE_DIR}/include)

# ------------------------------
# 3. C API Libraries
# ------------------------------
# Only include C API layer, link against C++ runtime
add_library(ejr_c_static STATIC src/ejr_c_api.cpp)
target_include_directories(ejr_c_static PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(ejr_c_static PRIVATE ejr_cpp_static)

add_library(ejr_c_shared SHARED src/ejr_c_api.cpp)
target_include_directories(ejr_c_shared PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(ejr_c_shared PRIVATE ejr_cpp_shared)

# ------------------------------
# 4. C Executable (test)
# ------------------------------
add_executable(ejr_c_test tests/test_c_main.c)
target_include_directories(ejr_c_test PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(ejr_c_test PRIVATE ejr_c_shared)

# ------------------------------
# Platform-specific libraries
# ------------------------------
if(WIN32)
    # nothing extra needed, m is inlined
elseif(UNIX)
    target_link_libraries(ejr_cpp_shared PRIVATE m dl pthread)
    target_link_libraries(ejr_cpp_static PRIVATE m dl pthread)
endif()
