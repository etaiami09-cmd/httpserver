# Handling HTTP Methods
HTTP Methods are enforced at compile-time, meaning a ```registerEndpoint``` call must pass a function that handles the specific HTTP method being intercepted.

The HTTP methods supported by this library are as follows:
* GET: ```HTTPMethod::Get```
* POST: ```HTTPMethod::Post```
* PUT: ```HTTPMethod::Put```
* PATCH: ```HTTPMethod::Patch```
* DELETE: ```HTTPMethod::Delete```

Each of these has its own associated ```RequestObject``` type:
```GetRequest```, ```PostRequest```, ```PutRequest```, ```PatchRequest```, and ```DeleteRequest```.
They can be accessed using their names or more explicit template syntax. Example:
```RequestObject<HTTPMethod::Get>``` is the defined as ```GetRequest```.

Each ```RequestObject``` has some shared fields:
* ```params``` is an optional hash map containing any URL parameters
* ```path``` is a string containing the URL path of the request (i.e. ```"/home"```)
* ```host``` contains the host field of the request
* ```cookies``` is an optional hash map containing any cookies passed with the request

Additionally, a POST, PUT, or PATCH ```RequestObject``` contains the following fields:
* ```contentType``` is an optional ```HTTPContentType``` value representing the *Content-Type* field in an HTTP request
* ```contentLength``` is an optional ```size_t``` representing the *Content-Length* field in an HTTP request
* ```body``` is an optional string containing the contents of the request