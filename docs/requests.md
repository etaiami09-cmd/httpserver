# Using The Networking Infrastructure To Manage Network Requests

This library provides an underlying struct to handle network requests: ```Request```.
A ```Request``` object is initialized using the ```Request::get``` method while the server is active:
```cpp
#include "Request.hpp"

int main() {
    initServer();

    Request request = Request::get();
    std::cout << request.readRequest();
    request.respond("Hello Network Request!");

    closeServer();
}
```

```Request``` allows for low-level control of the network IO without using the HTTP protocol.  When a ```Request``` object goes out of scope, the connection with the client is automatically closed using RAII principles.

```Request``` has four methods: ```readRequest()``` which returns a ```std::string``` containing the contents of the request, and ```respond(const std::string& message)``` which responds to the client conenction with the given message.

```readRequest(size_t charAmount)``` reads ```charAmount``` bytes from the socket stream.

```readUntil(const std::string& delimeter)``` reads from the socket stream until the buffer contains the value of ```delimeter```. Note: the method only guarantees it will continue reading until the delimeter is reached, not that it will only read up to the delimeter. There may be bytes after the delimeter in the returned ```std::string```.

```Request``` provides a low-level interface for network programming that allows you to easily implement any binary or text protocol without worrying about networking boilerplate.