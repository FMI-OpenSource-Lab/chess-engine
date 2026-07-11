#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

function print-yellow() {
    local YELLOW='\033[1;33m'
    local NC='\033[0m'

    printf "${YELLOW}${@}${NC}"
}

function print-red() {
    local RED='\033[0;31m'
    local NC='\033[0m'

    printf "${RED}${@}${NC}"
}

function print-green() {
    local GREEN='\033[0;32m'
    local NC='\033[0m'

    printf "${GREEN}${@}${NC}"
}

function print-orange() {
    local GREEN='\033[0;33m'
    local NC='\033[0m'

    printf "${GREEN}${@}${NC}"
}

function debug() {
    [ -d "${SCRIPT_DIR}/build" ]                                               && {
        print-yellow "Build exists in current! Removing...\n"
        rm -rf "${SCRIPT_DIR}/build/"
    }

    (
        print-yellow "Create build folder!\n"
        mkdir "${SCRIPT_DIR}/build"

        print-yellow "Build and configure into build folder!\n"
        incremental-build -DCMAKE_BUILD_TYPE=Debug                             \
            -DCMAKE_CXX_FLAGS_DEBUG="-DDEBUG_MODE"
    )
}

function release() {
    [ -d "${SCRIPT_DIR}/build" ]                                               && {
        print-yellow "Build exists in current! Removing...\n"
        rm -rf "${SCRIPT_DIR}/build/"
    }

    (
        print-yellow "Create build folder!\n"
        mkdir "${SCRIPT_DIR}/build"

        print-yellow "Build and configure into build folder!\n"
        incremental-build -DCMAKE_BUILD_TYPE=Release
    )
}

function incremental-build() {
    (
        cd "${SCRIPT_DIR}/build"

        cmake "${@}" ..                                                        && {
            print-green "CMake: Successful configure!\n"

            cmake --build . --parallel                                         && {
                print-green "CMake: Successful build!\n"
            }                                                                  || {
                print-red "CMake: Build failed!\n";
                return 1;
            }
        }                                                                      || {
            print-red "CMake: Configuration failed!\n";
            return 1;
        }
    ) && {
        print-green "CMake: Success!\n"
    }                                                                          || {
        print-red "CMake: Error occurred!\n"
    }
}

print-yellow "debug\n"
print-orange "  Builds with Debug mode\n"
print-yellow "release\n"
print-orange "  Builds with Release mode\n"
print-yellow "incremental-build\n"
print-orange "  Does incremental build with last build mode\n"
print-orange "  Can be built with a specific flags passed as arguments\n"
