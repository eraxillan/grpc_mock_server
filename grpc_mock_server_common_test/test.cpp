#include <catch2/catch_test_macros.hpp>

#include <grpc_mock_server_utils.h>
#include <grpc_mock_server_fs_utils.h>
#include <grpc_mock_server_configuration.h>
#include <google/protobuf/message.h>
#include <grpcpp/impl/codegen/metadata_map.h>
#include <grpc/impl/codegen/gpr_types.h>
#include "generated_code/test.pb.h"

TEST_CASE("ToString", "[utils]") {
    SECTION("null") {
        REQUIRE(ToString(grpc::string_ref()) == "");
    }
    SECTION("empty C-string") {
        const char* test = "";
        REQUIRE(ToString(grpc::string_ref(test)) == "");
    }
    SECTION("empty std::string") {
        std::string test = "";
        REQUIRE(ToString(grpc::string_ref(test)) == "");
    }
    SECTION("non-empty C-string") {
        const char* test = "test";
        REQUIRE(ToString(grpc::string_ref(test)) == "test");
    }
    SECTION("non-empty std::string") {
        std::string test = "test";
        REQUIRE(ToString(grpc::string_ref(test)) == "test");
    }
}

TEST_CASE("ltrim", "[utils]") {
    SECTION("empty string") {
        std::string test = "";
        REQUIRE(ltrim_copy(test) == "");
    }
    SECTION("trimming string with whitespaces on the left side") {
        std::string test = "   test";
        REQUIRE(ltrim_copy(test) == "test");
    }
    SECTION("trimming string without whitespaces on the left side") {
        std::string test = "test";
        REQUIRE(ltrim_copy(test) == "test");
    }
    SECTION("trimming string containing only whitespaces leads to empty string") {
        std::string test = "    ";
        REQUIRE(ltrim_copy(test).empty());
    }
}

TEST_CASE("rtrim", "[utils]") {
    SECTION("empty string") {
        std::string test = "";
        REQUIRE(rtrim_copy(test) == "");
    }
    SECTION("trimming string with whitespaces on the right side") {
        std::string test1 = "test   ";
        REQUIRE(rtrim_copy(test1) == "test");
    }
    SECTION("trimming string without whitespaces on the right side") {
        std::string test2 = "test";
        REQUIRE(rtrim_copy(test2) == "test");
    }
    SECTION("trimming string containing only whitespaces leads to empty string") {
        std::string test3 = "    ";
        REQUIRE(rtrim_copy(test3).empty());
    }
}

TEST_CASE("trim", "[utils]") {
    SECTION("empty string") {
        std::string test = "";
        REQUIRE(trim_copy(test) == "");
    }
    SECTION("trimming string with whitespaces on the left and on the right side") {
        std::string test1 = "   test   ";
        REQUIRE(trim_copy(test1) == "test");
    }
    SECTION("trimming string without whitespaces on the left and on the right side") {
        std::string test2 = "test";
        REQUIRE(trim_copy(test2) == "test");
    }
    SECTION("trimming string containing only whitespaces leads to empty string") {
        std::string test3 = "    ";
        REQUIRE(trim_copy(test3).empty());
    }
}

#ifdef WIN32

static int clock_gettime_realtime(timespec* tv)
{
    constexpr auto MS_PER_SEC = 1000ULL;     // MS = milliseconds
    constexpr auto US_PER_MS = 1000ULL;     // US = microseconds
    constexpr auto HNS_PER_US = 10ULL;       // HNS = hundred-nanoseconds (e.g., 1 hns = 100 ns)
    constexpr auto  NS_PER_US = 1000ULL;
    constexpr auto  HNS_PER_SEC = (MS_PER_SEC * US_PER_MS * HNS_PER_US);
    constexpr auto  NS_PER_HNS = (100ULL);    // NS = nanoseconds
    constexpr auto  NS_PER_SEC = (MS_PER_SEC * US_PER_MS * NS_PER_US);

    FILETIME ft;
    GetSystemTimePreciseAsFileTime(&ft);

    ULARGE_INTEGER hnsTime;
    hnsTime.LowPart = ft.dwLowDateTime;
    hnsTime.HighPart = ft.dwHighDateTime;

    // To get POSIX Epoch as baseline, subtract the number of hns intervals from Jan 1, 1601 to Jan 1, 1970.
    hnsTime.QuadPart -= (11644473600ULL * HNS_PER_SEC);

    // modulus by hns intervals per second first, then convert to ns, as not to lose resolution
    tv->tv_nsec = (long)((hnsTime.QuadPart % HNS_PER_SEC) * NS_PER_HNS);
    tv->tv_sec = (long)(hnsTime.QuadPart / HNS_PER_SEC);

    return 0;
}

#endif // WIN32


TEST_CASE("time", "[utils]") {
    timespec ts;
#ifdef WIN32
    clock_gettime_realtime(&ts);
#else
    clock_gettime(CLOCK_REALTIME, &ts);
#endif // WIN32
    
    auto ms = ts.tv_sec * 1000 + lround(ts.tv_nsec / 1e6);
    REQUIRE(current_unix_time() - ms <= 1);
}

namespace {
bool operator==(const grpc::Status& lhs, const grpc::Status& rhs) { return lhs.error_code() == rhs.error_code() && lhs.error_message() == rhs.error_message(); }
} // anonymous namespace

TEST_CASE("fromUtilStatus", "[utils]") {
    using InternalStatusCode = google::protobuf::util::status_internal::StatusCode;
    using InternalStatus = google::protobuf::util::status_internal::Status;
    using GrpcStatus = grpc::Status;

    REQUIRE((fromUtilStatus(InternalStatus(InternalStatusCode::kOk, std::string())) == grpc::Status(grpc::OK, "")));
    REQUIRE((fromUtilStatus(InternalStatus(InternalStatusCode::kAborted, std::string("test"))) == grpc::Status(grpc::ABORTED, "test")));
    REQUIRE((fromUtilStatus(InternalStatus(InternalStatusCode::kCancelled, std::string("test"))) == grpc::Status(grpc::CANCELLED, "test")));
    REQUIRE((fromUtilStatus(InternalStatus(InternalStatusCode::kUnknown, std::string("test"))) == grpc::Status(grpc::UNKNOWN, "test")));
    REQUIRE((fromUtilStatus(InternalStatus(InternalStatusCode::kInvalidArgument, std::string("test"))) == grpc::Status(grpc::INVALID_ARGUMENT, "test")));
    REQUIRE((fromUtilStatus(InternalStatus(InternalStatusCode::kDeadlineExceeded, std::string("test"))) == grpc::Status(grpc::DEADLINE_EXCEEDED, "test")));
    REQUIRE((fromUtilStatus(InternalStatus(InternalStatusCode::kNotFound, std::string("test"))) == grpc::Status(grpc::NOT_FOUND, "test")));
    REQUIRE((fromUtilStatus(InternalStatus(InternalStatusCode::kAlreadyExists, std::string("test"))) == grpc::Status(grpc::ALREADY_EXISTS, "test")));
    REQUIRE((fromUtilStatus(InternalStatus(InternalStatusCode::kPermissionDenied, std::string("test"))) == grpc::Status(grpc::PERMISSION_DENIED, "test")));
    REQUIRE((fromUtilStatus(InternalStatus(InternalStatusCode::kUnauthenticated, std::string("test"))) == grpc::Status(grpc::UNAUTHENTICATED, "test")));
    REQUIRE((fromUtilStatus(InternalStatus(InternalStatusCode::kResourceExhausted, std::string("test"))) == grpc::Status(grpc::RESOURCE_EXHAUSTED, "test")));
    REQUIRE((fromUtilStatus(InternalStatus(InternalStatusCode::kFailedPrecondition, std::string("test"))) == grpc::Status(grpc::FAILED_PRECONDITION, "test")));
    REQUIRE((fromUtilStatus(InternalStatus(InternalStatusCode::kAborted, std::string("test"))) == grpc::Status(grpc::ABORTED, "test")));
    REQUIRE((fromUtilStatus(InternalStatus(InternalStatusCode::kOutOfRange, std::string("test"))) == grpc::Status(grpc::OUT_OF_RANGE, "test")));
    REQUIRE((fromUtilStatus(InternalStatus(InternalStatusCode::kUnimplemented, std::string("test"))) == grpc::Status(grpc::UNIMPLEMENTED, "test")));
    REQUIRE((fromUtilStatus(InternalStatus(InternalStatusCode::kInternal, std::string("test"))) == grpc::Status(grpc::INTERNAL, "test")));
    REQUIRE((fromUtilStatus(InternalStatus(InternalStatusCode::kUnavailable, std::string("test"))) == grpc::Status(grpc::UNAVAILABLE, "test")));
    REQUIRE((fromUtilStatus(InternalStatus(InternalStatusCode::kDataLoss, std::string("test"))) == grpc::Status(grpc::DATA_LOSS, "test")));
}

class ServerContextFake : public grpc::ServerContextBase {
public:
    ServerContextFake(gpr_timespec deadline, grpc_metadata_array* arr)
        : ServerContextBase(deadline, arr) {}
    using ServerContextBase::client_metadata;
};

TEST_CASE("getDatasetName", "[utils]") {
    grpc_metadata_array metadata;
    grpc_metadata_array_init(&metadata);
    metadata.count = 1;
    metadata.metadata = new grpc_metadata[1]{ grpc::SliceFromCopiedString("gms_dataset"), grpc::SliceFromCopiedString("fixed_price_1234") };
    
    //pc_timespec
    ServerContextFake context(gpr_timespec(0, 0), &metadata);
    context.client_metadata();
    auto test = getDatasetName(&context);
    REQUIRE(getDatasetName(&context) == "fixed_price_1234.");

    delete[] metadata.metadata;
    grpc_metadata_array_destroy(&metadata);
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

TEST_CASE("readFile", "[fs_utils]") {
#ifdef WIN32
    auto result = grpc_mock_server::readFile("C:\\Windows\\win.ini");
    REQUIRE(result.substr(0, 24) == "; for 16-bit app support");
#else
#error "Not implemented yet!"
#endif
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

TEST_CASE("Config", "[config]") {
    SECTION("haveRemoteHost") {
        auto rc_fs = cmrc::grpc_mock_server::get_filesystem();
        auto request_file = rc_fs.open("assets/config.xml");
        auto request_data = std::string(request_file.cbegin(), request_file.cend());

        auto &config = Config::instance();
        REQUIRE(config.parse(request_data));

        // Remote gRPC server data
        REQUIRE(config.haveRemoteHostUrl());
        REQUIRE(config.haveRemoteHostPort());
        REQUIRE(config.haveLocalHostPort());
        REQUIRE(config.remoteHostUrl() == "remote_host");
        REQUIRE(config.remoteHostPort() == 123);
        REQUIRE(config.localHostPort() == 456);

        // Mock data
        REQUIRE(config.haveFullPath("fixed_price_1234.orderPackage.orderService/ListOrders"));
        REQUIRE(config.havePartialPath("fixed_price_1234.orderPackage.orderService/ListOrders"));
        REQUIRE(config.fullPath("fixed_price_1234.orderPackage.orderService/ListOrders") == "list_orders_response.txt");
        REQUIRE(config.partialPath("fixed_price_1234.orderPackage.orderService/ListOrders") == "list_orders_request.txt");
    }
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

TEST_CASE("message_as_json", "[utils]") {
    routeguide::RouteSummary message;
    message.set_point_count(5);
    message.set_feature_count(4);
    message.set_distance(3);
    message.set_elapsed_time(1);
    std::string message_json = message_as_json(message);
    REQUIRE(message_json == R"({"pointCount":5,"featureCount":4,"distance":3,"elapsedTime":1})");
}

TEST_CASE("evalRequest", "[utils]") {
    routeguide::RouteSummary message;
    message.set_point_count(5);
    message.set_feature_count(4);
    message.set_distance(3);
    message.set_elapsed_time(1);

    auto rc_fs = cmrc::grpc_mock_server::get_filesystem();
    auto request_file = rc_fs.open("assets/point_count_request.txt");
    auto request_data = std::string(request_file.cbegin(), request_file.cend());

    evalRequest(message, request_data);

    REQUIRE(message.point_count() == 12345);
}
