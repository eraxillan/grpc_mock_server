#include <SQLiteCpp/SQLiteCpp.h>

#include <grpc_mock_server_logger.h>

#include "business_logic.h"

// This function will be called by protobuf compiler generated code
void grpcMockServerMethodCallback(
    time_t time,
    const std::string &method,
    const std::string &request_json,
    int status,
    const std::string &response_json
) {
    if (status == grpc::OK) {
        SystemLogger->info("gRPC method '{}' succeeded", method);
    }
    else {
        SystemLogger->info("gRPC method '{}' failed with code {}", method, status);
    }
    BusinessLogic::getInstance().insertHistoryRow(time, method, request_json, status, response_json);
}
