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

#include "business_logic.h"

#include <grpc_mock_server_logger.h>

// Generated headers
#include <gmsServices.h>
#include <common/common.stub.h>

#include <sqlite3.h>
#include <SQLiteCpp/SQLiteCpp.h>
#include <pugixml.hpp>

#ifdef ANDROID
#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>
#endif // ANDROID

namespace {

std::shared_ptr<grpc::ServerCredentials> createLocalServerCredentials(
    bool use_ssl,
    const std::string &server_cert_data,
    const std::string &server_key_data,
    const std::string &ca_cert_data
) {
    if (use_ssl) {
        assert(!server_cert_data.empty());
        assert(!server_key_data.empty());
        assert(!ca_cert_data.empty());

        grpc::SslServerCredentialsOptions ssl_opts;
        ssl_opts.pem_key_cert_pairs.push_back({ server_key_data, server_cert_data });
        ssl_opts.pem_root_certs = ca_cert_data;
        return grpc::SslServerCredentials(ssl_opts);
    }
    else {
        return grpc::InsecureServerCredentials();
    }
}

std::shared_ptr<grpc::ChannelCredentials> createLocalChannelCredentials(bool use_ssl, const std::string &cert_data) {
    if (use_ssl) {
        assert(!cert_data.empty());

        grpc::SslCredentialsOptions ssl_options;
        ssl_options.pem_root_certs = cert_data;
        return grpc::SslCredentials(ssl_options);
    }
    else {
        return grpc::InsecureChannelCredentials();
    }
}

gpr_timespec grpc_timeout_milliseconds_to_deadline(int64_t time_ms) {
    return gpr_time_add(
        gpr_now(GPR_CLOCK_MONOTONIC),
        gpr_time_from_micros(static_cast<int64_t>(1e3) * time_ms, GPR_TIMESPAN)
    );
}

bool parsePackagesXml(const std::string &data, std::vector<std::string> &output) {
    SystemLogger->info("Parsing 'packages.xml'...");

    pugi::xml_document doc;
    pugi::xml_parse_result parser_result = doc.load_buffer(data.data(), data.size());
    if (!parser_result) return false;

    auto xpath_result_nodes = doc.select_nodes("/root/package/service/child::node()");
    for (pugi::xpath_node xpath_node : xpath_result_nodes)
    {
        pugi::xml_node node = xpath_node.node();
        auto serviceNode = node.parent();
        auto packageNode = serviceNode.parent();
        auto full_method_name = std::string(packageNode.attribute("name").as_string())
            + "." + serviceNode.attribute("name").as_string()
            + "/" + node.attribute("name").as_string();
        output.push_back(full_method_name);

        SystemLogger->info("Method '{}' found", full_method_name);
    }

    SystemLogger->info("'packages.xml' was successfully parsed: {} methods found", output.size());
    return !output.empty();
}

} // anonymous namespace

BusinessLogic::BusinessLogic() {
}

BusinessLogic::~BusinessLogic() {
}

BusinessLogic &BusinessLogic::getInstance() {
    static BusinessLogic instance;
    return instance;
}

bool BusinessLogic::isRemoteServerAvailable() const {
    auto channel = createRemoteChannel();
    auto was_connected = channel->WaitForConnected(grpc_timeout_milliseconds_to_deadline(1000));
    auto state = channel->GetState(true);
    return state == GRPC_CHANNEL_READY;
}

void BusinessLogic::setDatabaseFilePath(const std::string &database_file_path) {
    assert(!database_file_path.empty());
    m_database_file_path = database_file_path;
}

void BusinessLogic::setPackagesXmlData(const std::string &packages_xml_data) {
    assert(!packages_xml_data.empty());
    m_packages_xml_data = packages_xml_data;
}

bool BusinessLogic::openDatabase() {
    // Open the database file
    assert(!m_database_file_path.empty());
    m_database.reset(new SQLite::Database(m_database_file_path, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE));
    SystemLogger->info("SQLite database file '{}' opened successfully", m_database_file_path);

    // Create table
    // 1) Create a table for storing gRPC methods list
    m_database->exec("DROP TABLE IF EXISTS methods");
    m_database->exec("CREATE TABLE methods (id INTEGER PRIMARY KEY, name TEXT)");
    // Add the methods from packages.xml
    std::vector<std::string> method_names;
    if (!parsePackagesXml(m_packages_xml_data, method_names)) {
        SystemLogger->error("Unable to parse assets/packages.xml!");
        return false;
    }
    try {
        SQLite::Transaction transaction(*m_database);
        for (const auto &method_name : method_names) {
            auto insert_query = fmt::format("INSERT INTO methods VALUES (NULL, \"{}\")", method_name);
            int nb = m_database->exec(insert_query);
            assert(nb == 1);
        }
        transaction.commit();
    }
    catch (const SQLite::Exception& exc) {
        m_database.reset(nullptr);
        SystemLogger->error("Unable to add new method row to database: {}", exc.getErrorStr());
        return false;
    }

    // 2) Create a table for storing gRPC methods calls history
    m_database->exec("DROP TABLE IF EXISTS history");
    m_database->exec("CREATE TABLE history (id INTEGER PRIMARY KEY, time INTEGER, method_id INTEGER, request_json TEXT, status INTEGER, response_json TEXT)");
    return true;
}

void BusinessLogic::closeDatabase() {
    m_database.reset(nullptr);
}

void BusinessLogic::insertHistoryRow(
    time_t time,
    const std::string &method,
    const std::string &request_json,
    int status,
    const std::string &response_json
) {
    assert(m_database);

    try {
        auto insert_query = fmt::format(
            "INSERT INTO history VALUES (NULL, {}, (SELECT id FROM methods WHERE name = '{}'), '{}', {}, '{}')",
            time,
            method,
            request_json,
            status,
            response_json
        );
        int nb = m_database->exec(insert_query);
        assert(nb == 1);
    }
    catch (const SQLite::Exception &exc) {
        SystemLogger->error("Unable to add new history row to database: {}", exc.getErrorStr());
    }
}

void BusinessLogic::setRemoteServerCertificateData(const std::string &data) {
    assert(!data.empty());
    m_remote_server_certificate_data = data;
}

void BusinessLogic::setLocalServerCertificateData(
    const std::string &server_cert_data,
    const std::string &server_key_data,
    const std::string &ca_cert_data
) {
    assert(!server_cert_data.empty());
    assert(!server_key_data.empty());
    assert(!ca_cert_data.empty());
    
    m_local_server_cert_data = server_cert_data;
    m_local_server_key_data = server_key_data;
    m_local_ca_cert_data = ca_cert_data;
}

void BusinessLogic::setSslUsage(bool use_ssl) {
    m_use_ssl = use_ssl;
}

void BusinessLogic::setHostAndPort(const std::string &host_url, int port) {
    assert(!host_url.empty());
    assert(port != -1);

    m_host_url = host_url;
    m_port = port;
}

std::shared_ptr<grpc::Channel> BusinessLogic::createRemoteChannel() const {
    // NOTE: always use SSL for remote channel for more security, so ignore `m_use_ssl` here
    assert(!m_remote_server_certificate_data.empty());

    grpc::SslCredentialsOptions ssl_options;
    ssl_options.pem_root_certs = m_remote_server_certificate_data;

    auto ssl_credentials = grpc::SslCredentials(ssl_options);
    return grpc::CreateChannel(m_host_url, ssl_credentials);
}

std::shared_ptr<grpc::Channel> BusinessLogic::createLocalChannel() const {
    assert(!m_local_server_cert_data.empty());
    assert(!m_local_server_key_data.empty());
    assert(!m_local_ca_cert_data.empty());

    return grpc::CreateChannel(
        "localhost:50051",
        createLocalChannelCredentials(m_use_ssl, m_local_server_cert_data)
    );
}

#ifdef ANDROID

bool BusinessLogic::healthCheck(const std::shared_ptr<grpc::Channel> &channel) {
    grpc::ClientContext clientContext;
    goods::common::HealthCheckResponse response;
    auto healthServiceStub = goods::common::common::NewStub(channel);
    auto status = healthServiceStub->HealthCheck(
        &clientContext,
        google::protobuf::Empty::default_instance(),
        &response
    );
    if (status.ok()) {
        //__android_log_print(ANDROID_LOG_ERROR, APP_NAME, "HealthCheck: success");
        return true;
    }
    else {
        __android_log_print(
            ANDROID_LOG_ERROR,
            APP_NAME,
            "HealthCheck: failure: '%s'",
            status.error_message().data()
        );
        return false;
    }
}

void BusinessLogic::runServer(JNIEnv* env, jobject obj, jmethodID is_cancelled_mid, int port) {
    const int host_port_buf_size = 1024;
    char host_port[host_port_buf_size] = { 0 };
    snprintf(host_port, host_port_buf_size, "0.0.0.0:%d", port);

    GrpcServices services(createRemoteChannel());
    grpc::ServerBuilder builder;
    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(host_port, grpc::InsecureServerCredentials());
    // Register "service" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *synchronous* service.
    services.registerServices(builder);

    // Finally assemble the server.
    __android_log_print(ANDROID_LOG_ERROR, APP_NAME, "Server: starting");
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    openLogFile();
    __android_log_print(ANDROID_LOG_ERROR, APP_NAME, "Server: started");
    while (!m_stop_server_flag.load()) {
        // Check with the Java code to see if the user has requested the server stop
        // or the app is no longer in the foreground
        jboolean is_cancelled = env->CallBooleanMethod(obj, is_cancelled_mid);
        if (is_cancelled == JNI_TRUE) {
            __android_log_print(ANDROID_LOG_ERROR, APP_NAME, "Server: stopping");
            m_stop_server_flag = true;
        }
    }
    __android_log_print(ANDROID_LOG_ERROR, APP_NAME, "Server: stopped");
    closeLogFile();
}

void BusinessLogic::stopServer() {
    m_stop_server_flag = false;
    closeLogFile();
}

#else

bool BusinessLogic::healthCheck(const std::shared_ptr<grpc::Channel> &channel) {
    grpc::ClientContext clientContext;
    goods::common::HealthCheckResponse response;
    auto healthServiceStub = goods::common::common::NewStub(channel);
    auto status = healthServiceStub->HealthCheck(
        &clientContext,
        google::protobuf::Empty::default_instance(),
        &response
    );
    if (status.ok()) {
        SystemLogger->trace("HealthCheck: success");
        return true;
    }
    else {
        SystemLogger->error("HealthCheck: failure: '{}'", status.error_message());
        return false;
    }
}

void BusinessLogic::runServer(std::function<void()> on_started_callback) {
    assert(!m_host_url.empty());
    assert(m_port != -1);

    auto server_thread = std::thread([=, this]() {
        const int host_port_buf_size = 1024;
        char host_port[host_port_buf_size] = { 0 };
        snprintf(host_port, host_port_buf_size, "0.0.0.0:%d", m_port);

        GrpcServices services(createRemoteChannel());
        grpc::ServerBuilder builder;

        // Set the default compression algorithm for the server.
        builder.SetDefaultCompressionAlgorithm(GRPC_COMPRESS_GZIP);

        // Listen on the given address without any authentication mechanism.
        builder.AddListeningPort(
            host_port,
            createLocalServerCredentials(
                m_use_ssl,
                m_local_server_cert_data,
                m_local_server_key_data,
                m_local_ca_cert_data
            )
        );

        // Register "service" as the instance through which we'll communicate with
        // clients. In this case it corresponds to an *synchronous* service.
        services.registerServices(builder);

        // Finally assemble the server
        if (!openDatabase()) {
            SystemLogger->error("Unable to open database!");
            return;
        }

        SystemLogger->info("Server is starting...");

        m_server = builder.BuildAndStart();
        SystemLogger->info("Server was started");
        SystemLogger->info("Server is listening on port {}", host_port);

        on_started_callback();

        // Wait for the server to shutdown. Note that some other thread must be
        // responsible for shutting down the server for this call to ever return
        m_server->Wait();

        m_server.reset(nullptr);
        SystemLogger->info("Server was stopped");
        closeDatabase();
    });

    server_thread.detach();
}

void BusinessLogic::stopServer() {
    if (!m_server) { assert(0); return; }
    m_server->Shutdown();
}

#endif
