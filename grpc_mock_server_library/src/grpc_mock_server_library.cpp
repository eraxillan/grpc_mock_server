/*
 *
 * Copyright 2018 gRPC authors, 2022-2023 Aleksandr Kamyshnikov
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

#include "grpc_mock_server_library.h"
#include "business_logic.h"

#include <fstream>
#include <sstream>
#include <filesystem>

#ifdef ANDROID

extern "C"
JNIEXPORT jstring JNICALL
Java_name_eraxillan_mockgrpcserver_service_EndlessService_00024Companion_sayHello(
    JNIEnv * env,
    jobject,
    jstring host_raw,
    jint port_raw,
    jstring message_raw
) {
    const char* host_chars = env->GetStringUTFChars(host_raw, (jboolean*) nullptr);
    std::string host(host_chars, env->GetStringUTFLength(host_raw));

    int port = static_cast<int>(port_raw);

    const char* message_chars = env->GetStringUTFChars(message_raw, (jboolean*) nullptr);
    std::string message(message_chars, env->GetStringUTFLength(message_raw));

    const int host_port_buf_size = 1024;
    char host_port[host_port_buf_size] = { 0 };
    snprintf(host_port, host_port_buf_size, "%s:%d", host.c_str(), port);

    // 1) localhost
    BusinessLogic::getInstance().healthCheck(BusinessLogic::getInstance().createLocalhostChannel());
    // 2) remote
    BusinessLogic::getInstance().healthCheck(BusinessLogic::getInstance().createRemoteChannel());
    // 3) localhost + remote
    goods::common::commonServiceImpl healthCheckService(BusinessLogic::getInstance().createRemoteChannel());
    grpc::ServerContext server_context;
    goods::common::HealthCheckResponse response;
    auto status = healthCheckService.HealthCheck(
        &server_context,
        &google::protobuf::Empty::default_instance(),
        &response
    );

    if (!status.ok()) {
        __android_log_print(
            ANDROID_LOG_ERROR,
            APP_NAME,
            "HealthCheck: failure: '%s'",
            status.error_message().data()
        );
    }

    return env->NewStringUTF("Completed");
}

extern "C"
JNIEXPORT void JNICALL
Java_name_eraxillan_mockgrpcserver_service_EndlessService_startServer(
    JNIEnv * env,
    jobject obj_this,
    jint port_raw
) {
    int port = static_cast<int>(port_raw);

    jclass cls = env->GetObjectClass(obj_this);
    jmethodID is_cancelled_mid = env->GetMethodID(
        cls,
        "isRunServerTaskCancelled",
        "()Z"
    );

    BusinessLogic::getInstance().stopServer();
    BusinessLogic::getInstance().runServer(env, obj_this, is_cancelled_mid, port);
}

extern "C"
JNIEXPORT void JNICALL
Java_name_eraxillan_mockgrpcserver_service_EndlessService_stopServer(
    JNIEnv * env,
    jobject
) {
    BusinessLogic::getInstance().stopServer();
}

extern "C"
JNIEXPORT void JNICALL
Java_name_eraxillan_mockgrpcserver_service_EndlessService_00024Companion_loadResources(
    JNIEnv * env,
    jobject,
    jobject manager
) {
    AAssetManager* asset_manager = AAssetManager_fromJava(env, manager);
    AAsset* file = AAssetManager_open(
        asset_manager,
        "remote_host.crt",
        AASSET_MODE_BUFFER
    );
    size_t file_length = AAsset_getLength(file);
    char* file_content = new char[file_length + 1] {0};
    AAsset_read(file, file_content, file_length);
    BusinessLogic::getInstance().setCertificateData(std::string(file_content));
    delete[] file_content;
    file_content = nullptr;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_name_eraxillan_mockgrpcserver_service_EndlessService_00024Companion_healthCheck(
    JNIEnv*,
    jobject
) {
    return BusinessLogic::getInstance().healthCheck(
        BusinessLogic::getInstance().createLocalhostChannel()
    ) ? JNI_TRUE : JNI_FALSE;
}

extern "C"
JNIEXPORT void JNICALL
Java_name_eraxillan_mockgrpcserver_service_EndlessService_00024Companion_setLogDirectory(
    JNIEnv * env, jobject, jstring path
) {
    jboolean is_copy = JNI_FALSE;
    const char* converted_value = env->GetStringUTFChars(path, &is_copy);
    std::string app_directory = converted_value;
    std::__fs::filesystem::path app_directory_path(app_directory);
    auto log_file_path = app_directory_path /= "grpc.txt";
    auto log_file = log_file_path.string();

    BusinessLogic::getInstance().setLogFilePath(log_file);
}

#else

namespace grpc_mock_server {

void setHostAndPort(const std::string &host_url, int port) {
    BusinessLogic::getInstance().setHostAndPort(host_url, port);
}

void setSslUsage(bool use_ssl) {
    BusinessLogic::getInstance().setSslUsage(use_ssl);
}

void setRemoteServerCertificate(const std::string &crt_data) {
    BusinessLogic::getInstance().setRemoteServerCertificateData(crt_data);
}

void setLocalServerCertificate(
    const std::string &server_cert_data,
    const std::string &server_key_data,
    const std::string &ca_cert_data
) {
    BusinessLogic::getInstance().setLocalServerCertificateData(server_cert_data, server_key_data, ca_cert_data);
}

void setAppDirectory(const std::string &app_directory) {
#ifdef ANDROID
    std::__fs::filesystem::path app_directory_path(app_directory);
#else
    std::filesystem::path app_directory_path(app_directory);
#endif

    std::filesystem::path database_path = app_directory_path /= "database.db3";
    BusinessLogic::getInstance().setDatabaseFilePath(database_path.generic_string());
}

void setPackagesXmlData(const std::string &packages_xml_data) {
    BusinessLogic::getInstance().setPackagesXmlData(packages_xml_data);
}

bool isRemoteServerAvailable() {
    return BusinessLogic::getInstance().isRemoteServerAvailable();
}

bool healthCheck() {
    // FIXME: save channel in static variable
    return BusinessLogic::getInstance().healthCheck(
        BusinessLogic::getInstance().createLocalChannel()
    );
}

void startServer(std::function<void()> on_started_callback) {
    //BusinessLogic::getInstance().stopServer();
    BusinessLogic::getInstance().runServer(on_started_callback);
}

void stopServer() {
    BusinessLogic::getInstance().stopServer();
}

} // namespace grpc_mock_server

#endif // ANDROID
