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

#include <filesystem>

#include <cmrc/cmrc.hpp>
#include <argparse/argparse.hpp>

#include <grpc_mock_server_configuration.h>
#include <grpc_mock_server_fs_utils.h>
#include <grpc_mock_server_library.h>
#include <grpc_mock_server_logger.h>
#include <pem_certificate_download.h>

CMRC_DECLARE(grpc_mock_server);

class Log {
public:
    Log() {
        grpc_mock_server::initLogLibrary();
    }
    ~Log() {
        grpc_mock_server::deinitLogLibrary();
    }
};

int main(int argc, char* argv[]) {
    Log log;

    // Determine the executable directory
    auto program_directory = std::filesystem::current_path();

    // Validate the program directory
    assert(std::filesystem::exists(program_directory) && std::filesystem::is_directory(program_directory));
    SystemLogger->debug("Program directory: '{}'", program_directory.generic_string());
    grpc_mock_server::setAppDirectory(program_directory.generic_string());

    argparse::ArgumentParser program("grpc_mock_server_example", "1.0.0");
    program.add_argument("-c", "--config").required().help("Specify the configuration file");
    try {
        program.parse_args(argc, argv);
    }
    catch (const std::exception& err) {
        SystemLogger->error("Unable to parse command-line arguments: {}", err.what());
        return 1;
    }

    // Read and the configuration file
    auto config_file_path = program.get<std::string>("config");
    auto config_file_data = grpc_mock_server::readFile(config_file_path);
    if (!Config::instance().parse(config_file_data)) {
        SystemLogger->error("Unable to parse the configuration file '{}'", config_file_path);
        return 1;
    }

    // Validate the configuration file
    if (!Config::instance().haveLocalHostPort()) {
        SystemLogger->error("local_host_port is absent in configuration file '{}'", config_file_path);
        return 1;
    }
    SystemLogger->info("local_host_port={}", Config::instance().localHostPort());
    if (!Config::instance().isOfflineModeEnabled()) {
        if (!Config::instance().haveRemoteHostUrl()) {
            SystemLogger->error("remote_host_url is absent in configuration file '{}'", config_file_path);
            return 1;
        }
        SystemLogger->info("remote_host_url={}", Config::instance().remoteHostUrl());
        if (!Config::instance().haveRemoteHostPort()) {
            SystemLogger->error("remote_host_port is absent in configuration file '{}'", config_file_path);
            return 1;
        }
        SystemLogger->info("remote_host_port={}", Config::instance().remoteHostPort());
    }
    else {
        SystemLogger->warn("Offline mode is enabled");
        SystemLogger->warn("All absent in configuration file functions will return 'not implemented' error");
    }

    auto localHostPort = Config::instance().localHostPort();
    grpc_mock_server::setLocalPort(localHostPort);
    
    if (!Config::instance().isOfflineModeEnabled()) {
        auto remoteHostUrl = Config::instance().remoteHostUrl();
        auto remoteHostPortStr = std::to_string(Config::instance().remoteHostPort());
        auto remoteHostUrlAndPort = remoteHostUrl + ":" + remoteHostPortStr;
        grpc_mock_server::setRemoteHostAndPort(remoteHostUrlAndPort);

        std::string cert_path = (program_directory / "remote_host.crt").generic_string();
        if (grpc_mock_server::downloadPemCertificate(remoteHostUrl.data(), remoteHostPortStr.data(), cert_path.data()) < 0) {
            SystemLogger->error("unable to download PEM certificate from remote host");
            return 1;
        }
        grpc_mock_server::setSslUsage(true);
        auto remote_cert_data = grpc_mock_server::readFile(cert_path);
        grpc_mock_server::setRemoteServerCertificate(remote_cert_data);

        if (!grpc_mock_server::isRemoteServerAvailable()) {
            SystemLogger->error("Remote server '{}' is not available, maybe network problems and/or VPN is not connected yet?", remoteHostUrlAndPort);
            return 1;
        }
    }

    // Validate and load embedded resources
    auto rc_fs = cmrc::grpc_mock_server::get_filesystem();
    assert(rc_fs.exists("assets/server.crt") && rc_fs.is_file("assets/server.crt"));
    assert(rc_fs.exists("assets/server.key") && rc_fs.is_file("assets/server.key"));
    assert(rc_fs.exists("assets/ca.crt") && rc_fs.is_file("assets/ca.crt"));
    auto server_cert_file = rc_fs.open("assets/server.crt");
    auto server_cert_data = std::string(server_cert_file.cbegin(), server_cert_file.cend());
    auto server_key_file = rc_fs.open("assets/server.key");
    auto server_key_data = std::string(server_key_file.cbegin(), server_key_file.cend());
    auto ca_cert_file = rc_fs.open("assets/ca.crt");
    auto ca_cert_data = std::string(ca_cert_file.cbegin(), ca_cert_file.cend());
    grpc_mock_server::setLocalServerCertificate(server_cert_data, server_key_data, ca_cert_data);

    assert(rc_fs.exists("assets/packages.xml") && rc_fs.is_file("assets/packages.xml"));
    auto packages_xml_file = rc_fs.open("assets/packages.xml");
    auto packages_xml_data = std::string(packages_xml_file.cbegin(), packages_xml_file.end());
    grpc_mock_server::setPackagesXmlData(packages_xml_data);

    grpc_mock_server::startServer(Config::instance().isOfflineModeEnabled(), []() {
        SystemLogger->info("Press any key to stop the server");
    });

    // Stop the server after any key will be pressed
    std::cin.get();
    grpc_mock_server::stopServer();

    return 0;
}
