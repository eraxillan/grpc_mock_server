#include <src/compiler/cpp_generator.h>
#include <src/compiler/cpp_generator_helpers.h>


namespace grpc_cpp_generator {

std::string GetStubHeaderPrologue(grpc_generator::File* file, const Parameters& params);
std::string GetStubHeaderIncludes(grpc_generator::File* file, const Parameters& params);
std::string GetStubHeaderEpilogue(grpc_generator::File* file, const Parameters& params);
std::string GetStubHeaderServices(grpc_generator::File* file, const Parameters& params);
std::string GetStubSourcePrologue(grpc_generator::File* file, const Parameters& params);
std::string GetStubSourceIncludes(grpc_generator::File* file, const Parameters& params);
std::string GetStubSourceEpilogue(grpc_generator::File* file, const Parameters& params);
std::string GetStubSourceServices(grpc_generator::File* file, const Parameters& params);
std::string GetServicesHeaderPrologue(grpc_generator::File* file, const Parameters& params);
std::string GetServicesHeaderForwardDeclarations(grpc_generator::File* file, const Parameters& params);
std::string GetServicesHeaderPointerDeclarations(grpc_generator::File* file, const Parameters& params);
std::string GetServicesHeaderClassPrologue(grpc_generator::File* file, const Parameters& params);
std::string GetServicesHeaderClassEpilogue(grpc_generator::File* file, const Parameters& params);
std::string GetServicesHeaderClassDeclaration(grpc_generator::File* file, const Parameters& params);
std::string GetServicesHeaderEpilogue(grpc_generator::File* file, const Parameters& params);
std::string GetServicesSourcePrologue(grpc_generator::File* file, const Parameters& params);
std::string GetServicesSourceIncludes(grpc_generator::File* file, const Parameters& params);
std::string GetServicesSourceConstructorPrologue(grpc_generator::File* file, const Parameters& params);
std::string GetServicesSourceConstructorDeclaration(grpc_generator::File* file, const Parameters& params);
std::string GetServicesSourceConstructorEpilogue(grpc_generator::File* file, const Parameters& params);
std::string GetServicesSourceDestructor(grpc_generator::File* file, const Parameters& params);
std::string GetServicesSourceMethodPrologue(grpc_generator::File* file, const Parameters& params);
std::string GetServicesSourceMethodCall(grpc_generator::File* file, const Parameters& params);
std::string GetServicesSourceMethodEpilogue(grpc_generator::File* file, const Parameters& params);
std::string GetPackagesXmlPrologue(grpc_generator::File* file, const Parameters& params);
std::string GetPackagesXmlIncludes(grpc_generator::File* file, const Parameters& params);
std::string GetPackagesXmlEpilogue(grpc_generator::File* file, const Parameters& params);
std::string GetPackagesXmlMethods(grpc_generator::File* file, const Parameters& params);

}  // namespace grpc_cpp_generator
