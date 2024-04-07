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

#ifndef GRPC_MOCK_SERVER_LIBRARY_H
#define GRPC_MOCK_SERVER_LIBRARY_H

#include <grpc_mock_server_export.h>

#include <iostream>
#include <functional>
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
);

extern "C"
JNIEXPORT void JNICALL
Java_name_eraxillan_mockgrpcserver_service_EndlessService_startServer(
    JNIEnv * env,
    jobject obj_this,
    jint port_raw
);

extern "C"
JNIEXPORT void JNICALL
Java_name_eraxillan_mockgrpcserver_service_EndlessService_stopServer(
    JNIEnv * env,
    jobject
);

extern "C"
JNIEXPORT void JNICALL
Java_name_eraxillan_mockgrpcserver_service_EndlessService_00024Companion_loadResources(
    JNIEnv * env,
    jobject,
    jobject manager
);

extern "C"
JNIEXPORT jboolean JNICALL
Java_name_eraxillan_mockgrpcserver_service_EndlessService_00024Companion_healthCheck(
    JNIEnv*,
    jobject
)

extern "C"
JNIEXPORT void JNICALL
Java_name_eraxillan_mockgrpcserver_service_EndlessService_00024Companion_setLogDirectory(
    JNIEnv * env, jobject, jstring path
);

#else

namespace grpc_mock_server {

// Setters
extern "C" GRPC_MOCK_SERVER_LIBRARY_API void setHostAndPort(const std::string &host_url, int port);
extern "C" GRPC_MOCK_SERVER_LIBRARY_API void setSslUsage(bool use_ssl);
extern "C" GRPC_MOCK_SERVER_LIBRARY_API void setRemoteServerCertificate(const std::string &crt_data);
extern "C" GRPC_MOCK_SERVER_LIBRARY_API void setLocalServerCertificate(
    const std::string &server_cert_data,
    const std::string &server_key_data,
    const std::string &ca_cert_data
);
extern "C" GRPC_MOCK_SERVER_LIBRARY_API void setAppDirectory(const std::string &app_directory);
extern "C" GRPC_MOCK_SERVER_LIBRARY_API void setPackagesXmlData(const std::string &packages_xml_data);

// Actions
extern "C" GRPC_MOCK_SERVER_LIBRARY_API bool isRemoteServerAvailable();
extern "C" GRPC_MOCK_SERVER_LIBRARY_API bool healthCheck();
extern "C" GRPC_MOCK_SERVER_LIBRARY_API void startServer(std::function<void()> on_started_callback);
extern "C" GRPC_MOCK_SERVER_LIBRARY_API void stopServer();

} // namespace grpc_mock_server

#endif // ANDROID

#endif // GRPC_MOCK_SERVER_LIBRARY_H
