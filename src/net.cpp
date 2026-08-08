#include "net.hpp"
#include <cstdio>

Socket::Socket(SOCKET s) : sock_(s) {}        // note the Socket:: prefix
Socket::~Socket() { if (sock_ != INVALID_SOCKET) closesocket(sock_); }
SOCKET Socket::get() const { return sock_; }
void die(const char* what) {
    printf("%s failed. Winsock error: %d\n", what, WSAGetLastError());
}
Socket::Socket(Socket&& other) noexcept : sock_(other.sock_) {
    other.sock_ = INVALID_SOCKET;
}
Socket& Socket::operator=(Socket&& other) noexcept {
    if (this != &other) {
        if (sock_ != INVALID_SOCKET) closesocket(sock_);
        sock_ = other.sock_;
        other.sock_ = INVALID_SOCKET;
    }
    return *this;
}