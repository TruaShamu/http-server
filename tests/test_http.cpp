#include <catch2/catch_test_macros.hpp>
#include <fstream>
#include <string>
#include "../src/http.hpp"

using namespace std;

struct TempDir {
    fs::path path;
    TempDir() {
        static int counter = 0;
        path = fs::temp_directory_path()
             / ("httptest_" + std::to_string(counter++));
        fs::create_directories(path);
    }
    ~TempDir() { std::error_code ec; fs::remove_all(path, ec); }
};

TEST_CASE("serveStaticFile serves an existing file") {
    TempDir dir;
    std::ofstream(dir.path / "index.html") << "<h1>hi</h1>";
    Response resp = serveStaticFile("/", dir.path);
    REQUIRE(resp.statusCode == 200);
    REQUIRE(resp.body.find("hi") != std::string::npos);
}

TEST_CASE("Response::serialize produces a valid status line") {
    Response resp{404, "Not Found", "text/plain", "oops"};
    std::string raw = resp.serialize();
    REQUIRE(raw.rfind("HTTP/1.1 404 Not Found\r\n", 0) == 0);   // starts with status line
    REQUIRE(raw.find("Content-Length: 4\r\n") != std::string::npos);
    REQUIRE(raw.find("\r\n\r\noops") != std::string::npos);     // blank line then body
}

TEST_CASE("routeRequest returns 405 for non-GET methods") {
    Request req{"POST", "/", "HTTP/1.1"};
    Response resp = routeRequest(req);
    REQUIRE(resp.statusCode == 405);
}

// TODO(next session): re-add parseRequestLine test + serveStaticFile 404 and 403 cases.