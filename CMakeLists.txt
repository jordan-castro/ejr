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
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -fPIC")

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
# 1. C++ core static library (internal)
# ------------------------------
add_library(ejr_core STATIC ${CPP_SOURCES})
target_include_directories(ejr_core PUBLIC ${PROJECT_SOURCE_DIR}/include)

# ------------------------------
# 2. C wrapper dynamic library (libejr)
# ------------------------------
add_library(ejr SHARED src/ejr_c_api.cpp ${C_SOURCES})
target_include_directories(ejr PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(ejr PRIVATE ejr_core)

# ------------------------------
# 3. Static library (C wrapper + C++ core)
# ------------------------------
add_library(ejr_static STATIC src/ejr_c_api.cpp ${C_SOURCES} ${CPP_SOURCES})
target_include_directories(ejr_static PUBLIC ${PROJECT_SOURCE_DIR}/include)

# ------------------------------
# 4. Platform-specific libraries
# ------------------------------
if(UNIX)
    target_link_libraries(ejr PRIVATE m dl pthread)
endif()

# ------------------------------
# 5. Test library
# ------------------------------
add_executable(libejr_test tests/test_c_repl.c)
target_include_directories(libejr_test PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(libejr_test PRIVATE ejr)
