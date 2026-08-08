#include <iostream>
#include <string>
#include <winsock2.h>
#include <stdexcept>
#include <cstdio>
#include <sstream>
#include <thread>
#include <memory>
#include "net.hpp"
#include "server.hpp"
#include "threadpool.hpp"

using namespace std;

int main() {
    // 1. WSAStartup  -> load Winsock, check return
    WsaGuard wsa;
    // 2. socket()    -> AF_INET, SOCK_STREAM, 0 ; check for INVALID_SOCKET. IPPROTO_TCP for TCP protocol.
    // AF_INET is for IPv4. SOCK_STREAM is for bidirectional communication. 
    SOCKET raw = socket(AF_INET, SOCK_STREAM, 0);
    if (raw == INVALID_SOCKET) {
        die("socket");
        throw runtime_error("Server socket couldn't initialize.");
    }
    Socket serverSocket(raw);

    // 3. sockaddr_in -> fill sin_family, sin_port (htons!), sin_addr
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(8080);               // htons is "host to network short". Host is little-endian. Network byte order is big-endian.
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // 4. bind()      -> check for SOCKET_ERROR
    int bindResult = bind(serverSocket.get(), (SOCKADDR*)&addr, sizeof(addr));
    if (bindResult == SOCKET_ERROR) {
        die("bind");
        throw runtime_error("Bind failed");
    }

    // 5. listen()    -> check for SOCKET_ERROR
    int listenResult = listen(serverSocket.get(), SOMAXCONN); // Listen and allow as many connections queued as the OS can support.
    if (listenResult == SOCKET_ERROR) {
        die("listen");
    }
    printf("Listening on http://localhost:8080 ...\n");

    // 6. accept()    -> BLOCKS; returns the client socket; check INVALID_SOCKET
    ThreadPool pool(4);
    while (true) {
        sockaddr_in clientAddr{};
        int addrLen = sizeof(clientAddr);
        SOCKET rawClientSocket = accept(serverSocket.get(), (sockaddr*)&clientAddr, &addrLen);
        if (rawClientSocket == INVALID_SOCKET) {
            die("accept");
            continue;
        }
        Socket clientSocket(rawClientSocket);
        auto sock = std::make_shared<Socket>(std::move(clientSocket));
        pool.enqueue([sock]() {
            handleClient(std::move(*sock));
        });
    }
    return 0;

}