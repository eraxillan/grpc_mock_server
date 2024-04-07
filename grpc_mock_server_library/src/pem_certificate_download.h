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

#ifndef GRPC_MOCK_SERVER_PEM_CERTIFICATE_DOWNLOAD_H
#define GRPC_MOCK_SERVER_PEM_CERTIFICATE_DOWNLOAD_H

#include <grpc_mock_server_export.h>

namespace grpc_mock_server {

/// <summary>
/// Downloads a PEM-format certificate from a specified remote host to a specified local file
/// </summary>
/// <param name="remote_host">Remote host address, e.g. "google.ru" or "1.2.3.4"</param>
/// <param name="local_file_path">Local certificate file path to saved into, e.g. "/home/user/test/cert.pem"</param>
/// <returns>Zero if success or non-zero if failure</returns>
extern "C" GRPC_MOCK_SERVER_LIBRARY_API int downloadPemCertificate(const char* remote_host, const char* remote_port, const char* local_file_path);

} // namespace grpc_mock_server

#endif // GRPC_MOCK_SERVER_PEM_CERTIFICATE_DOWNLOAD_H
