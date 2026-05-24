# Using Callbacks To Treat Networking Failures
There are numerous ways in which the operating system might fail when working with network sockets. This library wants to give developers the freedom to treat these errors however they want, so developers can register callback functions for each potential error.
## Failure points
When ```initServer()``` is called, there are 4 potential failure points for the operating system:
* ```socket()``` can return an invalid socket fd
* ```setsockopt()``` can fail
* ```bind()``` can fail
* ```listen()``` can fail

By default, any of these failures will cause the library to print an error message and close the program with the ```EXIT_FAILURE``` status code.
If a callback function is registered, the program will call that function instead, clean up whatever progress has been made, and return.
## Registering a callback function
Each of the failure points requires a callback function of type ```std::function<void()>```. These functions can be registered with the following function calls:
* ```setFailedSocketCallback(FailedSocketHandler socketHandler)```
* ```setFailedSetSockOptCallback(FailedSetSockOptHandler setSockOptHandler)```
* ```setFailedBindCallback(FailedBindHandler bindHandler)```
* ```setFailedListenCallback(FailedListenHandler listenHandler)```

Example usage:
```cpp
bool initServerSucceeded = true;
setFailedBindCallback([&initServerSucceeded]() {
    initServerSucceeded = false;
});
initServer();
if (!initServerSucceeded) {
    setPort(5050);
    initServer();
}
```
This code checks if ```initServer()``` successfully managed to bind a socket to a port, and if it hasn't, sets a new port and attempts to bind it again.