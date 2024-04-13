cmake_minimum_required(VERSION 3.12.0)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(VERBOSE)

if (NOT DEFINED CMAKE_MAKE_PROGRAM)
    message(FATAL_ERROR "CMAKE_MAKE_PROGRAM is not set, aborting")
endif()

if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    message(FATAL_ERROR "CMAKE_TOOLCHAIN_FILE is not set, aborting")
endif()

if (NOT DEFINED PROTO_API_PATH)
    message(FATAL_ERROR "PROTO_API_PATH is not set, aborting")
endif()

# Supress "-Wdeprecated-declarations" on gcc/clang
if (LINUX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
endif()

include(helpers.cmake)

set(
    LIBS
    spdlog::spdlog
    protobuf::libprotobuf
    gRPC::grpc++
    SQLiteCpp
    pugixml
    grpcmockserver::rc
    grpc_mock_server_common
)

if (ANDROID)
    find_library(log-lib log)
    set(ANDROID_NDK ${CUSTOM_ANDROID_NDK})

    unset(VCPKG_TARGET_TRIPLET)
    if(ANDROID_ABI STREQUAL "armeabi-v7a")
        set(VCPKG_TARGET_TRIPLET "arm-android" CACHE STRING "Android ARM 32-bit")
    elseif(ANDROID_ABI STREQUAL "arm64-v8a")
        set(VCPKG_TARGET_TRIPLET "arm64-android" CACHE STRING "Android ARM 64-bit")
    elseif(ANDROID_ABI STREQUAL "x86")
        set(VCPKG_TARGET_TRIPLET "x86-android" CACHE STRING "Android x86 32-bit")
    elseif(ANDROID_ABI STREQUAL "x86_64")
        set(VCPKG_TARGET_TRIPLET "x64-android" CACHE STRING "Android x86 64-bit")
    else()
        message(SEND_ERROR "Invalid ABI ${ANDROID_ABI}")
    endif()

    list(APPEND LIBS android log)
endif()

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("grpc_mock_server_library")

set(GRPC_BUILD_DIR ../grpc/outputs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${GRPC_BUILD_DIR})

set(GRPC_PROTO_GENS_DIR ${CMAKE_BINARY_DIR}/gens)
file(MAKE_DIRECTORY ${GRPC_PROTO_GENS_DIR})

set(CMRC_INCLUDE_DIR ${CMAKE_BINARY_DIR}/_cmrc/include)

set(GRPC_PROTO_COPY_DIR ${CMAKE_BINARY_DIR}/grpc_protos_copy)
file(REMOVE_RECURSE  ${GRPC_PROTO_COPY_DIR})
file(MAKE_DIRECTORY ${GRPC_PROTO_COPY_DIR})

set(GOOGLE_APIS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/googleapis" CACHE INTERNAL "")
set(GRPC_GATEWAY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/grpc-gateway" CACHE INTERNAL "")

find_package(CMakeRC CONFIG REQUIRED)
cmrc_add_resource_library(
    grpc_mock_server_library_resources
    ALIAS grpcmockserver::rc
    NAMESPACE grpc_mock_server
    "assets/packages.xml"
)
set_property(TARGET grpc_mock_server_library_resources PROPERTY POSITION_INDEPENDENT_CODE ON)
message(STATUS "Using CMakeRC ${_CMRC_VERSION}")

find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")

set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf CONFIG REQUIRED)
message(STATUS "Using Protobuf ${Protobuf_VERSION}")

find_package(spdlog CONFIG REQUIRED)
message(STATUS "Using spdlog ${spdlog_VERSION}")

if (WIN32)
    set(Python3_EXECUTABLE  "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows/tools/python3/python.exe")
    set(PROTOBUF_PROTOC_EXECUTABLE "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows/tools/protobuf/protoc.exe")
    set(GRPC_CPP_PLUGIN_EXECUTABLE "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows/tools/grpc/grpc_cpp_plugin.exe")
elseif (LINUX)
    set(Python3_EXECUTABLE  "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-linux/tools/python3/python3")
    set(PROTOBUF_PROTOC_EXECUTABLE "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-linux/tools/protobuf/protoc")
    set(GRPC_CPP_PLUGIN_EXECUTABLE "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-linux/tools/grpc/grpc_cpp_plugin")
else()
    message(SEND_ERROR "Only Windows/Linux support is implemented yet")
endif()

find_package(Python3 COMPONENTS Interpreter REQUIRED)
message(STATUS "Python interpreter path: ${Python3_EXECUTABLE}")

# Call the Python script for copying protos to build directory and fixing C++-related compilation problems
set(FIX_SCRIPT_COMMAND "")
string(JOIN " " FIX_SCRIPT_COMMAND
    ${Python3_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/fix.py
    ${PROTO_API_PATH}
    ${GRPC_PROTO_COPY_DIR}
)
separate_arguments(FIX_SCRIPT_COMMAND_LIST WINDOWS_COMMAND "${FIX_SCRIPT_COMMAND}")
execute_process(COMMAND ${FIX_SCRIPT_COMMAND_LIST})

# Call the Python script for generating gPRC C++ code from proto-files
set(FIX_SCRIPT_COMMAND "")
string(JOIN " " FIX_SCRIPT_COMMAND
    ${Python3_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/generate.py
    "-spd ${GRPC_PROTO_COPY_DIR}"
    "-tpd ${GRPC_PROTO_GENS_DIR}"
    "-bd ${CMAKE_BINARY_DIR}"

)
separate_arguments(FIX_SCRIPT_COMMAND_LIST WINDOWS_COMMAND "${FIX_SCRIPT_COMMAND}")
execute_process(COMMAND ${FIX_SCRIPT_COMMAND_LIST})

find_package(SQLiteCpp CONFIG REQUIRED)
find_package(pugixml CONFIG REQUIRED)
find_package(argparse CONFIG REQUIRED)

android_protobuf_grpc_generate_cpp(
    SWAGGER_PROTO_SRCS
    SWAGGER_PROTO_HDRS
    ${GRPC_GATEWAY_PATH}
    ${GRPC_GATEWAY_PATH}/protoc-gen-openapiv2/options/annotations.proto
    ${GRPC_GATEWAY_PATH}/protoc-gen-openapiv2/options/openapiv2.proto
)

android_protobuf_grpc_generate_cpp(
    GOOGLE_PROTO_SRCS
    GOOGLE_PROTO_HDRS
    ${GOOGLE_APIS_PATH}
    ${GOOGLE_APIS_PATH}/google/api/http.proto
    ${GOOGLE_APIS_PATH}/google/api/annotations.proto
    ${GOOGLE_APIS_PATH}/google/type/date.proto
    ${GOOGLE_APIS_PATH}/google/type/datetime.proto
    ${GOOGLE_APIS_PATH}/google/type/timeofday.proto
)

file(GLOB_RECURSE _backend_proto_files ${GRPC_PROTO_COPY_DIR}/*.proto CONFIGURE_DEPEND)

android_protobuf_grpc_generate_backend_cpp(
    BACKEND_PROTO_SRCS
    BACKEND_PROTO_HDRS
    BACKEND_STUB_SRCS
    BACKEND_STUB_HDRS
    ${GRPC_PROTO_COPY_DIR}
    ${_backend_proto_files}
)

list(
    APPEND
    BACKEND_STUB_SRCS
    "${GRPC_PROTO_GENS_DIR}/gmsServices.cc"
)
list(
    APPEND
    BACKEND_STUB_HDRS
    "${GRPC_PROTO_GENS_DIR}/gmsServices.h"
)

add_library(
    grpc-mock-server
    SHARED
    "src/business_logic.h"
    "src/business_logic.cc"
    "src/grpc_mock_server_library.h"
    "src/grpc_mock_server_library.cpp"
    "src/database_logger.cc"
    "src/pem_certificate_download.h"
    "src/pem_certificate_download.cc"
    ${BACKEND_STUB_SRCS}
    ${BACKEND_STUB_HDRS}
    ${SWAGGER_PROTO_SRCS}
    ${SWAGGER_PROTO_HDRS}
    ${GOOGLE_PROTO_SRCS}
    ${GOOGLE_PROTO_HDRS}
    ${BACKEND_PROTO_HDRS}
    ${BACKEND_PROTO_SRCS}
)

include(GenerateExportHeader)
include_directories("${CMAKE_CURRENT_BINARY_DIR}")
generate_export_header(
    grpc-mock-server
    BASE_NAME grpc-mock-server
    EXPORT_MACRO_NAME GRPC_MOCK_SERVER_LIBRARY_API
    EXPORT_FILE_NAME grpc_mock_server_export.h
    STATIC_DEFINE SHARED_EXPORTS_BUILT_AS_STATIC
)

set_property(TARGET grpc-mock-server PROPERTY CXX_STANDARD 20)
set_property(TARGET grpc-mock-server PROPERTY CXX_STANDARD_REQUIRED ON)
if ((MSVC) AND (MSVC_VERSION GREATER_EQUAL 1914))
    target_compile_options(grpc-mock-server PUBLIC "/Zc:__cplusplus")
endif()

target_compile_definitions(grpc-mock-server PRIVATE GRPC_MOCK_SERVER_EXPORTS)

target_include_directories(
    grpc-mock-server
    PRIVATE
    ${CMRC_INCLUDE_DIR}
    ${GRPC_PROTO_GENS_DIR}
    ${Protobuf_INCLUDE_DIRS}
    "../grpc_mock_server_common"
)

target_compile_definitions(
    grpc-mock-server
    PRIVATE
    "SQLITECPP_COMPILE_DLL"
)

target_link_libraries(
    grpc-mock-server
    PRIVATE
    ${LIBS}
)

add_custom_command(
    TARGET grpc-mock-server POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${GRPC_PROTO_GENS_DIR}/packages.xml
    ${CMAKE_CURRENT_SOURCE_DIR}/assets/packages.xml
)

# TODO: Add tests and install targets if needed
