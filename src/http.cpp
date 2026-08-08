#include "http.hpp"
#include <sstream>
#include <fstream>
#include <filesystem>
#include <unordered_map>
#include <cctype>
#include <algorithm>

using namespace std;
namespace fs = std::filesystem;
std::string mimeType(const fs::path& p);

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
    std::error_code ec;
    fs::path relPath = fs::relative(candidate, docroot, ec);
    if (ec || relPath.empty() || relPath.begin()->string() == "..") {
        return Response{403, "Forbidden", "text/plain", "403 Forbidden"};
    }
    if (!fs::exists(candidate) || !fs::is_regular_file(candidate)) {
        return Response{404, "Not Found", "text/plain", "404 Not Found"};
    }
    std::ifstream file(candidate, std::ios::binary);
    std::ostringstream ss;
    ss << file.rdbuf();               // slurp entire file
    std::string body = ss.str();
    return Response{200, "OK", mimeType(candidate), body};
}

std::string mimeType(const fs::path& p) {
    std::string ext = p.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    static const std::unordered_map<std::string, std::string> types = {
        {".html", "text/html"}, {".htm", "text/html"},
        {".css",  "text/css"},  {".js",  "application/javascript"},
        {".png",  "image/png"}, {".jpg", "image/jpeg"}, {".jpeg", "image/jpeg"},
        {".gif",  "image/gif"}, {".svg", "image/svg+xml"}, {".txt", "text/plain"},
    };

    auto it = types.find(ext);
    return it != types.end() ? it->second : "application/octet-stream";
}