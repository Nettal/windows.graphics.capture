//
// Created by SnowNF on 2024/4/29.
//

#include <cerrno>
#include <cstring>
#include <cstdint>
#include "network.h"
#include "log_helper.h"

static bool isInitialized = false;

void static preInit() {
#ifdef WIN32
    if (isInitialized)
        return;
    isInitialized = true;
    WSADATA wsaData;
    if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        mw_fatal("failed to load Winsock");
    }
#endif
}

TheServer::TheServer(int port) {
    preInit();
    sock = (int) ::socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    int r = ::bind(sock, (const struct sockaddr *) &addr, sizeof(addr));
    if (r != 0) {
        mw_fatal("unable bind to %d\n", port);
    }
    if (::listen(sock, 5) != 0) {
        mw_fatal("listen: %s", strerror(errno));
    }
}

int TheServer::sAccept() const {
    struct sockaddr_in remoteAddr{};
    int nAddrlen = sizeof(remoteAddr);
    int r;
#ifdef __linux
    r = ::accept(sock, (struct sockaddr *) &remoteAddr, (socklen_t *) &nAddrlen);
#else
    r = (int) ::accept(sock, (struct sockaddr *) &remoteAddr, &nAddrlen);
#endif
    if (r == -1) {
        mw_fatal("unable to accept");
    }
    return r;
}

bool TheServer::send(const void *basePtr, int64_t len) const {
    assert(len != 0);
    int64_t result;
#ifdef __linux
    result = ::send(sock, basePtr, len, 0);
#elif defined(WIN32)
    result = ::send(sock, (const char *) basePtr, (int) len, 0);
#else
#error ""
#endif
    assert(result == len);
    return true;
}

TheClient::TheClient(const char *address, int port) {
    preInit();
    sock = (int) ::socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(address);
}

int TheClient::cConnect() {
    int r = ::connect(sock, (const sockaddr *) (&addr), sizeof(addr));
    if (r != 0) {
        mw_fatal("unable to connect");
    }
    return sock;
}

int TheClient::cConnectRaw(bool *success) {
    int r = ::connect(sock, (const sockaddr *) (&addr), sizeof(addr));
    if (r != 0) {
        *success = false;
        return -1;
    }
    *success = true;
    return sock;
}

int64_t mw_read(int handle, void *dest, int64_t len) {
    assert(len != 0);
    return ::recv(handle, (char *) dest, (int) len, 0);
}


void mw_read_all(int handle, void *basePtr_, int64_t len) {
    auto *basePtr = (int8_t *) basePtr_;
    assert(len != 0);
    int64_t result = 1;
    int64_t baseSize = len;
    int64_t sum = 0;
    while (result > 0) {
        result = ::mw_read(handle, basePtr, baseSize);
        sum += result;
        basePtr += result;
        baseSize -= result;
        if (result == -1) {
            mw_fatal("Error while receiving , received %lld expect %zu", sum, len);
        }
        if (sum == len)
            return;
    }
    assert(0);
}