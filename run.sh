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
    local ORANGE='\033[0;33m'
    local NC='\033[0m'

    printf "${ORANGE}${@}${NC}"
}


function print-blue() {
    local BLUE='\033[0;34m'
    local NC='\033[0m'

    printf "${BLUE}${@}${NC}"
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

function ktest() {
    local build_dir="${SCRIPT_DIR}/build"
    local tests_dir="${SCRIPT_DIR}/tests"

    list_suites() {
        print-yellow "Available test suites:\n"
        for f in "${tests_dir}"/*_tests.cpp; do
            print-orange "  $(basename "${f}" .cpp)\n"
        done
    }

    # Configure a build dir on first use (Release, tests included).
    [ -d "${build_dir}" ] || {
        print-yellow "No build dir - configuring Release...\n"
        mkdir -p "${build_dir}"
        ( cd "${build_dir}" && cmake -DCMAKE_BUILD_TYPE=Release .. ) || {
            print-red "Configure failed!\n"; return 1
        }
    }

    # No argument: list the discoverable suites and bail.
    [ -n "${1}" ] || { list_suites; return 0; }

    # "ktest all": build everything and run the whole CTest set.
    [ "${1}" = "all" ] && {
        ( cd "${build_dir}" && cmake --build . --parallel && ctest --output-on-failure )
        return ${?}
    }

    # The suite name is the binary name, e.g. perft_tests.
    local target="${1}"
    [ -f "${tests_dir}/${target}.cpp" ] || {
        print-red "No test suite matching '${1}'\n"
        list_suites
        return 1
    }

    # Build just that suite (pulls in khaos_core), then run its binary.
    ( cd "${build_dir}" && cmake --build . --parallel --target "${target}" ) || {
        print-red "Build failed!\n"; return 1
    }

    print-green "Running ${target}...\n"
    "${SCRIPT_DIR}/bin/tests/${target}" "${@:2}"
}


print-yellow "debug\n"
print-blue "  Builds with Debug mode\n"
print-yellow "release\n"
print-blue "  Builds with Release mode\n"
print-yellow "incremental-build\n"
print-blue "  Does incremental build with last build mode\n"
print-blue "  Can be built with a specific flags passed as arguments\n"
print-yellow "ktest\n"
print-blue "  Lists the available gtest suites\n"
print-yellow "ktest <suite>\n"
print-blue "  Builds and runs one suite by name, e.g. 'ktest perft_tests'\n"
print-yellow "ktest all\n"
print-blue "  Builds everything and runs the full ctest sweep\n"
