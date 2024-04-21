cmake_minimum_required (VERSION 3.8)

if (NOT DEFINED CMAKE_MAKE_PROGRAM)
    message(FATAL_ERROR "CMAKE_MAKE_PROGRAM is not set, aborting")
endif()

if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    message(FATAL_ERROR "CMAKE_TOOLCHAIN_FILE is not set, aborting")
endif()

# Enable Hot Reload for MSVC compilers if supported
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})

project("protoc-gen-cpp-mock-server")

# Add source to this project's executable
add_executable(
    protoc-gen-cpp-mock-server
    "grpc_mock_server_plugin.cpp"
    "grpc_mock_server_plugin.h"
    "generator.cpp"
    "generator.h"
)

set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set_property(TARGET protoc-gen-cpp-mock-server PROPERTY CXX_STANDARD 20)
set_property(TARGET protoc-gen-cpp-mock-server PROPERTY CXX_STANDARD_REQUIRED ON)
if ((MSVC) AND (MSVC_VERSION GREATER_EQUAL 1914))
    target_compile_options(protoc-gen-cpp-mock-server PUBLIC "/Zc:__cplusplus")
endif()

find_package(CMakeRC CONFIG REQUIRED)
cmrc_add_resource_library(
    protoc_gen_cpp_mock_server_resources
    ALIAS protoc_gen_cpp_mock_server::rc
    NAMESPACE grpc_mock_server_plugin
    "templates/stub_header_additional_include.txt"
    "templates/stub_header_server_method_no_streaming.txt"
    "templates/stub_header_server_method_client_only_streaming.txt"
    "templates/stub_header_server_method_server_only_streaming.txt"
    "templates/stub_header_server_method_bidi_streaming.txt"
    "templates/stub_source_server_method_no_streaming.txt"
    "templates/stub_source_server_method_no_streaming_health_check.txt"
    "templates/stub_source_server_method_client_only_streaming.txt"
    "templates/stub_source_server_method_server_only_streaming.txt"
    "templates/stub_source_server_method_bidi_streaming.txt"
    "templates/stub_source_server_method_unix_time.txt"
    "templates/stub_header_prologue.txt"
    "templates/stub_header_prologue_with_original_file_comments.txt"
    "templates/stub_header_epilogue.txt"
    "templates/stub_header_service.txt"
    "templates/stub_header_service_end.txt"
    "templates/stub_header_include.txt"
    "templates/stub_header_namespace_begin.txt"
    "templates/stub_header_namespace_end.txt"
    "templates/stub_header_service_namespace_begin.txt"
    "templates/stub_header_service_namespace_end.txt"
    "templates/stub_source_prologue.txt"
    "templates/stub_source_namespace_begin.txt"
    "templates/stub_source_namespace_end.txt"
    "templates/services_header_prologue.txt"
    "templates/services_header_namespace_begin.txt"
    "templates/services_header_namespace_end.txt"
    "templates/services_header_class_declaration.txt"
    "templates/services_header_pointer_declaration.txt"
    "templates/services_header_class_prologue.txt"
    "templates/services_header_class_epilogue.txt"
    "templates/services_header_pointer_definition.txt"
    "templates/services_header_epilogue.txt"
    "templates/services_source_prologue.txt"
    "templates/services_source_includes.txt"
    "templates/services_source_constructor_begin.txt"
    "templates/services_source_constructor_end.txt"
    "templates/services_source_pointer_definition.txt"
    "templates/services_source_destructor.txt"
    "templates/services_source_method_begin.txt"
    "templates/services_source_method_body.txt"
    "templates/services_source_method_end.txt"
    "templates/packages_xml_prologue.txt"
    "templates/packages_xml_epilogue.txt"
    "templates/packages_xml_package_begin.txt"
    "templates/packages_xml_service_begin.txt"
    "templates/packages_xml_service_method.txt"
    "templates/packages_xml_package_end.txt"
    "templates/packages_xml_service_end.txt"
    "templates/packages_xml_service_method_leading_comments.txt"
    "templates/packages_xml_service_method_trailing_comments.txt"
)
message(STATUS "Using CMakeRC ${_CMRC_VERSION}")

find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")

set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf CONFIG REQUIRED)
message(STATUS "Using protobuf ${Protobuf_VERSION}")

target_include_directories(
    protoc-gen-cpp-mock-server
    PRIVATE
    ${CMRC_INCLUDE_DIR}
    ${Protobuf_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
    "grpc"
)

target_link_libraries(
    protoc-gen-cpp-mock-server
    protoc_gen_cpp_mock_server::rc
    protobuf::libprotobuf
    protobuf::libprotoc
    gRPC::grpc++
)
