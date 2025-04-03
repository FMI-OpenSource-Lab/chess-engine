cmake_minimum_required(VERSION 3.10)

# If not explicitly set, default to Release build type
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "No build type specified. Defaulting to Release.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()

# Project name and version
project(Engine VERSION 1.0 LANGUAGES CXX)

# Compiler settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add include directories
include_directories(
    "${CMAKE_SOURCE_DIR}/include"
    )
    
# All cops files
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

# Add executable target
# The name of the executable will be the same as the project name
add_executable(${PROJECT_NAME} ${SOURCES})

# Compiler specific settings
if (MINGW)
    target_compile_definitions(${PROJECT_NAME} PRIVATE WIN64 _AMD64_ IS_64BIT)
endif()

# Show compiler and system information
message(STATUS "System: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Arch: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "Compiler flags: ${CMAKE_CXX_FLAGS}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")