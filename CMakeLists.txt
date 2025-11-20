cmake_minimum_required(VERSION 3.5)
project(db65816 VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable warnings
if (MSVC)
    add_compile_options(/W4)
else ()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif ()

# Set default build type
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

# ====== Logger Library ======
add_library(logger STATIC
        src/logger/Log.cpp
        src/logger/Log.hpp
)

target_include_directories(logger PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ====== CPU Library ======
# Collect all CPU source files
file(GLOB CPU_SOURCES
        src/cpu/*.cpp
        src/cpu/opcodes/*.cpp
)

# Collect all CPU header files
file(GLOB CPU_HEADERS
        src/cpu/include/*.hpp
)

add_library(cpu65816 STATIC
        ${CPU_SOURCES}
        ${CPU_HEADERS}
)

target_include_directories(cpu65816 PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/cpu/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(cpu65816 PUBLIC
        logger
)

# Optional: Uncomment to enable 65C02 emulation mode
# target_compile_definitions(cpu65816 PRIVATE EMU_65C02)

# ====== Devices Library ======
add_library(devices STATIC
        src/devices/Ram.cpp
        src/devices/Ram.hpp
)

target_include_directories(devices PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/devices
        ${CMAKE_CURRENT_SOURCE_DIR}/src/cpu/include
)

# ====== Main Executable ======
add_executable(db65816
        src/main.cpp
)

target_include_directories(db65816 PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/cpu/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src/devices
        ${CMAKE_CURRENT_SOURCE_DIR}/src/logger
)

target_link_libraries(db65816 PRIVATE
        cpu65816
        devices
        logger
)

# ====== Installation (optional) ======
install(TARGETS db65816
        RUNTIME DESTINATION bin
)

# ====== Testing (optional) ======
# Uncomment to enable testing
# enable_testing()
# add_subdirectory(tests)

# ====== Print build information ======
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Project version: ${PROJECT_VERSION}")
