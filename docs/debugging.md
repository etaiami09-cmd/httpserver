# Debugging The HTTP Server
## Hot Reload
Hot reloading is disabled by default, but you can enable it by defining the ```HOT_RELOAD``` macro before including ```HTTP.hpp```.
Example without hot reload:
```cpp
#ifdef HOT_RELOAD
#undef HOT_RELOAD
#endif

#include <iostream>
#include "HTTP.hpp"

int main() {
    initServer();

    registerEndpoint<HTTPMethod::Get>("/", [](GetRequest request) {
        return HTTPResponse::renderHTML("index.html");
    });

    while (true) {
        auto request = Request::get();
        handleRequest(request);
    }

    closeServer();
}
```
When ```HOT_RELOAD``` is not defined, files sent through ```HTTPResponse::renderHTML``` and ```HTTPResponse::executeJSON``` will only be loaded through file IO once on the first request and then cached for future use.
Example with hot reload:
```cpp
#ifndef HOT_RELOAD
#define HOT_RELOAD
#endif

#include <iostream>
#include "HTTP.hpp"

int main() {
    initServer();

    registerEndpoint<HTTPMethod::Get>("/", [](GetRequest request) {
        return HTTPResponse::renderHTML("index.html");
    });

    while (true) {
        auto request = Request::get();
        handleRequest(request);
    }

    closeServer();
}
```
When ```HOT_RELOAD``` is defined, files will be pulled through file IO every time a request requires them, with no caching. 