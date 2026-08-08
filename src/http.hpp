#pragma once
#include <string>

struct Request {
    std::string method;
    std::string path;
    std::string version;
};

Request parseRequestLine(const std::string& raw);

std::string buildResponse(int statusCode, const std::string& reason, const std::string& contentType, const std::string& body);