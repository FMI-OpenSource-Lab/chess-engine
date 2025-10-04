#!/bin/bash

function print-yellow() {
    local YELLOW='\033[1;33m'
    local NC='\033[0m' # No Color

    printf "${YELLOW}${@}${NC}"
}

function print-red() {
    local RED='\033[0;31m'
    local NC='\033[0m' # No Color

    printf "${RED}${@}${NC}"
}

function print-green() {
    local GREEN='\033[0;32m'
    local NC='\033[0m' # No Color

    printf "${GREEN}${@}${NC}"
}

function print-orange() {
    local GREEN='\033[0;33m'
    local NC='\033[0m' # No Color

    printf "${GREEN}${@}${NC}"
}

function debug() {
    [ -d "${PWD}/build" ]                                                   && {
        print-yellow "Build exists in current! Removing...\n"
        rm -rf "build/"
    }

    (
        print-yellow "Create build folder!\n"
        mkdir build

        print-yellow "Build and configure into build folder!\n"
        incremental-build -DCMAKE_BUILD_TYPE=Debug                             \
            -DCMAKE_CXX_FLAGS_DEBUG="-DDEBUG_MODE"
    )
}

function release() {
    [ -d "${PWD}/build" ]                                                   && {
        print-yellow "Build exists in current! Removing...\n"
        rm -rf "build/"
    }

    (
        print-yellow "Create build folder!\n"
        mkdir build

        print-yellow "Build and configure into build folder!\n"
        incremental-build -DCMAKE_BUILD_TYPE=Release
    )
}

function incremental-build() {
    # Do stuff into separate subprocess
    (
        cd build

        # Take build type type as argument
        cmake ${@} ..                                                       && {
            print-green "CMake: Successful configure!\n"

            cmake --build .                                                 && {
                print-green "CMake: Successful build!\n"

                print-green "Install binary or library into root!\n"

                sudo cmake --install . --prefix "/usr"                      && {
                    print-green "CMake: Installed successfully!\n"
                }                                                           || {
                    print-red "CMake: Installing failed!\n";
                    return 1;
                }
            }                                                               || {
                print-red "CMake: Build failed!\n";
                return 1;
            }
        }                                                                   || {
            print-red "CMake: Configuration failed!\n";
            return 1;
        }
    ) && {
        print-green "CMake: Success!\n"
    }                                                                       || {
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
