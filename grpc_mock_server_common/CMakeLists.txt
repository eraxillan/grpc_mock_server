# CMakeList.txt : CMake project for grpc_mock_server_common, include source and define project specific logic here

set(CMRC_INCLUDE_DIR ${CMAKE_BINARY_DIR}/_cmrc/include)

#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})

include(GenerateExportHeader)
include_directories("${CMAKE_CURRENT_BINARY_DIR}")
set(GRPC_MOCK_SERVER_COMMON_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}" PARENT_SCOPE)


set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)

# Add source to this project's library
add_library(
    grpc_mock_server_common
    SHARED
    "grpc_mock_server_configuration.cc"
    "grpc_mock_server_configuration.h"
    "grpc_mock_server_fs_utils.cc"
    "grpc_mock_server_fs_utils.h"
    "grpc_mock_server_logger.cc"
    "grpc_mock_server_logger.h"
    "grpc_mock_server_message_wrapper.cc"
    "grpc_mock_server_message_wrapper.h"
    "grpc_mock_server_utils.h"
)

generate_export_header(
    grpc_mock_server_common
    BASE_NAME grpc_mock_server_common
    EXPORT_MACRO_NAME GRPC_MOCK_SERVER_LIBRARY_API
    EXPORT_FILE_NAME grpc_mock_server_export.h
    STATIC_DEFINE SHARED_EXPORTS_BUILT_AS_STATIC
)

set_target_properties( 
    grpc_mock_server_common PROPERTIES 
    SOVERSION 1
    VERSION 1.0.0
)

if (MSVC)
    target_compile_definitions(grpc_mock_server_common PRIVATE GRPC_MOCK_SERVER_EXPORTS)
endif()

set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set_property(TARGET grpc_mock_server_common PROPERTY CXX_STANDARD 20)
set_property(TARGET grpc_mock_server_common PROPERTY CXX_STANDARD_REQUIRED ON)
if ((MSVC) AND (MSVC_VERSION GREATER_EQUAL 1914))
    target_compile_options(grpc_mock_server_common PUBLIC "/Zc:__cplusplus")
endif()

find_package(CMakeRC CONFIG REQUIRED)
cmrc_add_resource_library(
    grpc_mock_server_common_library_resources
    ALIAS grpcmockserver::rc
    NAMESPACE grpc_mock_server
    "assets/request_grammar.txt"
)

find_package(gRPC CONFIG REQUIRED)
set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(pugixml CONFIG REQUIRED)

target_include_directories(
    grpc_mock_server_common
    PRIVATE
    ${CMRC_INCLUDE_DIR}
    ${Protobuf_INCLUDE_DIRS}
)

set(LIBS spdlog::spdlog protobuf::libprotobuf gRPC::grpc++ pugixml grpcmockserver::rc)

target_link_libraries(
    grpc_mock_server_common
    PRIVATE
    ${LIBS}
)

# Add include directories
target_include_directories(
    grpc_mock_server_common
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>    # for headers when building
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>  # for client in install mode
)

