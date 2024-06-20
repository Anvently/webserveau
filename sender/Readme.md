# Sender

The sender program is an easy to use and transparent client program running on console and command line.

It can be used to conduct precise and controlled tests on server response more transparently and conveniently than a web-browser or curl would.

Although it was made to be tested on webserveau, it should work with any webserver by changing ip address and port.

# Usage

```
c++ sender.cpp

./a.out [request_file_to_send]
...
[another_file_request_to_send]
...
[another_file_request_to_send]
...
Ctrl-D or Ctrl-C to close connection
```

It takes as argument a file containing an HTTP request to be read and send to a server. It will keep running as long as the connection with the server is not closed.

While running, the path of a new request file to send can be written in stdin. This is really usefull to test request pipelining, multiple transactions connection via keep-alive or to test server response to "expect: 100-continue" header.

The client sender uses *epoll()* to read data from the server and write data to its at the same time.

# Configuration

## Server address/port

You can update the defines in **sender.cpp** to change the ip and/or the port you want to send your requests to.

## Buffer size

BUFFER_SIZE macro can be updated to change the size of each read/write of the client. This also impact the number of characters that will be read by the server at each event loop.

Default buffer size is 100 characters.

## Pause time

PAUSE_TIME can be used to add a delay between each write from the client. This can simulate a transaction with a slow client or can be used in addition with a small buffer_size value to test the reaction of the server when slowly parsing very small chunks of data.

PAUSE_TIME is NOT a sleep time, therefore having a slow writing time will not impact reading the data sent by the server.

Default pause time is 10ms.

# HTTP request file

HTTP protocol requires line breaks to be **\r\n** instead of simple **\n**. Therefore you need to make sure your editor is configured correctly when writing custom HTTP request files.

On Vscode, you can edit the line breaks in the bottom bar of the editor by changing **LF** property to **CRLF**.

The provided request file examples already contain **\r\n** as line breaks.

# Test scenarios

With the sender comes lot of scenario example based on the default configuration file of webserveau.

All the request are sent to 127.0.0.1:8000 (port at which the each configured host is listening), the actual host to which the request is intended for is defined in the host header.
