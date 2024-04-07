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

#ifndef GRPC_MOCK_SERVER_LOGGER_H
#define GRPC_MOCK_SERVER_LOGGER_H

#include "grpc_mock_server_export.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/fmt/ostr.h>

#if defined(ANDROID)
#include <spdlog/sinks/android_sink.h>
#endif
#if defined(_WIN32) || defined(_WIN64)
#include <spdlog/sinks/wincolor_sink.h>
#include <spdlog/sinks/msvc_sink.h>
#endif

#define SystemLogger spdlog::get("system")

namespace grpc_mock_server {

GRPC_MOCK_SERVER_LIBRARY_API bool initLogLibrary();
GRPC_MOCK_SERVER_LIBRARY_API void deinitLogLibrary();

}

#endif // GRPC_MOCK_SERVER_LOGGER_H
