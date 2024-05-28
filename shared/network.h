//
// Created by SnowNF on 2024/4/29.
//

#ifndef BRIDGE_NETWORK_H
#define BRIDGE_NETWORK_H

#ifdef WIN32

#include <winsock.h>

#endif

class TheServer {
public:
    explicit TheServer(int port);

    int sAccept() const;

private:
    int sock;
    struct sockaddr_in addr{};
};

class TheClient {
public:
    TheClient(const char *address, int port);

    int cConnect();

    int cConnectRaw(bool *success);

private:
    int sock;
    struct sockaddr_in addr{};
};

void mw_read_all(int handle, void *basePtr_, int64_t len);

bool mw_send(int64_t handle, const void *basePtr, int64_t len);

#endif //BRIDGE_NETWORK_H
