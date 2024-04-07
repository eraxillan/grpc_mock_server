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

#ifndef GRPC_MOCK_SERVER_BUSINESS_LOGIC_H
#define GRPC_MOCK_SERVER_BUSINESS_LOGIC_H

#include <string>
#include <fstream>
#include <atomic>

#include <grpc++/grpc++.h>

 // Generated headers
#include <gmsServices.h>

namespace SQLite { class Database; }

class BusinessLogic {
    bool m_use_ssl = true;
    std::string m_host_url = "";
    int m_port = -1;
    std::string m_database_file_path;
    std::string m_packages_xml_data;
    std::unique_ptr<SQLite::Database> m_database;
    std::string m_remote_server_certificate_data;
    std::string m_local_server_cert_data;
    std::string m_local_server_key_data;
    std::string m_local_ca_cert_data;
    std::unique_ptr<grpc::Server> m_server;

    BusinessLogic();
    ~BusinessLogic();

public:
    BusinessLogic(const BusinessLogic&) = delete;
    BusinessLogic &operator=(const BusinessLogic&) = delete;

public:
    static BusinessLogic &getInstance();
    static bool healthCheck(const std::shared_ptr<grpc::Channel> &channel);

    bool isRemoteServerAvailable() const;

    void setDatabaseFilePath(const std::string &database_file_path);
    void setPackagesXmlData(const std::string &packages_xml_data);
    bool openDatabase();
    void closeDatabase();
    void insertHistoryRow(
        time_t time,
        const std::string &method,
        const std::string &request_json,
        int status,
        const std::string &response_json
    );

    void setHostAndPort(const std::string &host_url, int port);
    void setSslUsage(bool use_ssl);
    void setRemoteServerCertificateData(const std::string &data);
    void setLocalServerCertificateData(
        const std::string &server_cert_data,
        const std::string &server_key_data,
        const std::string &ca_cert_data
    );
    std::shared_ptr<grpc::Channel> createRemoteChannel() const;
    std::shared_ptr<grpc::Channel> createLocalChannel() const;

#ifdef ANDROID
    void runServer(JNIEnv* env, jobject obj, jmethodID is_cancelled_mid, int port);
    void stopServer();
#else
    void runServer(std::function<void()> on_started_callback);
    void stopServer();
#endif
};

#endif // GRPC_MOCK_SERVER_BUSINESS_LOGIC_H
