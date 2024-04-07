// grpc_mock_server_plugin.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <iostream>

#include <google/protobuf/compiler/command_line_interface.h>
#include <google/protobuf/compiler/cpp/cpp_generator.h>
#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/compiler/plugin.h>
#include <google/protobuf/compiler/parser.h>

namespace google {
namespace protobuf {
namespace compiler {
namespace cpp {

class /*PROTOC_EXPORT*/ CppMockServerGenerator : public CodeGenerator {
public:
    CppMockServerGenerator();
    ~CppMockServerGenerator() override;

    // implements CodeGenerator ----------------------------------------
    bool GenerateAll(const std::vector<const FileDescriptor*>& files,
        const std::string& parameter,
        GeneratorContext* generator_context,
        std::string* error) const override;

    bool Generate(const FileDescriptor* file, const std::string& parameter,
        GeneratorContext* generator_context, std::string* error) const override;

    uint64_t GetSupportedFeatures() const override {
        // We don't fully support this yet, but this is needed to unblock the tests,
        // and we will have full support before the experimental flag is removed.
        return FEATURE_PROTO3_OPTIONAL;
    }

private:
    void WriteSummaryFile(const std::vector<const FileDescriptor*>& files,
        GeneratorContext* context) const;
};

}  // namespace cpp
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
