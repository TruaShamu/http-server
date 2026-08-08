#pragma once
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

struct Request {
    std::string method;
    std::string path;
    std::string version;
};

struct Response {
    int statusCode = 200;
    std::string reason = "OK";
    std::string contentType = "text/plain";
    std::string body;
    std::string serialize() const;
};

Request parseRequestLine(const std::string& raw);

Response routeRequest(const Request& req);

Response serveStaticFile(const std::string& urlPath, const fs::path& docroot = "public");