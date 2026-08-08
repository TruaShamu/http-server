#include "http.hpp"
#include <sstream>
#include <fstream>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

Request parseRequestLine(const string& raw) { 
    istringstream iss(raw); 
    Request req;
    iss >> req.method >> req.path >> req.version; 
    return req;
}

std::string Response::serialize() const {
    ostringstream oss;
    oss << "HTTP/1.1 " <<  to_string(statusCode) << " " << reason << "\r\n" <<  "Content-Type: " << contentType << "\r\n" << "Content-Length: " << to_string(body.size()) << "\r\n\r\n" << body;
    return oss.str();
}

Response routeRequest(const Request& req) {
    if (req.method != "GET") {
        return Response{405, "Method Not Allowed", "text/plain", "405 Method Not Allowed"};
    }
    return serveStaticFile(req.path);
}

Response serveStaticFile(const std::string& urlPath) {
    std::string rel = urlPath;
    if (rel == "/") rel = "/index.html";
    fs::path docroot = fs::weakly_canonical("public");     // the sandbox, absolute
    fs::path candidate = fs::weakly_canonical(docroot / rel.substr(1));
    auto rootStr = docroot.string();
    auto candStr = candidate.string();
    if (candStr.rfind(rootStr, 0) != 0) {          // rfind(x,0)==0 means "starts with x"
        return Response{403, "Forbidden", "text/plain", "403 Forbidden"};
    }
    if (!fs::exists(candidate) || !fs::is_regular_file(candidate)) {
        return Response{404, "Not Found", "text/plain", "404 Not Found"};
    }
    std::ifstream file(candidate, std::ios::binary);
    std::ostringstream ss;
    ss << file.rdbuf();               // slurp entire file
    std::string body = ss.str();
    return Response{200, "OK", "text/html", body};

}