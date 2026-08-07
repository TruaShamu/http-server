#include <iostream>
#include <string>
#include <winsock2.h>
#include <stdexcept>
#include <cstdio>
#include <sstream>

using namespace std;

void die(const char* what) {
    printf("%s failed. Winsock error: %d\n", what, WSAGetLastError());
}

struct Request {
    string method;
    string path;
    string version;
};

Request parseRequestLine(const string& raw) { 
    istringstream iss(raw); 
    Request req;
    iss >> req.method >> req.path >> req.version; 
    return req;
}

int main() {
    // 1. WSAStartup  -> load Winsock, check return
    WSAData wsaData;
    WORD wsVersion = MAKEWORD(2, 2);

    int status = WSAStartup(wsVersion, &wsaData);
    if (status != 0) {
        throw runtime_error("Winsocket couldn't start up.");
    }

    // 2. socket()    -> AF_INET, SOCK_STREAM, 0 ; check for INVALID_SOCKET. IPPROTO_TCP for TCP protocol.
    // AF_INET is for IPv4. SOCK_STREAM is for bidirectional communication. 
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        die("socket");
        throw runtime_error("Server socket couldn't initialize.");
    }

    // 3. sockaddr_in -> fill sin_family, sin_port (htons!), sin_addr
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(8080);               // htons is "host to network short". Host is little-endian. Network byte order is big-endian.
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // 4. bind()      -> check for SOCKET_ERROR
    int bindResult = bind(serverSocket, (SOCKADDR*)&addr, sizeof(addr));
    if (bindResult == SOCKET_ERROR) {
        die("bind");
        throw runtime_error("Bind failed");
    }

    // 5. listen()    -> check for SOCKET_ERROR
    int listenResult = listen(serverSocket, SOMAXCONN); // Listen and allow as many connections queued as the OS can support.
    if (listenResult == SOCKET_ERROR) {
        die("listen");
        throw runtime_error("Listen failed");
    }
    printf("Listening on http://localhost:8080 ...\n");

    // 6. accept()    -> BLOCKS; returns the client socket; check INVALID_SOCKET
    while (true) {
        sockaddr_in clientAddr{};
        int addrLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &addrLen);
        if (clientSocket == INVALID_SOCKET) {
            die("accept");
            throw runtime_error("Accept failed");
        }
        // 7. recv() loop -> read bytes; remember Q2 (>0 / ==0 / <0)
        char buffer[4096];
        while (true) {
            int bytes = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytes > 0) {
                string request(buffer, bytes);
                Request httpRequest = parseRequestLine(request);
                printf("%s %s %s\n", httpRequest.method.c_str(), httpRequest.path.c_str(), httpRequest.version.c_str());
                int total = 0;
                while (total < bytes) {
                    // 8. send()      -> echo the bytes back
                    int sent = send(clientSocket, buffer + total, bytes - total, 0);
                    if (sent == SOCKET_ERROR) {
                        die("send");
                        break;
                    }
                    total += sent;
                }
            }
            else if (bytes == 0) {
                break;
            }
            else {
                die("recv");
                break;
            }
        }
        // 9. closesocket(client), closesocket(listener)
        closesocket(clientSocket);
    }
    closesocket(serverSocket);
    WSACleanup();
    return 0;

}