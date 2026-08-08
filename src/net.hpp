#pragma once

#include <winsock2.h>
#include <stdexcept>

using namespace std;

class Socket {
public:
    explicit Socket(SOCKET s);
    ~Socket();
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    Socket(Socket&& other) noexcept;
    Socket& operator=(Socket&& other) noexcept;
    SOCKET get() const;
private:
    SOCKET sock_ = INVALID_SOCKET;
};

class WsaGuard {
public:
    WsaGuard()  {
        WSAData wsaData;
        WORD wsVersion = MAKEWORD(2, 2);

        int status = WSAStartup(wsVersion, &wsaData);
        if (status != 0) {
            throw runtime_error("Winsocket couldn't start up.");
        }
    }
    ~WsaGuard() {
        WSACleanup();
    }

    WsaGuard(const WsaGuard&)            = delete;   // no copy construction
    WsaGuard& operator=(const WsaGuard&) = delete;   // no copy assignment
};

void die(const char* what);