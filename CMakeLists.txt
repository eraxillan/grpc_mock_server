# CMakeList.txt : Top-level CMake project file, do global configuration and include sub-projects here
cmake_minimum_required(VERSION 3.12.0)

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

project("grpc_mock_server")

include(GNUInstallDirs)

set(CMRC_INCLUDE_DIR ${CMAKE_BINARY_DIR}/_cmrc/include)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})

# Include sub-projects
add_subdirectory("grpc_mock_server_common")
add_subdirectory("grpc_mock_server_common_test")
add_subdirectory("grpc_mock_server_plugin")
add_subdirectory("grpc_mock_server_library")
add_subdirectory("grpc_mock_server_executable")
