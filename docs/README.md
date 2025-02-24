# Project Specification and Documentation

## Project proposal

**Topic:** HTTP server library \
**Description:** Implement a C++ library providing public API to create and run HTTP (1.1 version) server with custom options described [here](#customizable)

### Features

#### Basic

These are the must-have features to even claim it is a HTTP server.

 1. Handle HTTP requests
    - accept incoming connections on a specified port (on both *Linux* and *Windows*)
      - will require to implement abstraction on top of `socket` from both platform-specific libraries
    - parse and process HTTP requests
 2. Serve static files
    - read files from a **public directory** and send them as responses
    - support for common **MIME types** (`.html`, `.css`, ...)
 3. Respond with HTTP headers
    - return an appropriate **HTTP status code**, *response* and *representation* headers
 4. Support persistent connections
    - Connection: keep-alive
 5. Run as a background service (daemon)

#### Customizable

Features that can be enabled/disabled, used/ignored by the library user.

 6. Custom routing
    - Map URLs to files or define custom handler functions
 7. Custom error pages
    - Map errors to individual custom HTML files
 8. Directory listing
    - List a directory content when an `index.html` file is missing
 9. Logging
    - Log requests, errors and server activity

### Dependencies

 1. `sys/socket.h`
 2. `winsock2.h`
