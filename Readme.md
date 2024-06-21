# Webserveau

Webserveau is an asynchronous HTTP server.

# Features
- Handles methods GET, POST, PUT, HEAD and DELETE
- Handles static file upload/deletion via POST, PUT or DELETE
- Modular and versatile configuration system via a .conf file
- Handles multi-transaction connection via keep-alive HTTP headers
- Handles request pipelining
- Handles chunked request sent by client or proxy
- Handles "expect-continue" header for large upload
- Handles interpreted languages CGIs and compiled CGIs.
- Handle multiple host listening to the same port and/or to multiple ports at once. 
- Handles HTTP redirection and CGI redirection (local or client-side)
- Handles dir_listing on allowed routes
- Handles CGI<->client session cookies transaction
- Comes with an handy transparent client sender program, allowing for specific test

# RFC 1.1 compliant

The webserver was made to answer to the HTTP 1.1 specifications given by the RFC.

Although there is some missing methods such as HEAD, PUT, TRACE or OPTIONS, as well as headers related to cache management or authorizations, all the response given to the client are consistent with RFC's specifications.

See **Response.hpp** for the list of implemented HTTP response status code.

**Notes regarding POST and PUT**

**POST** method is currently handled exactly as **PUT** method and the server will behave exactly in the same way for both method (except that CGIs output may differ). The RFC specify that whereas **PUT** method is to be used to apply entity data directly to the ressource identified by the URI, **POST** method should transmit data entity to the ressource identified by the URI. This imply that **PUT** should be used for static upload and  **POST** method should be used addressing script/CGIs URI. However if one choose to allow in the config file static upload for a specific URI, it's his responsability to choose to enable/disable **POST** method for this location, because as long as the URI is not identified as a CGI, a **POST** method will be considered as an attempt of static upload directly to the ressource.

# Asynchronous operations and security considerations

In order to reduce hardware load in term of ressource and memory, all the I/O operations are performed in an asynchronous manner via *epoll()* kernel's API. The idea is that we ensure that every read/write operation with a client is performed when there is actually something to read from the client or something to be send to the client. Therefore none I/O operation would be blocking and the server will be able to serve multiple client at the same time without getting blocked with slow or malicious clients.

For the same reason we limit I/O buffers to 4096 characters to reduce global memory usage and to make sure that even if a client is sending large amount of data, we'll be able to continue serving pending or incoming clients. That means most of client/server interaction will take at least 2 events loop to complete but no client will ever be hanging, and even if the server is at full load, incoming connection will be either rejected or accepted.

When sending data from server to client, large files/pages are chunked into smaller buffers, insuring write operations will also be limited to 4096 characters.

For security considerations against malicious transactions, clients taking too long to send a data previously announced will be timed out from the server. A maximum body size can also be set per host in order to reject any incoming transaction that will take to many events loops to complete.

However no IP ban mechanism is implemented, so client performing repeated malicious transactions will keep doing so during an attack. Therefore if the server is facing an attack by more than 1000 simultaneuous clients, the server is likely to be unable to respond to incoming transactions.

# Dynamic response vs File response

When sending a response, the server is only aware of 2 types of responses, regardless of the associated status code. 

File response is when the server use a local file to send the body of a response. This local file could be a static ressource asked by a client, a CGI output or an error page (403, 404, ...).

Dynamic response is when the server generate itself the body of a response. This occurs for dir_listing body or for some of the error status code that requires a description of the error context. For example this is the case for the Bad Request or Not Implemented status code. It is also used to generate 500 Internal Error, as when such an error occurs, we probably don't want to try reading from a file.

# Limitations

Webserv is limitated is currently limitated to a single process and therefore cannot serve more than 1020 concurents clients.

# Configuration file

Webserveau comes with a default configuration file consisting of 4 different servers listening to 2 port (a unique port and a common port). See [conf/default.conf](conf/default.conf).

Launch ./webserveau with the path of the config file for using another one than *conf/default.conf*.

```
./webserveau [conf_file_path]

```

# Sender client program

See [sender/Readme.md](sender/Readme.md)

# Stressing the server

Feel free to use siege to attack the server or simulate overload situation.
The server is not supposed to crash or leak memory at any point.

```
# Simple HTML page get
siege -c 500 -b http://localhost:8080/index.html

# Heavy get
siege -c 500 -b http://localhost:8080/images/forest.jpeg

# Get on CGI 
siege -c 500 -b "http://localhost:8080/pascal.out?taille=7"

# Post with form data on a CGI 
siege -c 500 -b "http://localhost:8080/form.py POST username=pouetpouet&emailaddress=ping"

# Get on a CGI will an infinite loop (default cgi time out is 10s)
siege -c 500 -b "http://localhost:8080/infinity.py"

```
