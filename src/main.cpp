#include <iostream>
#include <string>
#include <winsock2.h>
#include <stdexcept>
#include <cstdio>
#include <sstream>
#include <thread>

using namespace std;

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

class Socket {
public:
    explicit Socket(SOCKET s) : sock_(s) {}          // take ownership of a raw handle
    ~Socket() {
    if (sock_ != INVALID_SOCKET)   // don't close an emptied/moved-from socket
        closesocket(sock_);
    }
    Socket& operator=(const Socket&) = delete;
    Socket(const Socket&)            = delete;        // no copy
    Socket(Socket&& other) noexcept : sock_(other.sock_) {  // steal other's handle
        other.sock_ = INVALID_SOCKET;                        // empty the source!
    }
    Socket& operator=(Socket&& other) noexcept {
        if (this != &other) {                    // guard against self-move
            if (sock_ != INVALID_SOCKET)         // close MY current socket first (no leak)
                closesocket(sock_);
            sock_ = other.sock_;                 // steal
            other.sock_ = INVALID_SOCKET;        // empty source
        }
        return *this;
    }

    SOCKET get() const { return sock_; }              // access raw handle for recv/send/etc.

private:
    SOCKET sock_ = INVALID_SOCKET;
};

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

string buildResponse(int statusCode, const string& reason, const string& contentType, const string& body) {
    ostringstream oss;
    oss << "HTTP/1.1 " <<  to_string(statusCode) << " " << reason << "\r\n" <<  "Content-Type: " << contentType << "\r\n" << "Content-Length: " << to_string(body.size()) << "\r\n\r\n" << body;
    return oss.str();
}

void handleClient(Socket clientSocket) {
    char buffer[4096];
    while (true) {
        int bytes = recv(clientSocket.get(), buffer, sizeof(buffer), 0);
        if (bytes > 0) {
            string request(buffer, bytes);
            Request httpRequest = parseRequestLine(request);
            printf("%s %s %s\n", httpRequest.method.c_str(), httpRequest.path.c_str(), httpRequest.version.c_str());
            string response = buildResponse(200, "OK", "text/html",
                "<html><body><h1>Hello from my C++ server!</h1></body></html>");
            int total = 0;
            int len = (int) response.size();
            while (total < len) {
                // 8. send() -> echo the bytes back
                int sent = send(clientSocket.get(), response.c_str() + total, response.size() - total, 0);
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
}

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
    while (true) {
        sockaddr_in clientAddr{};
        int addrLen = sizeof(clientAddr);
        SOCKET rawClientSocket = accept(serverSocket.get(), (sockaddr*)&clientAddr, &addrLen);
        if (rawClientSocket == INVALID_SOCKET) {
            die("accept");
            continue;
        }
        Socket clientSocket(rawClientSocket);
        std::thread t(handleClient, std::move(clientSocket));
        t.detach();
    }
    return 0;

}