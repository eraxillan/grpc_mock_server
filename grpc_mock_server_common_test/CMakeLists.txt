set(CMRC_INCLUDE_DIR ${CMAKE_BINARY_DIR}/_cmrc/include)

include_directories("${GRPC_MOCK_SERVER_COMMON_BINARY_DIR}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})

find_package(Catch2 3 REQUIRED)

# These tests can use the Catch2-provided main
add_executable(
    grpc_mock_server_common_test
    test.cpp
    generated_code/test.pb.h
    generated_code/test.pb.cc
    generated_code/test.grpc.pb.h
    generated_code/test.grpc.pb.cc
)
#target_link_libraries(grpc_mock_server_common_test PRIVATE Catch2::Catch2WithMain)

# These tests need their own main
#add_executable(grpc_mock_server_common_test test.cpp test-main.cpp)
#target_link_libraries(grpc_mock_server_common_test PRIVATE Catch2::Catch2)

include(CTest)
include(Catch)
catch_discover_tests(grpc_mock_server_common_test)

set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set_property(TARGET grpc_mock_server_common_test PROPERTY CXX_STANDARD 20)
set_property(TARGET grpc_mock_server_common_test PROPERTY CXX_STANDARD_REQUIRED ON)
if ((MSVC) AND (MSVC_VERSION GREATER_EQUAL 1914))
    target_compile_options(grpc_mock_server_common_test PUBLIC "/Zc:__cplusplus")
endif()

find_package(CMakeRC CONFIG REQUIRED)
cmrc_add_resource_library(
    grpc_mock_server_common_test_resources
    ALIAS grpcmockserver::rc
    NAMESPACE grpc_mock_server
    "assets/config.xml"
    "assets/point_count_request.txt"
    "assets/request_grammar.txt"
)

find_package(gRPC CONFIG REQUIRED)
set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(pugixml CONFIG REQUIRED)

target_include_directories(
    grpc_mock_server_common_test
    PRIVATE
    ${CMRC_INCLUDE_DIR}
    ${Protobuf_INCLUDE_DIRS}
    "../grpc_mock_server_common"
)

set(
    LIBS
    spdlog::spdlog
    protobuf::libprotobuf
    gRPC::grpc++
    pugixml
    grpcmockserver::rc
    Catch2::Catch2WithMain 
    grpc_mock_server_common
)

target_link_libraries(
    grpc_mock_server_common_test
    PRIVATE
    ${LIBS}
)
