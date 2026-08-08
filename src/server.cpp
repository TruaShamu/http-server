#include "server.hpp"
#include "http.hpp"

#include <cstdio>
#include <winsock2.h>

using namespace std;

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