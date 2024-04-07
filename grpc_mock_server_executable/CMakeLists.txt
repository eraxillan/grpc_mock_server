cmake_minimum_required(VERSION 3.12.0)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(VERBOSE)

if (NOT DEFINED CMAKE_MAKE_PROGRAM)
    message(FATAL_ERROR "CMAKE_MAKE_PROGRAM is not set, aborting")
endif()

if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    message(FATAL_ERROR "CMAKE_TOOLCHAIN_FILE is not set, aborting")
endif()

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

include_directories("${GRPC_MOCK_SERVER_COMMON_BINARY_DIR}")
#message(FATAL_ERROR "TEST: ${GRPC_MOCK_SERVER_COMMON_BINARY_DIR}")

set(CMRC_INCLUDE_DIR ${CMAKE_BINARY_DIR}/_cmrc/include)
find_package(CMakeRC CONFIG REQUIRED)
cmrc_add_resource_library(
    grpc_mock_server_executable_resources
    ALIAS grpcmockserver::rc
    NAMESPACE grpc_mock_server
    "assets/ca.crt"
    "assets/ca.key"
    "assets/client.crt"
    "assets/client.csr"
    "assets/client.key"
    "assets/server.crt"
    "assets/server.csr"
    "assets/server.key"
    "assets/packages.xml"
    "assets/request_grammar.txt"
)
set_property(TARGET grpc_mock_server_executable_resources PROPERTY POSITION_INDEPENDENT_CODE ON)
message(STATUS "Using CMakeRC ${_CMRC_VERSION}")

find_package(spdlog CONFIG REQUIRED)
find_package(argparse CONFIG REQUIRED)

project("grpc_mock_server_executable")

add_executable(grpc_mock_server_executable "src/main.cc")

target_include_directories(
    grpc_mock_server_executable
    PRIVATE
    ${CMRC_INCLUDE_DIR}
    "../grpc_mock_server_common"
    "../grpc_mock_server_library/src"
)

target_link_libraries(
    grpc_mock_server_executable
    spdlog::spdlog
    argparse::argparse
    grpcmockserver::rc
    "grpc-mock-server"
    grpc_mock_server_common
)

set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set_property(TARGET grpc_mock_server_executable PROPERTY CXX_STANDARD 20)
set_property(TARGET grpc_mock_server_executable PROPERTY CXX_STANDARD_REQUIRED ON)
if ((MSVC) AND (MSVC_VERSION GREATER_EQUAL 1914))
    target_compile_options(grpc_mock_server_executable PUBLIC "/Zc:__cplusplus")
endif()
