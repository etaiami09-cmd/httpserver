# Registering Your First HTTP Endpoint
This page will walk you through the endpoint API and how to properly route HTTP requests to a certain endpoint.
## What is a registered endpoint?
A registered endpoint is an association of an HTTP method and a URL path to a function that takes a ```RequestObject``` of the corresponding method and returns an ```HTTPResponse``` object. The different ```RequestObject```s and their APIs are documented in ```methods.md```. A registered endpoint will route any requests given to ```handleRequest``` with its associated method and URL path to the function registered with it and will respond to the network connection with a formatted HTTP response based on the ```HTTPResponse``` object returned by the function.
## Registering an endpoint
Endpoints can be registered using the ```registerEndpoint``` function:
```cpp
registerEndpoint<HTTPMethod::Get>("/", [](GetRequest request) {
    return HTTPResponse(HTTPStatusCode::Ok)
        .addContentType(HTTPContentType::TextPlain)
        .addBody("This is a response")
        .closeConnection();
});
```
The first argument passed is the path, and the second argument passed is the handler function. A handler function must be of type ```RequestHandler<method>``` where ```method``` is the associated HTTP method being registered (in this example, ```HTTPMethod::Get```).
Information about creating an ```HTTPResponse``` object is documented in ```responses.md```.