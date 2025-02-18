# Project Specification and Documentation

## Project proposal

**Topic:** HTTP server library

### Rough idea

Implement a C++ library providing public API to create and run HTTP server with custom options

### HTTP protocol
 - 

### Features

#### Basic internal

These are the must-have features to even claim it is a HTTP server.

 1. Handle HTTP requests
    - accept incoming connections on a specified port
    - parse and process HTTP requests
 2. Serve static files
    - read files from a **public directory** and send them as responses
    - support for common **MIME types** (`.html`, `.css`, ...)
 3. Respond with HTTP headers
    - return an appropriate **HTTP status code**

#### API 

### Dependencies

 1. `sys/socket.h`
