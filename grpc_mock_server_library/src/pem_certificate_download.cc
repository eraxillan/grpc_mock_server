#include "pem_certificate_download.h"

#include <grpc_mock_server_logger.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#endif

#include <openssl/ssl.h>
#include <openssl/err.h>

#ifdef WIN32
#define SocketType SOCKET
#else
#define SocketType int
#define INVALID_SOCKET  (SOCKET)(~0)
#endif

namespace {

struct SocketWrapper {
    SocketType data;

    SocketWrapper(SocketType data) { this->data = data; }
};
void socketClose(SocketType socket_fd);

using BioHandle = std::unique_ptr < BIO, decltype([](BIO* p) {
    // 1
    SystemLogger->info("BIO_free");
    BIO_free(p);
}) > ;
using X509Handle = std::unique_ptr < X509, decltype([](X509* p) {
    // 2
    SystemLogger->info("X509_free");
    X509_free(p);
}) > ;
using SslHandle = std::unique_ptr < SSL, decltype([](SSL* p) {
    // 3
    SystemLogger->info("SSL_free");
    SSL_free(p);
}) > ;
using SslCtxHandle = std::unique_ptr < SSL_CTX, decltype([](SSL_CTX* p) { 
    // 4
    SystemLogger->info("SSL_CTX_free");
    SSL_CTX_free(p);
}) >;
using SocketHandle = std::unique_ptr < SocketWrapper, decltype([](SocketWrapper* p) {
    // 5
    SystemLogger->info("socketClose");
    socketClose(p->data);
}) >;

// TODO: class WinsockWrapper
int winsockInit() {
#ifdef WIN32
    WORD version = MAKEWORD(2, 0);
    WSADATA wsaData{};
    int error = WSAStartup(version, &wsaData);
    if (error != 0) {
        SystemLogger->error("WSAStartup() failed with code {}", error);
        return -1;
    }

    // Check for correct version
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 0) {
        SystemLogger->error("Invalid Winsock version {}", wsaData.wVersion);
        WSACleanup();
        return -1;
    }
#endif // WIN32

    return 0;
}

void winsockFree() {
#ifdef WIN32
    WSACleanup();
#endif // WIN32
}

void socketClose(SocketType socket_fd) {
#ifdef WIN32
    closesocket(socket_fd);
#else
    close(socket_fd);
#endif
}

} // namespace anonymous

static SocketType tcpConnect(const char* host, const char* port) {
    int status = -1;

    addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    
    addrinfo* res = nullptr;
    if ((status = getaddrinfo(host, port, &hints, &res)) != 0) {
        SystemLogger->error("getaddrinfo() error: {}", gai_strerror(status));
        return -1;
    }

    SocketType socket_fd = socket(res->ai_family, res->ai_socktype, (int)res->ai_protocol);
    if (socket_fd == INVALID_SOCKET) {
        SystemLogger->error("socket() error: {}", gai_strerror((int)socket_fd));
        return -1;
    }

#ifdef __linux__
    // Set the socket connect timeout to 2 seconds
    uint32_t user_timeout = 2000;
    socklen_t user_timeout_len = sizeof(user_timeout);
    setsockopt(socket_fd, IPPROTO_TCP, TCP_USER_TIMEOUT, &user_timeout, user_timeout_len);
#endif

    status = connect(socket_fd, res->ai_addr, static_cast<int>(res->ai_addrlen));
    if (status != 0) {
        SystemLogger->error("connect() error: {}", gai_strerror(status));
        return -1;
    }

    freeaddrinfo(res);

    return socket_fd;
}

namespace grpc_mock_server {

int downloadPemCertificate(const char* remote_host, const char* remote_port, const char* local_file_path) {
    winsockInit();

    SocketHandle socket;
    SslCtxHandle ctx;
    SslHandle ssl;
    X509Handle peer;

    OpenSSL_add_ssl_algorithms();
    ERR_load_crypto_strings();
    SSL_load_error_strings();

    const SSL_METHOD* method = SSLv23_client_method();
    ctx.reset(SSL_CTX_new(method));

    // Connect the TCP socket
    auto socket_fd = tcpConnect(remote_host, remote_port);
    if (socket_fd < 0) {
        SystemLogger->error("Unable to connect to host:port '{}:{}'", remote_host, remote_port);
        return -1;
    }
    socket.reset(new SocketWrapper(socket_fd));

    // Connect the SSL socket
    ssl.reset(SSL_new(ctx.get()));

    // SSL_free also frees the assigned BIO, so not need in RAII here
    BIO* sbio = BIO_new_socket((int)socket->data, BIO_NOCLOSE);
    SSL_set_bio(ssl.get(), sbio, sbio);

    int status = SSL_connect(ssl.get());
    if (status <= 0) {
        int err = SSL_get_error(ssl.get(), status);
        if (err == SSL_ERROR_SSL) {
            char message[1024];
            ERR_error_string_n(ERR_get_error(), message, sizeof(message));
            SystemLogger->error(
                "SSL_connect() errors: | {} | {} | {}",
                message,
                ERR_lib_error_string(0),
                ERR_reason_error_string(0)
            );
        }
        return -1;
    }

    peer.reset(SSL_get_peer_certificate(ssl.get()));

    {
        BioHandle local_file(BIO_new_file(local_file_path, "w"));
        if (!local_file) {
            SystemLogger->error("BIO_new_file() error: {}", status);
            return -1;
        }
        PEM_write_bio_X509(local_file.get(), peer.get());
    }

    winsockFree();

    ERR_free_strings();

    return 0;
}

} // grpc_mock_server
