import sys, os, re, shutil, fileinput, tempfile
from pathlib import Path

def escape_quotes(str):
    return str.replace("'", "\\'").replace("\"", "\\\"")

if not sys.version_info >= (3, 8):
    # Kindly tell your user (s)he needs to upgrade
    # because you're using 3.8 features
    sys.exit("Python >= 3.8 is required, please upgrade")

if len(sys.argv) != 3 or not sys.argv[1] or not sys.argv[2]:
    sys.exit("Two arguments required: the source protos directory path and temponary destination one")

if not os.path.isdir(sys.argv[1]):
    sys.exit("The source protos directory not found")
if not os.path.isdir(sys.argv[2]):
    sys.exit("The protos temponary destination directory not found")

src_proto_dir = Path(sys.argv[1])
dst_proto_dir = Path(sys.argv[2])

# Copy the proto root directory to this temponary one
shutil.copytree(src_proto_dir, dst_proto_dir, dirs_exist_ok=True)

# 1) Fix ANDROID
file_path = dst_proto_dir / "domain" / "domain.proto"
for line in fileinput.FileInput(file_path, encoding="utf8", inplace=True):
    line = line.replace("ANDROID = 1;", "ANDROID_ = 1;")
    print(line, end="")

# 2) Fix `auto`
file_path = dst_proto_dir / "autoAccessoriesWizard" / "methods.proto"
for line in fileinput.FileInput(file_path, encoding="utf8", inplace=True):
    line = line.replace("package auto.accessories.wizard;", "package auto_.accessories.wizard;")
    print(line, end="")

file_path = dst_proto_dir / "autoAccessoriesWizard" / "types.proto"
for line in fileinput.FileInput(file_path, encoding="utf8", inplace=True):
    line = line.replace("package auto.accessories.wizard;", "package auto_.accessories.wizard;")
    print(line, end="")

file_path = dst_proto_dir / "mgate" / "autoAccessoriesWizard.proto"
for line in fileinput.FileInput(file_path, encoding="utf8", inplace=True):
    line = line.replace(
        "repeated auto.accessories.wizard.CarVendor vendors = 4;",
        "repeated auto_.accessories.wizard.CarVendor vendors = 4;"
    )
    line = line.replace(
        "repeated auto.accessories.wizard.CarModel models = 4;",
        "repeated auto_.accessories.wizard.CarModel models = 4;"
    )
    line = line.replace(
        "repeated auto.accessories.wizard.CarModification models = 4;",
        "repeated auto_.accessories.wizard.CarModification models = 4;"
    )
    line = line.replace(
        "repeated auto.accessories.wizard.CarYear carYears = 4;",
        "repeated auto_.accessories.wizard.CarYear carYears = 4;"
    )
    line = line.replace(
        "repeated auto.accessories.wizard.CarBody carBodies = 4",
        "repeated auto_.accessories.wizard.CarBody carBodies = 4;"
    )
    line = line.replace(
        "repeated auto.accessories.wizard.CarAccessoryCollection carAccessoryCollections = 4;",
        "repeated auto_.accessories.wizard.CarAccessoryCollection carAccessoryCollections = 4;"
    )
    print(line, end="")

# 3) Replace `protoc-gen-swagger` with `protoc-gen-openapiv2`
proto_file_list = [f for f in dst_proto_dir.glob('**/*.proto') if os.path.isfile(f)]
for f in proto_file_list:
    for line in fileinput.FileInput(f, encoding="utf8", inplace=True):
        line = line.replace(
            "import \"protoc-gen-swagger/options/annotations.proto\";",
            "import \"protoc-gen-openapiv2/options/annotations.proto\";"
        )
        line = re.sub(
            "\.protoc_gen_swagger\.",
            ".protoc_gen_openapiv2.",
            line
        )
        match = re.search(r"example:\s*{value:\s*'({.*?})'\s*};$", line) # \s*{\s*(.*?)\s*}\s*;
        if match:
            match_escaped = escape_quotes(match.group(1))
            line = re.sub(r"example:\s*{value:\s*'{.*?}'\s*};$", f"example: \"{match_escaped}\";", line)
        print(line, end="")
