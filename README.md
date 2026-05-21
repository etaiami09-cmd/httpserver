# A C++23 HTTP And Networking Server Library
## Create and manage an HTTP server and network sockets with low-level control
## Features
* HTTP 1.1 compatibility
* Cookies, rendering HTML and sending JavaScript files
* Sending any kind of information you want using the HTTP 1.1 protocol
* Simple builder pattern to create responses and cookies
* Convenient syntax to create endpoints using lambdas
* Modern C++ style, library currently only works on C++23 due to its use of ```constexpr std::string```
* Heavy leveraging of templates and concepts to create readable and refactorable code
## Examples
To use either the entire HTTP library or just the networking infrastructure, use ```#include "HTTP.hpp"``` or ```#include "Request.hpp"``` respectively.
### Using the networking infrastucture
Below is a code snippet to handle a single request by printing its contents and responding:
```cpp
initServer();

auto request = Request::get();
std::cout << request.readRequest();
request.respond("Hello, World!");

closeServer();
```
### Using the HTTP infrastructure to run a server
```cpp
initServer();

registerEndpoint<HTTPMethod::Get>("/", [](GetRequest request) {
    return HTTPResponse::renderHTML("index.html")
            .addCookie(
            ResponseCookie("username", "foo")
            .addAge(CookieLength::Days(30))
    );
});

while (true) {
    auto request = Request::get();
    handleRequest(request);
}

closeServer();
```