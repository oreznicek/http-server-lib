# Programmer documentation

If the server configuration is correct, the server constructor is called inside `::http::ServerBuilder::build()`, where all the configuration is stolen and passed to the correct places and the most important thing is that a `net::ServerSocket` is created, which listens on the given address and waits for someone to connect.

The entry point of the library is a loop inside the `::http::Server::run()` function:

```c++
while (is_running_) {
    SocketAddr6 client_addr;
    // The poll function checks if a client has connected (waits for at most 100ms):
    // - if not:
    // - returns an invalid `ClientSocket`
    // - if yes
    // - provides us with a client that we can serve
    ClientSocket csock = ssock_.poll(client_addr, &timeout_);
    if (!csock.is_valid()) {
        continue;
    }
    // At this point, we add a function to serve the client to the thread-safe queue
    // A free worker thread will pick up this work and process the request
    // If there is no free worker thread, the Task must sit in the queue
    pool_.submit_task([this, conn = Connection(std::move(csock))]() mutable {
        handle_client(std::move(conn));
    });
}
```

`::http::Connection` class is very important, it encapsulates `net::ClientSocket` and allows for more convenient reading of data that comes to us over the network.

The private function `::http::Server::handle_client(Connection&&)` could be called the heart of the library:

```c++
Request request;
Response response;

try {
    // We want to parse the request
    auto req = parser_.parse_request(conn);

    if (req.has_value()) {
        request = *req;
        // Request parsed successfully... The router will know what to do with it
        // The router either finds (or doesn't -> 404) a specific file or runs a
        // custom function that the library user added by using ServerBuilder::add_route
        response = router_.handle_request(request);
    } else if (req.error().code == StatusCode::None) {
        return; // client closed
    } else {
        // Request didn't parse successfully
        // We return error to the client
        response = Response(req.error());
    }
} catch (...) {
    log_exception(std::current_exception());
    response = Response(ServerErr(StatusCode::InternalServerError));
}

try {
    conn.send(response.to_string());
} catch (...) {
    log_exception(std::current_exception());
}
```

## Tests

A very interesting part of the code are the tests. In the header file `tests/test_common.hpp` there is the main macro `TEST_CASE`, which is used to create tests. They are gradually added to the `TestRegistry` class, which maintains them, and a common `main()` function from `tests/test_main.cpp` is linked to each test.

I admit that I stole the `TEST_CASE` macro from the last assignment for advanced C++, which I completed last year.

## Progress of work

Unfortunately, I was unable to fulfill everything from the defined assignment. Specifically:
- keep-alive
- running server as background daemon

The project was tough nut to crack. An HTTP server-type application must be written precisely and almost every part of it should be properly tested. Unfortunately, I only wrote tests for the parts that I thought were important. Writing tests was quite time-consuming.

Also, all parts of the application are very closely related to each other. Moreover, network errors are sometimes very difficult to debug. I definitely should have added logging to the project much earlier, that would have helped me the most. I guess it's standard for network applications to list every little thing so that you know what's going on.
