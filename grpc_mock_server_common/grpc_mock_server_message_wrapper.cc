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

#include "grpc_mock_server_message_wrapper.h"
#include "grpc_mock_server_logger.h"

#include <regex>
#include <ranges>
#include <peglib.h>

std::string MessageWrapper::getBooleanValueAsString(
    const google::protobuf::Message* message,
    const google::protobuf::FieldDescriptor* field_descriptor,
    const google::protobuf::Reflection* reflection
) {
    if (field_descriptor->is_repeated()) {
        std::string result;
        result += "[";
        int repeated_message_count = reflection->FieldSize(*message, field_descriptor);
        for (int i = 0; i < repeated_message_count; i++) {
            result += reflection->GetRepeatedBool(*message, field_descriptor, i) ? "true" : "false";
            result += (i != repeated_message_count - 1) ? "," : "";
        }
        result += "]";
        return result;
    }
    else {
        return reflection->GetBool(*message, field_descriptor) ? "true" : "false";
    }
}

void MessageWrapper::setBooleanValue(
    google::protobuf::Message* message,
    const google::protobuf::FieldDescriptor* field_descriptor,
    const google::protobuf::Reflection* reflection,
    const bool value
) {
    if (field_descriptor->is_repeated()) {
        int repeated_message_count = reflection->FieldSize(*message, field_descriptor);
        for (int i = 0; i < repeated_message_count; i++) {
            reflection->SetRepeatedBool(message, field_descriptor, i, value);
        }
    }
    else {
        reflection->SetBool(message, field_descriptor, value);
    }
}

std::string MessageWrapper::getFloatValueAsString(
    const google::protobuf::Message* message,
    const google::protobuf::FieldDescriptor* field_descriptor,
    const google::protobuf::Reflection* reflection
) {
    if (field_descriptor->is_repeated()) {
        std::string result;
        result += "[";
        int repeated_message_count = reflection->FieldSize(*message, field_descriptor);
        for (int i = 0; i < repeated_message_count; i++) {
            result += std::to_string(reflection->GetRepeatedFloat(*message, field_descriptor, i));
            result += (i != repeated_message_count - 1) ? "," : "";
        }
        result += "]";
        return result;
    }
    else {
        return std::to_string(reflection->GetFloat(*message, field_descriptor));
    }
}

void MessageWrapper::setFloatValue(
    google::protobuf::Message* message,
    const google::protobuf::FieldDescriptor* field_descriptor,
    const google::protobuf::Reflection* reflection,
    const float value
) {
    if (field_descriptor->is_repeated()) {
        int repeated_message_count = reflection->FieldSize(*message, field_descriptor);
        for (int i = 0; i < repeated_message_count; i++) {
            reflection->SetRepeatedFloat(message, field_descriptor, i, value);
        }
    }
    else {
        reflection->SetFloat(message, field_descriptor, value);
    }
}

std::string MessageWrapper::getDoubleValueAsString(
    const google::protobuf::Message* message,
    const google::protobuf::FieldDescriptor* field_descriptor,
    const google::protobuf::Reflection* reflection
) {
    if (field_descriptor->is_repeated()) {
        std::string result;
        result += "[";
        int repeated_message_count = reflection->FieldSize(*message, field_descriptor);
        for (int i = 0; i < repeated_message_count; i++) {
            result += std::to_string(reflection->GetRepeatedDouble(*message, field_descriptor, i));
            result += (i != repeated_message_count - 1) ? "," : "";
        }
        result += "]";
        return result;
    }
    else {
        return std::to_string(reflection->GetDouble(*message, field_descriptor));
    }
}

void MessageWrapper::setDoubleValue(
    google::protobuf::Message* message,
    const google::protobuf::FieldDescriptor* field_descriptor,
    const google::protobuf::Reflection* reflection,
    const double value
) {
    if (field_descriptor->is_repeated()) {
        int repeated_message_count = reflection->FieldSize(*message, field_descriptor);
        for (int i = 0; i < repeated_message_count; i++) {
            reflection->SetRepeatedDouble(message, field_descriptor, i, value);
        }
    }
    else {
        reflection->SetDouble(message, field_descriptor, value);
    }
}

std::string MessageWrapper::getEnumValueAsString(
    const google::protobuf::Message* message,
    const google::protobuf::FieldDescriptor* field_descriptor,
    const google::protobuf::Reflection* reflection
) {
    std::string result;
    if (field_descriptor->is_repeated()) {
        std::string result;
        result += "[";
        int repeated_message_count = reflection->FieldSize(*message, field_descriptor);
        for (int i = 0; i < repeated_message_count; i++) {
            result += reflection->GetRepeatedEnum(*message, field_descriptor, i)->name();
            result += ":";
            result += std::to_string(reflection->GetRepeatedEnumValue(*message, field_descriptor, i));
            result += (i != repeated_message_count - 1) ? "," : "";
        }
        result += "]";
    }
    else {
        result += reflection->GetEnum(*message, field_descriptor)->name();
        result += ":";
        result += std::to_string(reflection->GetEnumValue(*message, field_descriptor));
    }
    return result;
}

void MessageWrapper::setEnumValue(
    google::protobuf::Message* message,
    const google::protobuf::FieldDescriptor* field_descriptor,
    const google::protobuf::Reflection* reflection,
    const std::string& value_string
) {
    int value_int{ -1 };

    auto enum_descriptor = field_descriptor->enum_type();
    for (int i = 0; i < enum_descriptor->value_count(); i++) {
        auto evd = enum_descriptor->value(i);
        const auto& name = evd->name();
        auto number = evd->number();

        if (name == value_string) {
            value_int = number;
            break;
        }
    }
    if (value_int == -1) {
        std::cout << "ERROR: invalid enum value " << value_int << std::endl;
        return;
    }

    if (field_descriptor->is_repeated()) {
        int repeated_message_count = reflection->FieldSize(*message, field_descriptor);
        for (int i = 0; i < repeated_message_count; i++) {
            reflection->SetRepeatedEnumValue(message, field_descriptor, i, value_int);
        }
    }
    else {
        reflection->SetEnumValue(message, field_descriptor, value_int);
    }
}

std::string MessageWrapper::getInt32ValueAsString(
    const google::protobuf::Message* message,
    const google::protobuf::FieldDescriptor* field_descriptor,
    const google::protobuf::Reflection* reflection
) {
    if (field_descriptor->is_repeated()) {
        std::string result;
        result += "[";
        int repeated_message_count = reflection->FieldSize(*message, field_descriptor);
        for (int i = 0; i < repeated_message_count; i++) {
            result += std::to_string(reflection->GetRepeatedInt32(*message, field_descriptor, i));
            result += (i != repeated_message_count - 1) ? "," : "";
        }
        result += "]";
        return result;
    }
    else {
        return std::to_string(reflection->GetInt32(*message, field_descriptor));
    }
}

void MessageWrapper::setInt32Value(
    google::protobuf::Message* message,
    const google::protobuf::FieldDescriptor* field_descriptor,
    const google::protobuf::Reflection* reflection,
    const int32_t value
) {
    if (field_descriptor->is_repeated()) {
        int repeated_message_count = reflection->FieldSize(*message, field_descriptor);
        for (int i = 0; i < repeated_message_count; i++) {
            reflection->SetRepeatedInt32(message, field_descriptor, i, value);
        }
    }
    else {
        reflection->SetInt32(message, field_descriptor, value);
    }
}

std::string MessageWrapper::getInt64ValueAsString(
    const google::protobuf::Message* message,
    const google::protobuf::FieldDescriptor* field_descriptor,
    const google::protobuf::Reflection* reflection
) {
    if (field_descriptor->is_repeated()) {
        std::string result;
        result += "[";
        int repeated_message_count = reflection->FieldSize(*message, field_descriptor);
        for (int i = 0; i < repeated_message_count; i++) {
            result += std::to_string(reflection->GetRepeatedInt64(*message, field_descriptor, i));
            result += (i != repeated_message_count - 1) ? "," : "";
        }
        result += "]";
        return result;
    }
    else {
        return std::to_string(reflection->GetInt64(*message, field_descriptor));
    }
}

void MessageWrapper::setInt64Value(
    google::protobuf::Message* message,
    const google::protobuf::FieldDescriptor* field_descriptor,
    const google::protobuf::Reflection* reflection,
    const int64_t value
) {
    if (field_descriptor->is_repeated()) {
        int repeated_message_count = reflection->FieldSize(*message, field_descriptor);
        for (int i = 0; i < repeated_message_count; i++) {
            reflection->SetRepeatedInt64(message, field_descriptor, i, value);
        }
    }
    else {
        reflection->SetInt64(message, field_descriptor, value);
    }
}

std::string MessageWrapper::getUInt32ValueAsString(
    const google::protobuf::Message* message,
    const google::protobuf::FieldDescriptor* field_descriptor,
    const google::protobuf::Reflection* reflection
) {
    if (field_descriptor->is_repeated()) {
        std::string result;
        result += "[";
        int repeated_message_count = reflection->FieldSize(*message, field_descriptor);
        for (int i = 0; i < repeated_message_count; i++) {
            result += std::to_string(reflection->GetRepeatedUInt32(*message, field_descriptor, i));
            result += (i != repeated_message_count - 1) ? "," : "";
        }
        result += "]";
        return result;
    }
    else {
        return std::to_string(reflection->GetUInt32(*message, field_descriptor));
    }
}

void MessageWrapper::setUInt32Value(
    google::protobuf::Message* message,
    const google::protobuf::FieldDescriptor* field_descriptor,
    const google::protobuf::Reflection* reflection,
    const uint32_t value
) {
    if (field_descriptor->is_repeated()) {
        int repeated_message_count = reflection->FieldSize(*message, field_descriptor);
        for (int i = 0; i < repeated_message_count; i++) {
            reflection->SetRepeatedUInt32(message, field_descriptor, i, value);
        }
    }
    else {
        reflection->SetUInt32(message, field_descriptor, value);
    }
}

std::string MessageWrapper::getUInt64ValueAsString(
    const google::protobuf::Message* message,
    const google::protobuf::FieldDescriptor* field_descriptor,
    const google::protobuf::Reflection* reflection
) {
    if (field_descriptor->is_repeated()) {
        std::string result;
        result += "[";
        int repeated_message_count = reflection->FieldSize(*message, field_descriptor);
        for (int i = 0; i < repeated_message_count; i++) {
            result += std::to_string(reflection->GetRepeatedUInt64(*message, field_descriptor, i));
            result += (i != repeated_message_count - 1) ? "," : "";
        }
        result += "]";
        return result;
    }
    else {
        return std::to_string(reflection->GetUInt64(*message, field_descriptor));
    }
}

void MessageWrapper::setUInt64Value(
    google::protobuf::Message* message,
    const google::protobuf::FieldDescriptor* field_descriptor,
    const google::protobuf::Reflection* reflection,
    const uint64_t value
) {
    if (field_descriptor->is_repeated()) {
        int repeated_message_count = reflection->FieldSize(*message, field_descriptor);
        for (int i = 0; i < repeated_message_count; i++) {
            reflection->SetRepeatedUInt64(message, field_descriptor, i, value);
        }
    }
    else {
        reflection->SetUInt64(message, field_descriptor, value);
    }
}

std::string MessageWrapper::getStringValueAsString(
    const google::protobuf::Message* message,
    const google::protobuf::FieldDescriptor* field_descriptor,
    const google::protobuf::Reflection* reflection
) {
    if (field_descriptor->is_repeated()) {
        std::string result;
        result += "[";
        int repeated_message_count = reflection->FieldSize(*message, field_descriptor);
        for (int i = 0; i < repeated_message_count; i++) {
            result += reflection->GetRepeatedString(*message, field_descriptor, i);
            result += (i != repeated_message_count - 1) ? "," : "";
        }
        result += "]";
        return result;
    }
    else {
        return reflection->GetString(*message, field_descriptor);
    }
}

void MessageWrapper::setStringValue(
    google::protobuf::Message* message,
    const google::protobuf::FieldDescriptor* field_descriptor,
    const google::protobuf::Reflection* reflection,
    const std::string& value_string
) {
    if (field_descriptor->is_repeated()) {
        int repeated_message_count = reflection->FieldSize(*message, field_descriptor);
        for (int i = 0; i < repeated_message_count; i++) {
            reflection->SetRepeatedString(message, field_descriptor, i, value_string);
        }
    }
    else {
        reflection->SetString(message, field_descriptor, value_string);
    }
}

void MessageWrapper::setValue(
    google::protobuf::Message* message,
    const google::protobuf::FieldDescriptor* field_descriptor,
    const google::protobuf::Reflection* reflection,
    const ValueWrapper& value
) {
    switch (field_descriptor->type()) {
    case google::protobuf::FieldDescriptor::TYPE_BOOL: {
        assert(std::holds_alternative<bool>(value));
        bool value_boolean = std::get<bool>(value);

        setBooleanValue(message, field_descriptor, reflection, value_boolean);
        break;
    }
    case google::protobuf::FieldDescriptor::TYPE_FLOAT: {
        assert(std::holds_alternative<double>(value));
        float value_float = static_cast<float>(std::get<double>(value));

        setFloatValue(message, field_descriptor, reflection, value_float);
        break;
    }
    case google::protobuf::FieldDescriptor::TYPE_DOUBLE: {
        assert(std::holds_alternative<double>(value));
        double value_double = std::get<double>(value);

        setDoubleValue(message, field_descriptor, reflection, value_double);
        break;
    }
    case google::protobuf::FieldDescriptor::TYPE_ENUM: {
        assert(std::holds_alternative<std::string>(value));
        std::string value_string = std::get<std::string>(value);

        setEnumValue(message, field_descriptor, reflection, value_string);
        break;
    }
    case google::protobuf::FieldDescriptor::TYPE_INT32: {
        // TODO: dangerous cast
        assert(std::holds_alternative<int64_t>(value));
        int32_t value_int = static_cast<int32_t>(std::get<int64_t>(value));

        setInt32Value(message, field_descriptor, reflection, value_int);
        break;
    }
    case google::protobuf::FieldDescriptor::TYPE_INT64: {
        assert(std::holds_alternative<int64_t>(value));
        int64_t value_int = std::get<int64_t>(value);

        setInt64Value(message, field_descriptor, reflection, value_int);
        break;
    }
    case google::protobuf::FieldDescriptor::TYPE_UINT32: {
        assert(std::holds_alternative<int64_t>(value));
        uint32_t value_int = static_cast<uint32_t>(std::get<int64_t>(value));

        setUInt32Value(message, field_descriptor, reflection, value_int);
        break;
    }
    case google::protobuf::FieldDescriptor::TYPE_UINT64: {
        assert(std::holds_alternative<int64_t>(value));
        uint64_t value_int = std::get<int64_t>(value);

        setUInt64Value(message, field_descriptor, reflection, value_int);
        break;
    }
    case google::protobuf::FieldDescriptor::TYPE_STRING: {
        assert(std::holds_alternative<std::string>(value));
        std::string value_string = std::get<std::string>(value);

        setStringValue(message, field_descriptor, reflection, value_string);
        break;
    }
    default: {
        assert(false);
        break;
    }
    }
}

std::string MessageWrapper::getValueString(
    google::protobuf::Message* message,
    const google::protobuf::FieldDescriptor* field_descriptor,
    const google::protobuf::Reflection* reflection
) {
    switch (field_descriptor->type()) {
    case google::protobuf::FieldDescriptor::TYPE_BOOL: {
        return getBooleanValueAsString(message, field_descriptor, reflection);
    }
    case google::protobuf::FieldDescriptor::TYPE_FLOAT: {
        return getFloatValueAsString(message, field_descriptor, reflection);
    }
    case google::protobuf::FieldDescriptor::TYPE_DOUBLE: {
        return getDoubleValueAsString(message, field_descriptor, reflection);
    }
    case google::protobuf::FieldDescriptor::TYPE_ENUM: {
        return getEnumValueAsString(message, field_descriptor, reflection);
    }
    case google::protobuf::FieldDescriptor::TYPE_INT32: {
        return getInt32ValueAsString(message, field_descriptor, reflection);
    }
    case google::protobuf::FieldDescriptor::TYPE_INT64: {
        return getInt64ValueAsString(message, field_descriptor, reflection);
    }
    case google::protobuf::FieldDescriptor::TYPE_UINT32: {
        return getUInt32ValueAsString(message, field_descriptor, reflection);
    }
    case google::protobuf::FieldDescriptor::TYPE_UINT64: {
        return getUInt64ValueAsString(message, field_descriptor, reflection);
    }
    case google::protobuf::FieldDescriptor::TYPE_STRING: {
        return getStringValueAsString(message, field_descriptor, reflection);
    }
    default: {
        assert(false);
        return "";
    }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string MessageWrapper::normalizePathToken(const std::string& current_path_token_raw, const bool is_repeated) {
    std::string current_path_token = current_path_token_raw;
    if (is_repeated) {
        current_path_token = current_path_token_raw.substr(0, current_path_token_raw.size() - 2);
    }
    return current_path_token;
}

std::vector<MessageWrapper::MessageData> MessageWrapper::getMockDataRequestMessages(
    const google::protobuf::Message& input_message,
    const RequestWithValue& request
) {
    std::vector<MessageWrapper::MessageData> result;

    auto* input_message_copy = const_cast<google::protobuf::Message*>(&input_message);
    std::vector<MessageWrapper::MessageData> current_messages{
        MessageWrapper::MessageData(input_message_copy, input_message_copy->GetDescriptor(), input_message_copy->GetReflection())
    };

    for (const auto& current_path_token_raw : request.first) {
        bool is_repeated = current_path_token_raw.ends_with("[]");
        std::string current_path_token = normalizePathToken(current_path_token_raw, is_repeated);

        std::vector<MessageWrapper::MessageData> temp_messages;
        for (auto& current_message_data : current_messages) {
            current_message_data.field_descriptor = current_message_data.descriptor->FindFieldByName(current_path_token);
            assert(is_repeated == current_message_data.field_descriptor->is_repeated());
            if (is_repeated) {
                int repeated_message_count = current_message_data.reflection->FieldSize(
                    *current_message_data.message,
                    current_message_data.field_descriptor
                );

                for (int i = 0; i < repeated_message_count; i++) {
                    auto repeated_message = current_message_data.reflection->MutableRepeatedMessage(
                        current_message_data.message,
                        current_message_data.field_descriptor,
                        i
                    );
                    auto repeated_message_descriptor = repeated_message->GetDescriptor();
                    auto repeated_message_reflection = repeated_message->GetReflection();
                    temp_messages.push_back(
                        MessageWrapper::MessageData(
                            repeated_message,
                            repeated_message_descriptor,
                            repeated_message_reflection
                        )
                    );
                }
            }
            else {
                if (current_path_token == request.first.back()) {
                    result = current_messages;
                }
                else {
                    auto temp_message = current_message_data.reflection->MutableMessage(
                        current_message_data.message,
                        current_message_data.field_descriptor
                    );
                    auto temp_message_descriptor = temp_message->GetDescriptor();
                    auto temp_message_reflection = temp_message->GetReflection();
                    temp_messages.push_back(
                        MessageWrapper::MessageData(temp_message, temp_message_descriptor, temp_message_reflection)
                    );
                }
            }
        }
        temp_messages.swap(current_messages);
    }

    return result;
}

auto MessageWrapper::parse(const std::string& grammar, const std::string& program) -> std::optional<std::vector<RequestWithValue>> {
    peg::parser parser(grammar);
    assert(static_cast<bool>(parser) == true);

    parser["program"] = [](const peg::SemanticValues& vs) {
        std::vector<RequestWithValue> result;
        for (int i = 0; i < vs.size(); i++) {
            RequestWithValue value = std::any_cast<RequestWithValue>(vs[i]);
            result.push_back(value);
        }
        return result;
        };

    parser["statement"] = [](const peg::SemanticValues& vs) {
        // Request
        std::vector <std::string> request_path;
        if (vs[0].type().hash_code() == typeid(std::vector<std::string>).hash_code()) {
            request_path = std::any_cast<std::vector<std::string>>(vs[0]);
            std::string result;
            for (int i = 0; i < request_path.size(); i++) {
                result += request_path[i];
                if (i != request_path.size() - 1) {
                    result += ".";
                }
            }
            std::string result_trim = std::regex_replace(result, std::regex{ R"(^\s+|\s+$)" }, "");
            std::cout << result_trim << " = ";
        }
        else {
            assert(0);
        }

        // Value
        ValueWrapper value;
        // null
        if (vs[1].type().hash_code() == typeid(nullptr).hash_code()) {
            std::cout << nullptr << " : null";

            value = nullptr;
            assert(std::holds_alternative<nullptr_t>(value));
            assert(std::get<nullptr_t>(value) == nullptr);
        }
        // boolean
        else if (vs[1].type().hash_code() == typeid(bool).hash_code()) {
            bool boolean_value = std::any_cast<bool>(vs[1]);
            std::cout << (boolean_value ? "true" : "false") << " : boolean";

            value = boolean_value;
            assert(std::holds_alternative<bool>(value));
            assert(std::get<bool>(value) == boolean_value);
        }
        // number (int)
        else if (vs[1].type().hash_code() == typeid(int).hash_code()) {
            int int_value = std::any_cast<int>(vs[1]);
            std::cout << int_value << " : int";

            value = int_value;
            assert(std::holds_alternative<int64_t>(value));
            assert(std::get<int64_t>(value) == int_value);
        }
        // number (double)
        else if (vs[1].type().hash_code() == typeid(double).hash_code()) {
            double double_value = std::any_cast<double>(vs[1]);
            std::cout << double_value << " : double";

            value = double_value;
            assert(std::holds_alternative<double>(value));
            assert(std::get<double>(value) == double_value);
        }
        // string
        else if (vs[1].type().hash_code() == typeid(std::string).hash_code()) {
            std::string string_value = std::any_cast<std::string>(vs[1]);
            std::cout << "\"" << string_value << "\"" << " : string";

            value = string_value;
            assert(std::holds_alternative<std::string>(value));
            assert(std::get<std::string>(value) == string_value);
        }
        // enum
        else if (vs[1].type().hash_code() == typeid(EnumWrapper).hash_code()) {
            EnumWrapper wrapper_value = std::any_cast<EnumWrapper>(vs[1]);
            std::cout << wrapper_value.name << " : enum";

            value = wrapper_value;
            assert(std::holds_alternative<EnumWrapper>(value));
            assert(std::get<EnumWrapper>(value).name == wrapper_value.name);
        }
        else {
            assert(0);
        }

        std::cout << std::endl;
        return RequestWithValue(request_path, value);
        };

    parser["request"] = [](const peg::SemanticValues& vs) {
        std::vector<std::string> result;
        for (int i = 0; i < vs.size(); i++) {
            result.push_back(std::any_cast<std::string>(vs[i]));
        }
        return result;
        };

    parser["ident"] = [](const peg::SemanticValues& vs) {
        return vs.token_to_string();
        };

    parser["null"] = [](const peg::SemanticValues& vs) {
        return nullptr;
        };

    parser["boolean"] = [](const peg::SemanticValues& vs) {
        // Case-insensitive compare with "true" string
        std::string value_string = vs.token_to_string();
        bool value_boolean = std::ranges::equal(
            value_string,
            std::string("true"), [value_string](unsigned char a, unsigned char b) {
                return std::tolower(a) == std::tolower(b);
            }
        );
        return value_boolean;
        };

    parser["float"] = [](const peg::SemanticValues& vs) {
        auto s = vs.token_to_number<double>();
        return s;
        };

    parser["int"] = [](const peg::SemanticValues& vs) {
        auto s = vs.token_to_number<int>();
        return s;
        };

    parser["string"] = [](const peg::SemanticValues& vs) {
        auto s = vs.token_to_string();
        return s;
        };

    parser["enum"] = [](const peg::SemanticValues& vs) {
        auto s = vs.token_to_string();
        return EnumWrapper(s);
        };

    parser.enable_packrat_parsing();

    std::vector<RequestWithValue> result;
    bool parse_result = parser.parse(program, result);

    std::cout << std::endl;

    return parse_result ? std::optional<std::vector<RequestWithValue>>(result) : std::nullopt;
}

void MessageWrapper::eval(const google::protobuf::Message& root_message, const std::string& grammar, const std::string& program) {
    auto request_with_value_collection_opt = parse(grammar, program);
    if (!request_with_value_collection_opt.has_value()) {
        assert(false);
        return;
    }

    const auto& request_with_value_collection = request_with_value_collection_opt.value();
    for (const auto& request_with_value : request_with_value_collection) {
        auto messages = getMockDataRequestMessages(root_message, request_with_value);

        for (const auto& message_data : messages) {

            auto old_value = getValueString(message_data.message, message_data.field_descriptor, message_data.reflection);
            setValue(message_data.message, message_data.field_descriptor, message_data.reflection, request_with_value.second);
            auto new_value = getValueString(message_data.message, message_data.field_descriptor, message_data.reflection);

            std::cout << "OLD: " << old_value << std::endl;
            std::cout << "NEW: " << new_value << std::endl;
        }
    }
}
