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

#ifndef GRPC_MOCK_SERVER_MESSAGE_WRAPPER_H
#define GRPC_MOCK_SERVER_MESSAGE_WRAPPER_H

#include "grpc_mock_server_export.h"

#include <grpc++/grpc++.h>
#include <google/protobuf/message.h>

#include <variant>

class GRPC_MOCK_SERVER_LIBRARY_API MessageWrapper {
    static std::string getBooleanValueAsString(
        const google::protobuf::Message* message,
        const google::protobuf::FieldDescriptor* field_descriptor,
        const google::protobuf::Reflection* reflection
    );

    static void setBooleanValue(
        google::protobuf::Message* message,
        const google::protobuf::FieldDescriptor* field_descriptor,
        const google::protobuf::Reflection* reflection,
        const bool value
    );

    static std::string getFloatValueAsString(
        const google::protobuf::Message* message,
        const google::protobuf::FieldDescriptor* field_descriptor,
        const google::protobuf::Reflection* reflection
    );

    static void setFloatValue(
        google::protobuf::Message* message,
        const google::protobuf::FieldDescriptor* field_descriptor,
        const google::protobuf::Reflection* reflection,
        const float value
    );

    static std::string getDoubleValueAsString(
        const google::protobuf::Message* message,
        const google::protobuf::FieldDescriptor* field_descriptor,
        const google::protobuf::Reflection* reflection
    );

    static void setDoubleValue(
        google::protobuf::Message* message,
        const google::protobuf::FieldDescriptor* field_descriptor,
        const google::protobuf::Reflection* reflection,
        const double value
    );

    static std::string getEnumValueAsString(
        const google::protobuf::Message* message,
        const google::protobuf::FieldDescriptor* field_descriptor,
        const google::protobuf::Reflection* reflection
    );

    static void setEnumValue(
        google::protobuf::Message* message,
        const google::protobuf::FieldDescriptor* field_descriptor,
        const google::protobuf::Reflection* reflection,
        const std::string& value_string
    );

    static std::string getInt32ValueAsString(
        const google::protobuf::Message* message,
        const google::protobuf::FieldDescriptor* field_descriptor,
        const google::protobuf::Reflection* reflection
    );

    static void setInt32Value(
        google::protobuf::Message* message,
        const google::protobuf::FieldDescriptor* field_descriptor,
        const google::protobuf::Reflection* reflection,
        const int32_t value
    );

    static std::string getInt64ValueAsString(
        const google::protobuf::Message* message,
        const google::protobuf::FieldDescriptor* field_descriptor,
        const google::protobuf::Reflection* reflection
    );

    static void setInt64Value(
        google::protobuf::Message* message,
        const google::protobuf::FieldDescriptor* field_descriptor,
        const google::protobuf::Reflection* reflection,
        const int64_t value
    );

    static std::string getUInt32ValueAsString(
        const google::protobuf::Message* message,
        const google::protobuf::FieldDescriptor* field_descriptor,
        const google::protobuf::Reflection* reflection
    );

    static void setUInt32Value(
        google::protobuf::Message* message,
        const google::protobuf::FieldDescriptor* field_descriptor,
        const google::protobuf::Reflection* reflection,
        const uint32_t value
    );

    static std::string getUInt64ValueAsString(
        const google::protobuf::Message* message,
        const google::protobuf::FieldDescriptor* field_descriptor,
        const google::protobuf::Reflection* reflection
    );

    static void setUInt64Value(
        google::protobuf::Message* message,
        const google::protobuf::FieldDescriptor* field_descriptor,
        const google::protobuf::Reflection* reflection,
        const uint64_t value
    );

    static std::string getStringValueAsString(
        const google::protobuf::Message* message,
        const google::protobuf::FieldDescriptor* field_descriptor,
        const google::protobuf::Reflection* reflection
    );

    static void setStringValue(
        google::protobuf::Message* message,
        const google::protobuf::FieldDescriptor* field_descriptor,
        const google::protobuf::Reflection* reflection,
        const std::string& value
    );

public:
    struct EnumWrapper;
    struct MessageData;

    using ValueWrapper = std::variant<std::nullptr_t, bool, double, int64_t, std::string, EnumWrapper>;
    using RequestWithValue = std::pair<std::vector<std::string>, ValueWrapper>;

private:
    static std::string normalizePathToken(const std::string& current_path_token_raw, const bool is_repeated);
    static std::vector<MessageData> getMockDataRequestMessages(
        const google::protobuf::Message& input_message,
        const RequestWithValue& request
    );

public:
    static void setValue(
        google::protobuf::Message* message,
        const google::protobuf::FieldDescriptor* field_descriptor,
        const google::protobuf::Reflection* reflection,
        const ValueWrapper& value
    );

    static std::string getValueString(
        google::protobuf::Message* message,
        const google::protobuf::FieldDescriptor* field_descriptor,
        const google::protobuf::Reflection* reflection
    );

    static auto parse(const std::string& grammar, const std::string& program) -> std::optional<std::vector<RequestWithValue>>;
    static void eval(const google::protobuf::Message& root_message, const std::string& grammar, const std::string& program);

public:
    struct EnumWrapper { std::string name; };

    struct MockDataRequest {
        std::vector<std::string> mock_path_tokens;
        std::string mock_value;
    };

    struct MessageData {
        google::protobuf::Message* message;
        const google::protobuf::Descriptor* descriptor;
        const google::protobuf::Reflection* reflection;
        const google::protobuf::FieldDescriptor* field_descriptor = nullptr;
    };
};

#endif // GRPC_MOCK_SERVER_MESSAGE_WRAPPER_H
