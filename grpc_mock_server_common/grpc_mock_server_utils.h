/*
 *
 * Copyright 2018 gRPC authors, 2022 Aleksandr Kamyshnikov
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef GRPC_MOCK_SERVER_UTILS_H
#define GRPC_MOCK_SERVER_UTILS_H

#include <memory>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <random>
#include <iomanip>

// gRPC headers
#include <grpc++/grpc++.h>
#include <google/protobuf/message.h>
#include <google/protobuf/util/json_util.h>

// XML parser
#include <pugixml.hpp>

// CMakeRC
#include <cmrc/cmrc.hpp>
CMRC_DECLARE(grpc_mock_server);

// Internal
#include "grpc_mock_server_message_wrapper.h"
#include "grpc_mock_server_fs_utils.h"

inline std::string ToString(const grpc::string_ref& r) {
    return std::string(r.data(), r.size());
}

inline void ltrim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
}

inline void rtrim(std::string& s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
}

inline void trim(std::string& s) {
    rtrim(s);
    ltrim(s);
}

inline std::string ltrim_copy(std::string s) {
    ltrim(s);
    return s;
}

inline std::string rtrim_copy(std::string s) {
    rtrim(s);
    return s;
}

inline std::string trim_copy(std::string s) {
    trim(s);
    return s;
}

// ----------------------------------------------------------------------------------------------------------------------------------------

inline time_t current_unix_time() {
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
}

// ----------------------------------------------------------------------------------------------------------------------------------------

inline std::string message_as_json(const google::protobuf::Message& message) {
    std::string json;
    google::protobuf::util::JsonPrintOptions options;
    //options.add_whitespace = true;
    google::protobuf::util::MessageToJsonString(message, &json, options);
    trim(json);
    if (json != "{}") {
        return json;
    }
    return "";
}

void grpcMockServerMethodCallback(
    time_t time,
    const std::string& method,
    const std::string& request_json,
    int status,
    const std::string& response_json
);

inline void evalRequest(const google::protobuf::Message& root_message, const std::string& request_file_path) {
    auto rc_fs = cmrc::grpc_mock_server::get_filesystem();
    auto grammar_file = rc_fs.open("assets/request_grammar.txt");
    auto grammar_data = std::string(grammar_file.cbegin(), grammar_file.cend());
    auto request_data = grpc_mock_server::readFile(request_file_path);
    MessageWrapper::eval(root_message, grammar_data, request_data);
}

inline grpc::Status fromUtilStatus(const google::protobuf::util::status_internal::Status& status) {
    grpc::StatusCode code { grpc::UNKNOWN };
    switch (status.code()) {
        case google::protobuf::util::status_internal::StatusCode::kOk: {
            code = grpc::OK;
            break;
        }
        case google::protobuf::util::status_internal::StatusCode::kCancelled: {
            code = grpc::CANCELLED;
            break;
        }
        case google::protobuf::util::status_internal::StatusCode::kUnknown: {
            code = grpc::UNKNOWN;
            break;
        }
        case google::protobuf::util::status_internal::StatusCode::kInvalidArgument: {
            code = grpc::INVALID_ARGUMENT;
            break;
        }
        case google::protobuf::util::status_internal::StatusCode::kDeadlineExceeded: {
            code = grpc::DEADLINE_EXCEEDED;
            break;
        }
        case google::protobuf::util::status_internal::StatusCode::kNotFound: {
            code = grpc::NOT_FOUND;
            break;
        }
        case google::protobuf::util::status_internal::StatusCode::kAlreadyExists: {
            code = grpc::ALREADY_EXISTS;
            break;
        }
        case google::protobuf::util::status_internal::StatusCode::kPermissionDenied: {
            code = grpc::PERMISSION_DENIED;
            break;
        }
        case google::protobuf::util::status_internal::StatusCode::kUnauthenticated: {
            code = grpc::UNAUTHENTICATED;
            break;
        }
        case google::protobuf::util::status_internal::StatusCode::kResourceExhausted: {
            code = grpc::RESOURCE_EXHAUSTED;
            break;
        }
        case google::protobuf::util::status_internal::StatusCode::kFailedPrecondition: {
            code = grpc::FAILED_PRECONDITION;
            break;
        }
        case google::protobuf::util::status_internal::StatusCode::kAborted: {
            code = grpc::ABORTED;
            break;
        }
        case google::protobuf::util::status_internal::StatusCode::kOutOfRange: {
            code = grpc::OUT_OF_RANGE;
            break;
        }
        case google::protobuf::util::status_internal::StatusCode::kUnimplemented: {
            code = grpc::UNIMPLEMENTED;
            break;
        }
        case google::protobuf::util::status_internal::StatusCode::kInternal: {
            code = grpc::INTERNAL;
            break;
        }
        case google::protobuf::util::status_internal::StatusCode::kUnavailable: {
            code = grpc::UNAVAILABLE;
            break;
        }
        case google::protobuf::util::status_internal::StatusCode::kDataLoss: {
            code = grpc::DATA_LOSS;
            break;
        }
        default: {
            code = grpc::UNKNOWN;
            break;
        }
    }
    return grpc::Status(code, status.message().as_string());
}

inline std::string getDatasetName(::grpc::ServerContextBase* context) {
    const auto& client_metadata = context->client_metadata();
    
    std::string dataset_name_with_dot;
    auto dataset_range = client_metadata.equal_range("gms_dataset");
    if (dataset_range.first != dataset_range.second) {
        dataset_name_with_dot = ToString(dataset_range.first->second);
        dataset_name_with_dot += ".";
    }

    return dataset_name_with_dot;
}

#endif  // GRPC_MOCK_SERVER_UTILS_H
