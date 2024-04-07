
function(android_protobuf_grpc_generate_cpp SRC_FILES HDR_FILES INCLUDE_ROOT)
    if(NOT ARGN)
        message(SEND_ERROR "Error: android_protobuf_grpc_generate_cpp() called without any proto files")
        return()
    endif()

    set(${SRC_FILES})
    set(${HDR_FILES})
    set(
        PROTOBUF_INCLUDE_PATH
        -I ${INCLUDE_ROOT}
        -I ${GOOGLE_APIS_PATH}
        -I ${GRPC_GATEWAY_PATH}
    )
    foreach(FIL ${ARGN})
        get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
        get_filename_component(FIL_WE ${FIL} NAME_WE)
        file(RELATIVE_PATH REL_FIL ${INCLUDE_ROOT} ${ABS_FIL})
        get_filename_component(REL_DIR ${REL_FIL} DIRECTORY)
        set(RELFIL_WE "${REL_DIR}/${FIL_WE}")

        list(APPEND ${SRC_FILES} "${GRPC_PROTO_GENS_DIR}/${RELFIL_WE}.pb.cc")
        list(APPEND ${HDR_FILES} "${GRPC_PROTO_GENS_DIR}/${RELFIL_WE}.pb.h")
        list(APPEND ${SRC_FILES} "${GRPC_PROTO_GENS_DIR}/${RELFIL_WE}.grpc.pb.cc")
        list(APPEND ${HDR_FILES} "${GRPC_PROTO_GENS_DIR}/${RELFIL_WE}.grpc.pb.h")

        add_custom_command(
            OUTPUT
            "${GRPC_PROTO_GENS_DIR}/${RELFIL_WE}.grpc.pb.cc"
            "${GRPC_PROTO_GENS_DIR}/${RELFIL_WE}.grpc.pb.h"
            "${GRPC_PROTO_GENS_DIR}/${RELFIL_WE}.pb.cc"
            "${GRPC_PROTO_GENS_DIR}/${RELFIL_WE}.pb.h"
            COMMAND
            ${PROTOBUF_PROTOC_EXECUTABLE}
            ARGS
            --grpc_out=${GRPC_PROTO_GENS_DIR}
            --cpp_out=${GRPC_PROTO_GENS_DIR}
            --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN_EXECUTABLE}
            ${PROTOBUF_INCLUDE_PATH}
            ${REL_FIL}
            WORKING_DIRECTORY
            ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS
            ${PROTOBUF_PROTOC_EXECUTABLE}
            ${GRPC_CPP_PLUGIN_EXECUTABLE}
            ${ABS_FIL}
        )
    endforeach()

    set_source_files_properties(${${SRC_FILES}} ${${HDR_FILES}} PROPERTIES GENERATED TRUE)
    set(${SRC_FILES} ${${SRC_FILES}} PARENT_SCOPE)
    set(${HDR_FILES} ${${HDR_FILES}} PARENT_SCOPE)
endfunction()

function(android_protobuf_grpc_generate_backend_cpp SRC_FILES HDR_FILES STUB_SRC_FILES STUB_HDR_FILES INCLUDE_ROOT)
    if(NOT ARGN)
        message(SEND_ERROR "Error: android_protobuf_grpc_generate_cpp() called without any proto files")
        return()
    endif()

    set(${SRC_FILES})
    set(${HDR_FILES})
    set(${STUB_SRC_FILES})
    set(${STUB_HDR_FILES})

    set(_protoc_input_files)
    set(_protoc_depends)
    foreach(_proto_abs_path ${ARGN})
        get_filename_component(_proto_file_name ${_proto_abs_path} NAME)
        get_filename_component(_proto_file_name_wo_ext ${_proto_abs_path} NAME_WE)
        file(RELATIVE_PATH _proto_relative_path ${INCLUDE_ROOT} ${_proto_abs_path})
        get_filename_component(_proto_relative_dir ${_proto_relative_path} DIRECTORY)
        set(_proto_file_name_relative "${_proto_relative_dir}/${_proto_file_name}")
        set(_proto_file_name_wo_ext_relative "${_proto_relative_dir}/${_proto_file_name_wo_ext}")

        list(
            APPEND
            SRC_FILES
            ${GRPC_PROTO_GENS_DIR}/${_proto_file_name_wo_ext_relative}.grpc.pb.cc
            ${GRPC_PROTO_GENS_DIR}/${_proto_file_name_wo_ext_relative}.pb.cc
        )
        list(
            APPEND
            HDR_FILES
            ${GRPC_PROTO_GENS_DIR}/${_proto_file_name_wo_ext_relative}.grpc.pb.h
            ${GRPC_PROTO_GENS_DIR}/${_proto_file_name_wo_ext_relative}.pb.h
        )
        list(
            APPEND
            STUB_SRC_FILES
            ${GRPC_PROTO_GENS_DIR}/${_proto_file_name_wo_ext_relative}.stub.cc
        )
        list(
            APPEND
            STUB_HDR_FILES
            ${GRPC_PROTO_GENS_DIR}/${_proto_file_name_wo_ext_relative}.stub.h
        )

        list(APPEND _protoc_input_files ${_proto_file_name_relative})
        list(APPEND _protoc_depends ${_proto_abs_path})
    endforeach()

    set(
        PROTOBUF_INCLUDE_PATH
        -I ${INCLUDE_ROOT}
        -I ${GOOGLE_APIS_PATH}
        -I ${GRPC_GATEWAY_PATH}
    )

    add_custom_command(
        COMMENT "Google protobuf C++ code generator"
        OUTPUT
        ${SRC_FILES}
        ${HDR_FILES}
        COMMAND
        ${PROTOBUF_PROTOC_EXECUTABLE}
        ARGS
        --grpc_out=${GRPC_PROTO_GENS_DIR}
        --cpp_out=${GRPC_PROTO_GENS_DIR}
        --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN_EXECUTABLE}
        ${PROTOBUF_INCLUDE_PATH}
        ${_protoc_input_files}
        WORKING_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS
        ${PROTOBUF_PROTOC_EXECUTABLE}
        ${GRPC_CPP_PLUGIN_EXECUTABLE}
        ${_protoc_depends}
    )

    add_custom_command(
        COMMENT "Google protobuf C++ mock server code generator"
        OUTPUT
        ${STUB_SRC_FILES}
        ${STUB_HDR_FILES}
        ${GRPC_PROTO_GENS_DIR}/services.cc
        ${GRPC_PROTO_GENS_DIR}/services.h
        ${GRPC_PROTO_GENS_DIR}/messageWrapper.cc
        ${GRPC_PROTO_GENS_DIR}/messageWrapper.h
        ${GRPC_PROTO_GENS_DIR}/logger.cc
        ${GRPC_PROTO_GENS_DIR}/logger.h
        COMMAND
        ${PROTOBUF_PROTOC_EXECUTABLE}
        ARGS
        --cpp-mock-server_out=${GRPC_PROTO_GENS_DIR}
        ${PROTOBUF_INCLUDE_PATH}
        ${_protoc_input_files}
        WORKING_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS
        ${PROTOBUF_PROTOC_EXECUTABLE}
        ${GRPC_CPP_PLUGIN_EXECUTABLE}
        ${_protoc_depends}
    )

    set_source_files_properties(
        ${${SRC_FILES}}
        ${${HDR_FILES}}
        ${${STUB_SRC_FILES}}
        ${${STUB_HDR_FILES}}
        ${${GRPC_PROTO_GENS_DIR}/services.cc}
        ${${GRPC_PROTO_GENS_DIR}/services.h}
        ${${GRPC_PROTO_GENS_DIR}/messageWrapper.cc}
        ${${GRPC_PROTO_GENS_DIR}/messageWrapper.h}
        ${${GRPC_PROTO_GENS_DIR}/logger.cc}
        ${${GRPC_PROTO_GENS_DIR}/logger.h}
        PROPERTIES GENERATED TRUE
    )
    set(${SRC_FILES} ${${SRC_FILES}} PARENT_SCOPE)
    set(${HDR_FILES} ${${HDR_FILES}} PARENT_SCOPE)
    set(${STUB_SRC_FILES} ${${STUB_SRC_FILES}} PARENT_SCOPE)
    set(${STUB_HDR_FILES} ${${STUB_HDR_FILES}} PARENT_SCOPE)
    set(${GRPC_PROTO_GENS_DIR/services.cc} ${${GRPC_PROTO_GENS_DIR}/services.cc} PARENT_SCOPE)
    set(${GRPC_PROTO_GENS_DIR/services.h} ${${GRPC_PROTO_GENS_DIR}/services.h} PARENT_SCOPE)
    set(${GRPC_PROTO_GENS_DIR/messageWrapper.cc} ${${GRPC_PROTO_GENS_DIR}/messageWrapper.cc} PARENT_SCOPE)
    set(${GRPC_PROTO_GENS_DIR/messageWrapper.h} ${${GRPC_PROTO_GENS_DIR}/messageWrapper.h} PARENT_SCOPE)
    set(${GRPC_PROTO_GENS_DIR/logger.cc} ${${GRPC_PROTO_GENS_DIR}/logger.cc} PARENT_SCOPE)
    set(${GRPC_PROTO_GENS_DIR/logger.h} ${${GRPC_PROTO_GENS_DIR}/logger.h} PARENT_SCOPE)
endfunction()
