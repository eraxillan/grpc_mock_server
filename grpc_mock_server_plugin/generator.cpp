#include "generator.h"

#include <cmrc/cmrc.hpp>
CMRC_DECLARE(grpc_mock_server_plugin);

namespace grpc_cpp_generator {

namespace {

std::string read_resource_file(const std::string& file_name) {
    // Validate and load embedded resources
    auto rc_fs = cmrc::grpc_mock_server_plugin::get_filesystem();
    assert(rc_fs.exists(file_name) && rc_fs.is_file(file_name));
    auto file = rc_fs.open(file_name);
    auto data = std::string(file.cbegin(), file.cend());
    return data;
}

auto stub_header_additional_include = read_resource_file("templates/stub_header_additional_include.txt");
auto stub_header_server_method_no_streaming = read_resource_file("templates/stub_header_server_method_no_streaming.txt");
auto stub_header_server_method_client_only_streaming = read_resource_file("templates/stub_header_server_method_client_only_streaming.txt");
auto stub_header_server_method_server_only_streaming = read_resource_file("templates/stub_header_server_method_server_only_streaming.txt");
auto stub_header_server_method_bidi_streaming = read_resource_file("templates/stub_header_server_method_bidi_streaming.txt");
auto stub_source_server_method_no_streaming = read_resource_file("templates/stub_source_server_method_no_streaming.txt");
auto stub_source_server_method_no_streaming_health_check = read_resource_file("templates/stub_source_server_method_no_streaming_health_check.txt");
auto stub_source_server_method_client_only_streaming = read_resource_file("templates/stub_source_server_method_client_only_streaming.txt");
auto stub_source_server_method_server_only_streaming = read_resource_file("templates/stub_source_server_method_server_only_streaming.txt");
auto stub_source_server_method_bidi_streaming = read_resource_file("templates/stub_source_server_method_bidi_streaming.txt");
auto stub_source_server_method_unix_time = read_resource_file("templates/stub_source_server_method_unix_time.txt");
auto stub_header_prologue = read_resource_file("templates/stub_header_prologue.txt");
auto stub_header_prologue_with_original_file_comments = read_resource_file("templates/stub_header_prologue_with_original_file_comments.txt");
auto stub_header_epilogue = read_resource_file("templates/stub_header_epilogue.txt");
auto stub_header_service = read_resource_file("templates/stub_header_service.txt");
auto stub_header_service_end = read_resource_file("templates/stub_header_service_end.txt");
auto stub_header_include = read_resource_file("templates/stub_header_include.txt");
auto stub_header_namespace_begin = read_resource_file("templates/stub_header_namespace_begin.txt");
auto stub_header_namespace_end = read_resource_file("templates/stub_header_namespace_end.txt");
auto stub_header_service_namespace_begin = read_resource_file("templates/stub_header_service_namespace_begin.txt");
auto stub_header_service_namespace_end = read_resource_file("templates/stub_header_service_namespace_end.txt");
auto stub_source_prologue = read_resource_file("templates/stub_source_prologue.txt");
auto stub_source_namespace_begin = read_resource_file("templates/stub_source_namespace_begin.txt");
auto stub_source_namespace_end = read_resource_file("templates/stub_source_namespace_end.txt");
auto services_header_prologue = read_resource_file("templates/services_header_prologue.txt");
auto services_header_namespace_begin = read_resource_file("templates/services_header_namespace_begin.txt");
auto services_header_namespace_end = read_resource_file("templates/services_header_namespace_end.txt");
auto services_header_class_declaration = read_resource_file("templates/services_header_class_declaration.txt");
auto services_header_pointer_declaration = read_resource_file("templates/services_header_pointer_declaration.txt");
auto services_header_class_prologue = read_resource_file("templates/services_header_class_prologue.txt");
auto services_header_class_epilogue = read_resource_file("templates/services_header_class_epilogue.txt");
auto services_header_pointer_definition = read_resource_file("templates/services_header_pointer_definition.txt");
auto services_header_epilogue = read_resource_file("templates/services_header_epilogue.txt");
auto services_source_prologue = read_resource_file("templates/services_source_prologue.txt");
auto services_source_includes = read_resource_file("templates/services_source_includes.txt");
auto services_source_constructor_begin = read_resource_file("templates/services_source_constructor_begin.txt");
auto services_source_constructor_end = read_resource_file("templates/services_source_constructor_end.txt");
auto services_source_pointer_definition = read_resource_file("templates/services_source_pointer_definition.txt");
auto services_source_destructor = read_resource_file("templates/services_source_destructor.txt");
auto services_source_method_begin = read_resource_file("templates/services_source_method_begin.txt");
auto services_source_method_body = read_resource_file("templates/services_source_method_body.txt");
auto services_source_method_end = read_resource_file("templates/services_source_method_end.txt");
auto packages_xml_prologue = read_resource_file("templates/packages_xml_prologue.txt");
auto packages_xml_epilogue = read_resource_file("templates/packages_xml_epilogue.txt");
auto packages_xml_package_begin = read_resource_file("templates/packages_xml_package_begin.txt");
auto packages_xml_service_begin = read_resource_file("templates/packages_xml_service_begin.txt");
auto packages_xml_service_method = read_resource_file("templates/packages_xml_service_method.txt");
auto packages_xml_package_end = read_resource_file("templates/packages_xml_package_end.txt");
auto packages_xml_service_end = read_resource_file("templates/packages_xml_service_end.txt");
auto packages_xml_service_method_leading_comments = read_resource_file("templates/packages_xml_service_method_leading_comments.txt");
auto packages_xml_service_method_trailing_comments = read_resource_file("templates/packages_xml_service_method_trailing_comments.txt");


// trim from start (in place)
static inline void ltrim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
static inline void rtrim(std::string& s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
        [](unsigned char ch) { return !std::isspace(ch); })
        .base(),
        s.end());
}

// trim from both ends (in place)
static inline void trim(std::string& s) {
    rtrim(s);
    ltrim(s);
}

// trim from start (copying)
static inline std::string ltrim_copy(std::string s) {
    ltrim(s);
    return s;
}

// trim from end (copying)
static inline std::string rtrim_copy(std::string s) {
    rtrim(s);
    return s;
}

// trim from both ends (copying)
static inline std::string trim_copy(std::string s) {
    trim(s);
    return s;
}

template <class T>
std::string as_string(T x) {
    std::ostringstream out;
    out << x;
    return out.str();
}

inline bool ClientOnlyStreaming(const grpc_generator::Method* method) {
    return method->ClientStreaming() && !method->ServerStreaming();
}

inline bool ServerOnlyStreaming(const grpc_generator::Method* method) {
    return !method->ClientStreaming() && method->ServerStreaming();
}

std::string FilenameIdentifier(const std::string& filename) {
    std::string result;
    for (unsigned i = 0; i < filename.size(); i++) {
        char c = filename[i];
        if (isalnum(c)) {
            result.push_back(c);
        }
        else {
            static char hex[] = "0123456789abcdef";
            result.push_back('_');
            result.push_back(hex[(c >> 4) & 0xf]);
            result.push_back(hex[c & 0xf]);
        }
    }
    return result;
}

template <class T, size_t N>
T* array_end(T(&array)[N]) {
    return array + N;
}

void PrintIncludes(grpc_generator::Printer* printer,
    const std::vector<std::string>& headers,
    bool use_system_headers, 
    const std::string& search_path
) {
    std::map<std::string, std::string> vars;

    vars["l"] = use_system_headers ? '<' : '"';
    vars["r"] = use_system_headers ? '>' : '"';

    if (!search_path.empty()) {
        vars["l"] += search_path;
        if (search_path[search_path.size() - 1] != '/') {
            vars["l"] += '/';
        }
    }

    for (auto i = headers.begin(); i != headers.end(); i++) {
        vars["h"] = *i;
        printer->Print(vars, stub_header_additional_include.data());
    }
}

void PrintStubHeaderServerMethodSync(grpc_generator::Printer* printer,
    const grpc_generator::Method* method,
    std::map<std::string, std::string>* vars) {
    (*vars)["Method"] = method->name();
    (*vars)["Request"] = method->input_type_name();
    (*vars)["Response"] = method->output_type_name();
    printer->Print(method->GetLeadingComments("//").c_str());
    if (method->NoStreaming()) {
        printer->Print(*vars, stub_header_server_method_no_streaming.data());
    }
    else if (ClientOnlyStreaming(method)) {
        printer->Print(*vars, stub_header_server_method_client_only_streaming.data());
    }
    else if (ServerOnlyStreaming(method)) {
        printer->Print(*vars, stub_header_server_method_server_only_streaming.data());
    }
    else if (method->BidiStreaming()) {
        printer->Print(*vars, stub_header_server_method_bidi_streaming.data());
    }
    printer->Print(method->GetTrailingComments("//").c_str());
}

void PrintStubHeaderService(grpc_generator::Printer* printer,
    const grpc_generator::Service* service,
    std::map<std::string, std::string>* vars) {
    // Package string is empty or ends with a dot. It is used to fully qualify method names
    if (vars->find("Package") != vars->end()) {
        std::string packageValue = (*vars)["Package"];
        if (!packageValue.empty()) {
            packageValue = packageValue.erase(packageValue.length() - 1);
            packageValue = DotsToColons(packageValue);
            (*vars)["PackageRaw"] = packageValue;
        }
    }

    (*vars)["Service"] = service->name();

    printer->Print(service->GetLeadingComments("//").c_str());

    printer->Print(*vars, stub_header_service.data());
    printer->Indent();
    for (int i = 0; i < service->method_count(); ++i) {
        PrintStubHeaderServerMethodSync(printer, service->method(i).get(), vars);
    }
    printer->Outdent();
    printer->Print(stub_header_service_end.data());
    printer->Print(service->GetTrailingComments("//").c_str());
}

void PrintStubSourceServerMethod(grpc_generator::Printer* printer,
    const grpc_generator::Method* method,
    std::map<std::string, std::string>* vars) {
    (*vars)["Method"] = method->name();
    (*vars)["Request"] = method->input_type_name();
    (*vars)["Response"] = method->output_type_name();
    (*vars)["UnixTime"] = stub_source_server_method_unix_time.data();
    if (method->NoStreaming()) {
        if (method->name() == "HealthCheck") {
            printer->Print(*vars, stub_source_server_method_no_streaming_health_check.data());
        }
        else {
            printer->Print(*vars, stub_source_server_method_no_streaming.data());
        }
    }
    else if (ClientOnlyStreaming(method)) {
        printer->Print(*vars, stub_source_server_method_client_only_streaming.data());
    }
    else if (ServerOnlyStreaming(method)) {
        printer->Print(*vars, stub_source_server_method_server_only_streaming.data());
    }
    else if (method->BidiStreaming()) {
        printer->Print(*vars, stub_source_server_method_bidi_streaming.data());
    }
}

void PrintStubSourceService(grpc_generator::Printer* printer,
    const grpc_generator::Service* service,
    std::map<std::string, std::string>* vars) {
    (*vars)["Service"] = service->name();

    for (int i = 0; i < service->method_count(); ++i) {
        (*vars)["Idx"] = as_string(i);
        PrintStubSourceServerMethod(printer, service->method(i).get(), vars);
    }
}

}  // anonymous namespace

// Stub headers and sources

// Return the prologue of the generated stub header file
std::string GetStubHeaderPrologue(grpc_generator::File* file, const Parameters& params) {
    std::string output;
    {
        // Scope the output stream so it closes and finalizes output to the string
        auto printer = file->CreatePrinter(&output);
        std::map<std::string, std::string> vars;

        vars["filename"] = file->filename();
        vars["filename_identifier"] = FilenameIdentifier(file->filename());
        vars["filename_base"] = file->filename_without_ext();
        vars["message_header_ext"] = params.message_header_extension.empty()
            ? kCppGeneratorServiceHeaderExt
            : params.message_header_extension;
        vars["additional_headers"] = file->additional_headers();

        std::string leading_comments = file->GetLeadingComments("//");
        if (leading_comments.empty()) {
            printer->Print(vars, stub_header_prologue.data());
        }
        else {
            vars["leading_comments"] = leading_comments;
            printer->Print(vars, stub_header_prologue_with_original_file_comments.data());
        }
    }
    return output;
}

// Return the includes needed for generated stub header file
std::string GetStubHeaderIncludes(grpc_generator::File* file, const Parameters& params) {
    std::string output;
    {
        // Scope the output stream so it closes and finalizes output to the string
        auto printer = file->CreatePrinter(&output);
        std::map<std::string, std::string> vars;

        if (!params.additional_header_includes.empty()) {
            PrintIncludes(printer.get(), params.additional_header_includes, false, "");
        }

        vars["message_header_ext"] = params.message_header_extension.empty()
            ? kCppGeneratorMessageHeaderExt
            : params.message_header_extension;

        if (params.include_import_headers) {
            const std::vector<std::string> import_names = file->GetImportNames();
            for (const auto& import_name : import_names) {
                vars["proto_file_name"] = import_name.substr(0, import_name.size() - 6);
                printer->Print(vars, stub_header_include.data());
            }
            printer->PrintRaw("\n");
        }

        if (!file->package().empty()) {
            std::vector<std::string> parts = file->package_parts();

            for (auto part = parts.begin(); part != parts.end(); part++) {
                vars["part"] = *part;
                printer->Print(vars, stub_header_namespace_begin.data());
            }
            printer->Print(vars, "\n");
        }
    }
    return output;
}

// Return the epilogue of the generated stb header file
std::string GetStubHeaderEpilogue(grpc_generator::File* file, const Parameters& params) {
    std::string output;
    {
        // Scope the output stream so it closes and finalizes output to the string
        auto printer = file->CreatePrinter(&output);
        std::map<std::string, std::string> vars;

        vars["filename"] = file->filename();
        vars["filename_identifier"] = FilenameIdentifier(file->filename());

        if (!file->package().empty()) {
            std::vector<std::string> parts = file->package_parts();

            for (auto part = parts.rbegin(); part != parts.rend(); part++) {
                vars["part"] = *part;
                printer->Print(vars, stub_header_namespace_end.data());
            }
            printer->Print("\n");
        }

        printer->Print(vars, stub_header_epilogue.data());
        printer->Print(file->GetTrailingComments("//").c_str());
    }
    return output;
}

// Return the services for generated stub header file
std::string GetStubHeaderServices(grpc_generator::File* file, const Parameters& params) {
    std::string output;
    {
        // Scope the output stream so it closes and finalizes output to the string
        auto printer = file->CreatePrinter(&output);
        std::map<std::string, std::string> vars;
        // Package string is empty or ends with a dot. It is used to fully qualify method names
        vars["Package"] = file->package();
        if (!file->package().empty()) {
            vars["Package"].append(".");
        }

        if (!params.services_namespace.empty()) {
            vars["services_namespace"] = params.services_namespace;
            printer->Print(vars, stub_header_service_namespace_begin.data());
        }

        for (int i = 0; i < file->service_count(); ++i) {
            PrintStubHeaderService(printer.get(), file->service(i).get(), &vars);
            printer->Print("\n");
        }

        if (!params.services_namespace.empty()) {
            printer->Print(vars, stub_header_service_namespace_end.data());
        }
    }
    return output;
}

// Return the prologue of the generated stub source file
std::string GetStubSourcePrologue(grpc_generator::File* file, const Parameters& params) {
    std::string output;
    {
        // Scope the output stream so it closes and finalizes output to the string
        auto printer = file->CreatePrinter(&output);
        std::map<std::string, std::string> vars;

        vars["filename"] = file->filename();
        vars["filename_base"] = file->filename_without_ext();
        vars["message_header_ext"] = params.message_header_extension.empty()
            ? kCppGeneratorMessageHeaderExt
            : params.message_header_extension;
        vars["service_header_ext"] = kCppGeneratorServiceHeaderExt;
        printer->Print(vars, stub_source_prologue.data());
    }
    return output;
}

// Return the includes needed for generated stub source file
std::string GetStubSourceIncludes(grpc_generator::File* file, const Parameters& params) {
    std::string output;
    {
        // Scope the output stream so it closes and finalizes output to the string
        auto printer = file->CreatePrinter(&output);
        std::map<std::string, std::string> vars;

        if (!file->package().empty()) {
            std::vector<std::string> parts = file->package_parts();

            for (auto part = parts.begin(); part != parts.end(); part++) {
                vars["part"] = *part;
                printer->Print(vars, stub_source_namespace_begin.data());
            }
        }

        printer->Print(vars, "\n");
    }
    return output;
}

// Return the epilogue of the generated stub source file
std::string GetStubSourceEpilogue(grpc_generator::File* file, const Parameters& params) {
    std::string temp;

    if (!file->package().empty()) {
        std::vector<std::string> parts = file->package_parts();

        for (auto part = parts.begin(); part != parts.end(); part++) {
            temp.append(stub_source_namespace_end.data());
            temp.append(*part);
            temp.append("\n");
        }
    }

    return temp;
}

// Return the services for generated stub source file.
std::string GetStubSourceServices(grpc_generator::File* file, const Parameters& params) {
    std::string output;
    {
        // Scope the output stream so it closes and finalizes output to the string
        auto printer = file->CreatePrinter(&output);
        std::map<std::string, std::string> vars;
        // Package string is empty or ends with a dot. It is used to fully qualify method names
        vars["Package"] = file->package();
        if (!file->package().empty()) {
            vars["Package"].append(".");
        }
        if (!params.services_namespace.empty()) {
            vars["ns"] = params.services_namespace + "::";
            vars["prefix"] = params.services_namespace;
        }
        else {
            vars["ns"] = "";
            vars["prefix"] = "";
        }

        for (int i = 0; i < file->service_count(); ++i) {
            PrintStubSourceService(printer.get(), file->service(i).get(), &vars);
            printer->Print("\n");
        }
    }
    return output;
}

// Services header

std::string GetServicesHeaderPrologue(grpc_generator::File* file, const Parameters& params) {
    std::string output;
    {
        // Scope the output stream so it closes and finalizes output to the string
        auto printer = file->CreatePrinter(&output);
        std::map<std::string, std::string> vars;

        vars["filename_identifier"] = FilenameIdentifier("services.h");

        printer->Print(vars, services_header_prologue.data());
    }
    return output;
}

std::string GetServicesHeaderForwardDeclarations(grpc_generator::File* file, const Parameters& params) {
    if (file->service_count() == 0) return "";

    std::string output;
    {
        // Scope the output stream so it closes and finalizes output to the string
        auto printer = file->CreatePrinter(&output);
        std::map<std::string, std::string> vars;

        // TODO: group by namespace
        if (!file->package().empty()) {
            std::vector<std::string> parts = file->package_parts();
            for (auto part = parts.begin(); part != parts.end(); part++) {
                vars["part"] = *part;
                printer->Print(vars, services_header_namespace_begin.data());
            }
        }

        for (int i = 0; i < file->service_count(); ++i) {
            vars["Service"] = file->service(i)->name();
            printer->Print(vars, services_header_class_declaration.data());
        }

        if (!file->package().empty()) {
            std::vector<std::string> parts = file->package_parts();

            for (auto part = parts.rbegin(); part != parts.rend(); part++) {
                vars["part"] = *part;
                printer->Print(vars, services_header_namespace_end.data());
            }
            printer->Print(vars, "\n");
        }
    }
    return output;
}

std::string GetServicesHeaderPointerDeclarations(grpc_generator::File* file, const Parameters& params) {
    if (file->service_count() == 0) return "";

    std::string output;
    {
        // Scope the output stream so it closes and finalizes output to the string
        auto printer = file->CreatePrinter(&output);
        std::map<std::string, std::string> vars;

        auto packageValue = file->package();
        if (!packageValue.empty()) {
            vars["PackageRaw"] = DotsToColons(packageValue);
            vars["PackageRaw2"] = DotsToUnderscores(packageValue);
        }

        for (int i = 0; i < file->service_count(); ++i) {
            vars["Service"] = file->service(i)->name();
            printer->Print(vars, services_header_pointer_declaration.data());
        }
    }
    return output;
}

std::string GetServicesHeaderClassPrologue(grpc_generator::File* file, const Parameters& params) {
    if (file->service_count() == 0) return "";

    std::string output;
    {
        // Scope the output stream so it closes and finalizes output to the string
        auto printer = file->CreatePrinter(&output);
        std::map<std::string, std::string> vars;

        printer->Print(vars, services_header_class_prologue.data());
    }
    return output;
}

std::string GetServicesHeaderClassEpilogue(grpc_generator::File* file, const Parameters& params) {
    if (file->service_count() == 0) return "";

    std::string output;
    {
        // Scope the output stream so it closes and finalizes output to the string
        auto printer = file->CreatePrinter(&output);
        std::map<std::string, std::string> vars;

        printer->Print(vars, services_header_class_epilogue.data());
    }
    return output;
}

std::string GetServicesHeaderClassDeclaration(grpc_generator::File* file, const Parameters& params) {
    if (file->service_count() == 0) return "";

    std::string output;
    {
        // Scope the output stream so it closes and finalizes output to the string
        auto printer = file->CreatePrinter(&output);
        std::map<std::string, std::string> vars;

        auto packageValue = file->package();
        if (!packageValue.empty()) {
            packageValue = DotsToUnderscores(packageValue);
            vars["PackageRaw"] = packageValue;
        }

        printer->Indent();
        for (int i = 0; i < file->service_count(); ++i) {
            vars["Service"] = file->service(i)->name();
            printer->Print(vars, services_header_pointer_definition.data());
        }
        printer->Outdent();
    }
    return output;
}

std::string GetServicesHeaderEpilogue(grpc_generator::File* file, const Parameters& params) {
    if (file->service_count() == 0) return "";

    std::string output;
    {
        // Scope the output stream so it closes and finalizes output to the string
        auto printer = file->CreatePrinter(&output);
        std::map<std::string, std::string> vars;
        vars["filename_identifier"] = FilenameIdentifier("services.h");
        printer->Print(vars, services_header_epilogue.data());
    }
    return output;
}

// Services source

std::string GetServicesSourcePrologue(grpc_generator::File* file, const Parameters& params) {
    std::string output;
    {
        // Scope the output stream so it closes and finalizes output to the string
        auto printer = file->CreatePrinter(&output);
        std::map<std::string, std::string> vars;

        vars["filename"] = file->filename();
        vars["filename_base"] = file->filename_without_ext();

        printer->Print(vars, services_source_prologue.data());
    }
    return output;
}

std::string GetServicesSourceIncludes(grpc_generator::File* file, const Parameters& params) {
    std::string output;
    {
        // Scope the output stream so it closes and finalizes output to the string
        auto printer = file->CreatePrinter(&output);
        std::map<std::string, std::string> vars;

        vars["filename"] = file->filename();
        vars["filename_base"] = file->filename_without_ext();

        printer->Print(vars, services_source_includes.data());
    }
    return output;
}

std::string GetServicesSourceConstructorPrologue(grpc_generator::File* file, const Parameters& params) {
    std::string output;
    {
        // Scope the output stream so it closes and finalizes output to the string
        auto printer = file->CreatePrinter(&output);
        std::map<std::string, std::string> vars;

        printer->Print(services_source_constructor_begin.data());
    }
    return output;
}

std::string GetServicesSourceConstructorDeclaration(grpc_generator::File* file, const Parameters& params) {
    std::string output;
    {
        // Scope the output stream so it closes and finalizes output to the string
        auto printer = file->CreatePrinter(&output);
        std::map<std::string, std::string> vars;

        auto packageValue = file->package();
        if (!packageValue.empty()) {
            vars["PackageColons"] = DotsToColons(packageValue);
            vars["PackageUnderscores"] = DotsToUnderscores(packageValue);
        }

        printer->Indent();
        for (int i = 0; i < file->service_count(); ++i) {
            vars["Service"] = file->service(i)->name();
            printer->Print(vars, services_source_pointer_definition.data());
        }
        printer->Outdent();
    }
    return output;
}

std::string GetServicesSourceConstructorEpilogue(grpc_generator::File* file, const Parameters& params) {
    std::string output;
    {
        // Scope the output stream so it closes and finalizes output to the string
        auto printer = file->CreatePrinter(&output);
        std::map<std::string, std::string> vars;

        printer->Print(services_source_constructor_end.data());
    }
    return output;
}

std::string GetServicesSourceDestructor(grpc_generator::File* file, const Parameters& params) {
    std::string output;
    {
        // Scope the output stream so it closes and finalizes output to the string
        auto printer = file->CreatePrinter(&output);
        std::map<std::string, std::string> vars;

        printer->Print(services_source_destructor.data());
    }
    return output;
}

std::string GetServicesSourceMethodPrologue(grpc_generator::File* file, const Parameters& params) {
    std::string output;
    {
        // Scope the output stream so it closes and finalizes output to the string
        auto printer = file->CreatePrinter(&output);
        std::map<std::string, std::string> vars;

        printer->Print("\n");
        printer->Print(services_source_method_begin.data());
    }
    return output;
}

std::string GetServicesSourceMethodCall(grpc_generator::File* file, const Parameters& params) {
    std::string output;
    {
        // Scope the output stream so it closes and finalizes output to the string
        auto printer = file->CreatePrinter(&output);
        std::map<std::string, std::string> vars;

        auto packageValue = file->package();
        if (!packageValue.empty()) {
            vars["PackageColons"] = DotsToColons(packageValue);
            vars["PackageUnderscores"] = DotsToUnderscores(packageValue);
        }

        printer->Indent();
        for (int i = 0; i < file->service_count(); ++i) {
            vars["Service"] = file->service(i)->name();
            printer->Print(vars, services_source_method_body.data());
        }
        printer->Outdent();
    }
    return output;
}

std::string GetServicesSourceMethodEpilogue(grpc_generator::File* file, const Parameters& params) {
    std::string output;
    {
        // Scope the output stream so it closes and finalizes output to the string
        auto printer = file->CreatePrinter(&output);
        printer->Print(services_source_method_end.data());
    }
    return output;
}

// Packages XML file

std::string GetPackagesXmlPrologue(grpc_generator::File* file, const Parameters& params) {
    std::string output;
    {
        // Scope the output stream so it closes and finalizes output to the string
        auto printer = file->CreatePrinter(&output);
        printer->Print(packages_xml_prologue.data());
    }
    return output;
}

std::string GetPackagesXmlIncludes(grpc_generator::File* file, const Parameters& params) {
    std::string output;
    {
        // Scope the output stream so it closes and finalizes output to the string.
        auto printer = file->CreatePrinter(&output);
        std::map<std::string, std::string> vars;

        if (!params.additional_header_includes.empty()) {
            PrintIncludes(printer.get(), params.additional_header_includes, false, "");
        }
    }
    return output;
}

std::string GetPackagesXmlEpilogue(grpc_generator::File* file, const Parameters& params) {
    std::string output;
    {
        // Scope the output stream so it closes and finalizes output to the string
        auto printer = file->CreatePrinter(&output);
        printer->Print(packages_xml_epilogue.data());
    }
    return output;
}

std::string GetPackagesXmlMethods(grpc_generator::File* file, const Parameters& params) {
    std::string output;
    do {
        // Scope the output stream so it closes and finalizes output to the string
        auto printer = file->CreatePrinter(&output);
        std::map<std::string, std::string> vars;

        // Package
        auto packageValue = file->package();
        if (!packageValue.empty()) {
            vars["PackageColons"] = packageValue;
        }
        else {
            break;
        }

        if (file->service_count() == 0) break;
        printer->Indent();
        printer->Print(vars, packages_xml_package_begin.data());

        for (int i = 0; i < file->service_count(); ++i) {
            // Service
            auto service = file->service(i);
            vars["Service"] = service->name();
            if (service->method_count() == 0) continue;

            printer->Indent();
            printer->Print(vars, packages_xml_service_begin.data());
            printer->Indent();

            for (int j = 0; j < service->method_count(); ++j) {
                // Method
                auto method = file->service(i)->method(j);
                vars["Method"] = method->name();

                // Method original comments: leading
                auto leadingComments = method->GetLeadingComments("");
                if (!leadingComments.empty()) {
                    vars["LeadingComments"] = grpc_cpp_generator::trim_copy(leadingComments);
                    printer->Print(vars, packages_xml_service_method_leading_comments.data());
                }

                // Method name
                printer->Print(vars, packages_xml_service_method.data());

                // Method original comments: trailing
                auto trailingComments = method->GetTrailingComments("");
                if (!trailingComments.empty()) {
                    vars["TrailingComments"] = grpc_cpp_generator::trim_copy(trailingComments);
                    printer->Print(vars, packages_xml_service_method_trailing_comments.data());
                }
            }

            printer->Outdent();
            printer->Print(packages_xml_service_end.data());
            printer->Outdent();
        }

        printer->Print(packages_xml_package_end.data());
        printer->Outdent();
    } while (false);
    return output;
}

}  // namespace grpc_cpp_generator
