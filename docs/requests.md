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

```Request::get``` will return either a request that has been passed to ```Request::feedBackRequest``` or a newly accepted request.

```Request::feedBackRequest``` allows you to place an existing request inside the request queue, which is useful for handling multiple requests on the same connection.

```Request``` has five methods: ```readRequest()``` which returns a ```std::string``` containing the contents of the request, and ```respond(const std::string& message)``` which responds to the client conenction with the given message.

```readRequest(size_t charAmount)``` reads ```charAmount``` bytes from the socket stream.

```readUntil(const std::string& delimeter)``` reads from the socket stream until the buffer contains the value of ```delimeter```.

```unread(const std::string& requestRemainder)``` allows you to 'stuff' a string into request stream so next time you read from it, those bytes will be read first. This is useful for when you read too far and reached a new request, so you can throw that stuff back and then call ```Request::feedBackRequest``` with that request for later use.

```Request``` provides a low-level interface for network programming that allows you to easily implement any binary or text protocol without worrying about networking boilerplate.