import pathlib
import sys, os, re, shutil, fileinput, tempfile, subprocess
from pathlib import Path
import shutil
import argparse
import glob


parser = argparse.ArgumentParser()
parser.add_argument("-cpd", "--custom-plugin-directory", required=True, help="the custom gPRC plugin binary directory")
parser.add_argument("-spd", "--source-proto-directory", required=True, help="the source protos directory path")
parser.add_argument("-tpd", "--temponary-proto-directory", required=True, help="the temponary destination one")
parser.add_argument("-bd", "--build-directory", required=True, help="program build directory absolute path")
args = parser.parse_args()

# Add custom plugin to PATH to allow `protoc` to find it
os.environ["PATH"] += os.pathsep + args.custom_plugin_directory
os.environ["LD_PRELOAD"] = (Path(args.custom_plugin_directory) / "libgrpc_mock_server_common.so.1.0.0").as_posix()

CMAKE_BINARY_DIR = args.build_directory
CMAKE_CURRENT_SOURCE_DIR = Path(sys.argv[0]).parent
GRPC_PROTO_GENS_DIR = Path(CMAKE_BINARY_DIR) / args.temponary_proto_directory

# Re-create the generated files directory
if GRPC_PROTO_GENS_DIR.exists() and GRPC_PROTO_GENS_DIR.is_dir():
    shutil.rmtree(GRPC_PROTO_GENS_DIR)
os.makedirs(GRPC_PROTO_GENS_DIR)

PROTOBUF_PROTOC_EXECUTABLE = ""
GRPC_CPP_PLUGIN_EXECUTABLE = ""
if sys.platform.startswith('win32'):
    PROTOBUF_PROTOC_EXECUTABLE = Path(f"{CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows/tools/protobuf/protoc.exe")
    GRPC_CPP_PLUGIN_EXECUTABLE = Path(f"{CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows/tools/grpc/grpc_cpp_plugin.exe")
elif sys.platform.startswith('linux'):
    PROTOBUF_PROTOC_EXECUTABLE = Path(f"{CMAKE_BINARY_DIR}/vcpkg_installed/x64-linux/tools/protobuf/protoc")
    GRPC_CPP_PLUGIN_EXECUTABLE = Path(f"{CMAKE_BINARY_DIR}/vcpkg_installed/x64-linux/tools/grpc/grpc_cpp_plugin")
elif sys.platform.startswith('darwin'):
    PROTOBUF_PROTOC_EXECUTABLE = Path(f"{CMAKE_BINARY_DIR}/vcpkg_installed/arm64-osx/tools/protobuf/protoc")
    GRPC_CPP_PLUGIN_EXECUTABLE = Path(f"{CMAKE_BINARY_DIR}/vcpkg_installed/arm64-osx/tools/grpc/grpc_cpp_plugin")
else:
    print("ERROR: Only Windows/Linux support is implemented yet")
    sys.exit(1)

if not PROTOBUF_PROTOC_EXECUTABLE.exists() or not PROTOBUF_PROTOC_EXECUTABLE.is_file():
    print("ERROR: Cannot detect protoc executable")
    sys.exit(1)

GOOGLE_APIS_PATH = f"{CMAKE_CURRENT_SOURCE_DIR}/googleapis"
GRPC_GATEWAY_PATH = f"{CMAKE_CURRENT_SOURCE_DIR}/grpc-gateway"

#####################################################################################################################################################

def android_protobuf_grpc_generate_cpp(src_files, hdr_files, include_root, *proto_files):
    if len(proto_files) == 0:
        print('Error: android_protobuf_grpc_generate_cpp() called without any proto files')
        return

    src_files_temp = []
    hdr_files_temp = []
    
    PROTOBUF_INCLUDE_PATH = [
        f"-I{include_root}",
        f"-I{GOOGLE_APIS_PATH}",
        f"-I{GRPC_GATEWAY_PATH}"
    ]
    
    proto_relative_files = []
    for proto_file in proto_files:
        path_object = Path(proto_file)
        
        # Get absolute file name
        proto_file_absolute = path_object.absolute()
        
        # Get file name without extension
        proto_file_without_extension = path_object.stem
        
        # Get relative to include root path
        proto_file_relative = path_object.relative_to(include_root)
        
        # Get the directory path of path
        proto_file_relative_dir = proto_file_relative.parent
        
        # Set the new path in generated directory
        generated_file_without_extension = proto_file_relative_dir / proto_file_without_extension

        src_files_temp.append(f"{GRPC_PROTO_GENS_DIR}/{generated_file_without_extension}.pb.cc")
        hdr_files_temp.append(f"{GRPC_PROTO_GENS_DIR}/{generated_file_without_extension}.pb.h")
        src_files_temp.append(f"{GRPC_PROTO_GENS_DIR}/{generated_file_without_extension}.grpc.pb.cc")
        hdr_files_temp.append(f"{GRPC_PROTO_GENS_DIR}/{generated_file_without_extension}.grpc.pb.h")
        
        proto_relative_files.append(proto_file_relative.as_posix())

    #print(f"Generate standard gPRC header and source...")
    subprocess.run(
        [
            PROTOBUF_PROTOC_EXECUTABLE,
            f"--grpc_out={GRPC_PROTO_GENS_DIR}",
            f"--cpp_out={GRPC_PROTO_GENS_DIR}",
            f"--plugin=protoc-gen-grpc={GRPC_CPP_PLUGIN_EXECUTABLE}",
            *PROTOBUF_INCLUDE_PATH,
            *proto_relative_files
        ],
        cwd=CMAKE_CURRENT_SOURCE_DIR,
        #shell=True,
        check=True,
        capture_output=True
    )
    
    src_files.extend(src_files_temp)
    hdr_files.extend(hdr_files_temp)

#####################################################################################################################################################

def android_protobuf_grpc_generate_backend_cpp(src_files, hdr_files, stub_src_files, stub_hdr_files, include_root, *proto_files):
    if len(proto_files) == 0:
        print('Error: android_protobuf_grpc_generate_backend_cpp() called without any proto files')
        return

    src_files_temp = []
    hdr_files_temp = []
    stub_src_files_temp = []
    stub_hdr_files_temp = []
    
    PROTOBUF_INCLUDE_PATH = [
        f"-I{include_root}",
        f"-I{GOOGLE_APIS_PATH}",
        f"-I{GRPC_GATEWAY_PATH}"
    ]

    proto_relative_files = []
    for proto_file in proto_files:
        # Get absolute file name
        proto_file_absolute = proto_file.absolute()
        
        # Get file name without extension
        proto_file_without_extension = proto_file.stem
        
        # Get relative to include root path
        proto_file_relative = proto_file.relative_to(include_root)
        
        # Get the directory path of path
        proto_file_relative_dir = proto_file_relative.parent
        
        # Set the new path in generated directory
        generated_file_without_extension = proto_file_relative_dir / proto_file_without_extension
        
        src_files_temp.append(f"{GRPC_PROTO_GENS_DIR}/{generated_file_without_extension}.grpc.pb.cc")
        src_files_temp.append(f"{GRPC_PROTO_GENS_DIR}/{generated_file_without_extension}.pb.cc")
        hdr_files_temp.append(f"{GRPC_PROTO_GENS_DIR}/{generated_file_without_extension}.grpc.pb.h")
        hdr_files_temp.append(f"{GRPC_PROTO_GENS_DIR}/{generated_file_without_extension}.pb.h")
        stub_src_files_temp.append(f"{GRPC_PROTO_GENS_DIR}/{generated_file_without_extension}.stub.cc")
        stub_hdr_files_temp.append(f"{GRPC_PROTO_GENS_DIR}/{generated_file_without_extension}.stub.h")
        
        proto_relative_files.append(proto_file_relative.as_posix())

    print(f"Generate gPRC headers and sources...")
    subprocess.run(
        [
            PROTOBUF_PROTOC_EXECUTABLE,
            f"--grpc_out={GRPC_PROTO_GENS_DIR}",
            f"--cpp_out={GRPC_PROTO_GENS_DIR}",
            f"--plugin=protoc-gen-grpc={GRPC_CPP_PLUGIN_EXECUTABLE}",
            *PROTOBUF_INCLUDE_PATH,
            *proto_relative_files
        ],
        cwd=CMAKE_CURRENT_SOURCE_DIR,
        #shell=True,
        check=True,
        capture_output=True
    )

    print(f"Generate gPRC stub headers and sources...")
    subprocess.run(
        [
            PROTOBUF_PROTOC_EXECUTABLE,
            f"--cpp-mock-server_out={GRPC_PROTO_GENS_DIR}",
            *PROTOBUF_INCLUDE_PATH,
            *proto_relative_files
        ],
        cwd=CMAKE_CURRENT_SOURCE_DIR,
        #shell=True,
        check=True,
        capture_output=True
    )
    
    src_files.extend(src_files_temp)
    hdr_files.extend(hdr_files_temp)
    stub_src_files.extend(stub_src_files_temp)
    stub_hdr_files.extend(stub_hdr_files_temp)

#####################################################################################################################################################

GOOGLE_APIS_PATH = f"{CMAKE_CURRENT_SOURCE_DIR}/googleapis"
GRPC_GATEWAY_PATH = f"{CMAKE_CURRENT_SOURCE_DIR}/grpc-gateway"
SWAGGER_PROTO_SRCS = []
SWAGGER_PROTO_HDRS = []
android_protobuf_grpc_generate_cpp(
    SWAGGER_PROTO_SRCS,
    SWAGGER_PROTO_HDRS,
    f"{GRPC_GATEWAY_PATH}",
    f"{GRPC_GATEWAY_PATH}/protoc-gen-openapiv2/options/annotations.proto",
    f"{GRPC_GATEWAY_PATH}/protoc-gen-openapiv2/options/openapiv2.proto"
)

GOOGLE_PROTO_SRCS = []
GOOGLE_PROTO_HDRS = []
android_protobuf_grpc_generate_cpp(
    GOOGLE_PROTO_SRCS,
    GOOGLE_PROTO_HDRS,
    f'{GOOGLE_APIS_PATH}',
    f'{GOOGLE_APIS_PATH}/google/api/http.proto',
    f'{GOOGLE_APIS_PATH}/google/api/annotations.proto',
    f'{GOOGLE_APIS_PATH}/google/type/date.proto',
    f'{GOOGLE_APIS_PATH}/google/type/datetime.proto',
    f'{GOOGLE_APIS_PATH}/google/type/timeofday.proto'
)

source_proto_path = Path(args.source_proto_directory)
protoc_input_files = []
for proto_file in source_proto_path.rglob("*.proto"):
    path_object = Path(proto_file)
    protoc_input_files.append(path_object)

BACKEND_PROTO_SRCS = []
BACKEND_PROTO_HDRS = []
BACKEND_STUB_SRCS = []
BACKEND_STUB_HDRS = []
android_protobuf_grpc_generate_backend_cpp(
    BACKEND_PROTO_SRCS,
    BACKEND_PROTO_HDRS,
    BACKEND_STUB_SRCS,
    BACKEND_STUB_HDRS,
    args.source_proto_directory,
    *protoc_input_files
)

# Print all generated headers to stdout
#for header in SWAGGER_PROTO_HDRS + GOOGLE_PROTO_HDRS + BACKEND_PROTO_HDRS + BACKEND_STUB_HDRS:
#    print(header)

# Print all generated sources to stdout
#for source in SWAGGER_PROTO_SRCS + GOOGLE_PROTO_SRCS + BACKEND_PROTO_SRCS + BACKEND_STUB_SRCS:
#    print(source)

sys.exit(0)
