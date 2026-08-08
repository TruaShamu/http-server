#include "http.hpp"
#include <sstream>

using namespace std;

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