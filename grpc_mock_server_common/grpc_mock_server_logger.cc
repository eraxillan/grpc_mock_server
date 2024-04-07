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

#include "grpc_mock_server_logger.h"

#include <iostream>
#include <memory>

namespace grpc_mock_server {

bool initLogLibrary() {
    try {
        std::cout << "initializing spdlog..." << std::endl;

        // Console multi threaded logger with color
#if defined(ANDROID)
        spdlog::android_logger_mt("system", "gRPC mock server");
#elif defined(_WIN32) || defined(_WIN64)
        auto sink_debug = std::make_shared<spdlog::sinks::msvc_sink_mt>();
        auto sink_console = std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>();
        spdlog::logger logger1("multi_sink", { sink_debug, sink_console });
        spdlog::sinks_init_list sinks = { sink_debug, sink_console };
        auto logger = std::make_shared<spdlog::logger>("system", sinks);
        spdlog::register_logger(logger);
#else
        spdlog::stdout_color_mt("system");
#endif
        // Customize msg format for all messages
        spdlog::set_pattern("[%^%L%$][%D %H:%M:%S.%e][%P:%t] %v");
        spdlog::set_level(spdlog::level::trace);

        spdlog::flush_every(std::chrono::seconds(3));

        SystemLogger->info("spdlog was successfully initialized");
    }
    // Exceptions will only be thrown upon failed logger or sink construction (not during logging)
    catch (const spdlog::spdlog_ex& ex) {
        std::cout << "spdlog init failed: " << ex.what() << std::endl;
        return false;
    }

    return true;
}

void deinitLogLibrary() {
    std::cout << "deinitializing spdlog..." << std::endl;

    // Release and close all loggers
    // NOTE: This is optional (only mandatory if using windows + async log).
    spdlog::shutdown();

    std::cout << "spdlog deinit succeeded" << std::endl;
}

}
