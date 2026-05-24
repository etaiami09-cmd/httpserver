# Using Headers
This library does not really provide a direct interface to interacting with headers and creating custom headers. Instead, headers are automatically parsed when ```handleRequest``` is called and then passed to the ```RequestObject``` of the request method.
## Value types
To easily access the value type of a certain header, you can use ```HeaderValueType<header>``` where ```header``` is an ```HTTPHeader``` value. Only one header is required by the HTTP standard to be passed in every request, the ```Host``` header.
## Potential headers
Every other header is optional. Headers can be accessed in ```RequestObject```s through their ```PotentialHeader``` types. ```PotentialHeader``` is defined for every ```HTTPHeader``` as ```std::optional<HeaderValueType<header>>```.

Note: ```PotentialHeader```s exist for ```HTTPResponse```s as well.