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

#ifndef GRPC_MOCK_SERVER_FS_UTILS_H
#define GRPC_MOCK_SERVER_FS_UTILS_H

#include "grpc_mock_server_export.h"

#include <string_view>
#include <fstream>

namespace grpc_mock_server {

GRPC_MOCK_SERVER_LIBRARY_API auto readFile(std::string_view path) -> std::string;

} // namespace grpc_mock_server

#endif // GRPC_MOCK_SERVER_FS_UTILS_H
