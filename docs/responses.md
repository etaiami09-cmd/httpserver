# Creating An HTTP Response Object
In order to handle an HTTP request, you must register an endpoint with a ```RequestHandler``` function. A ```RequestHandler``` is a ```std::function<HTTPResponse(RequestObject<method>)>``` where ```method``` is the ```HTTPMethod``` of the endpoint being registered.
## Builder pattern
The ```HTTPResponse``` API uses a builder pattern to make code explicit and clear without being overly verbose. Builder methods of an ```HTTPResponse``` can be chained together:
```cpp
auto cookie = ResponseCookie("password", "password123")
                    .addAge(CookieLength::Days(30));
auto response = HTTPResponse(HTTPStatusCode::Ok)
                    .addContentType(HTTPContentType::ApplicationJson)
                    .addBody("console.log(\"Hello, World!\");")
                    .addCookie(cookie)
                    .closeConnection();
```
## Static methods for file loading
JavaScript and HTML files can be easily converted to ```HTTPResponse``` objects using static methods:
* ```HTTPResponse::renderHTML(const std::string& path)``` creates an ```HTTPResponse``` which can be sent to a client to render an HTML page
* ```HTTPResponse::executeJSON(const std::string& path)``` creates an ```HTTPResponse``` which can be sent to a client to execute some JavaScript code

Both of these cache the files used so they are only loaded from disk the first time they are required. Information about file caching and turning it off can be found in ```debugging.md```.