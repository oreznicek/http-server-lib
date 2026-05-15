# User documentation

## Library build

Before building the library, check the [system requirements](./system_requirements.md).

### Automatic build

The project includes automated build scripts that configure CMake, compile the library, build examples, and automatically run tests to validate your environment.

Open a terminal in the root of the `project` folder and run the following command
- `./build.sh` for Linux
- `./build.bat` for Windows

### Manual build

The following sequence of commands is run inside the `project` folder:

```
mkdir <BUILD-DIR>
cd <BUILD-DIR>
cmake ..
```

This creates the necessary configuration for the build and then there are three options to choose from:
1. `make` builds only the library in static form
2. `make tests` builds the library with a set of tests, which can then be run in bulk using `ctest` or individually - by finding and running the given test (e.g. `<BUILD-DIR>/tests/parser_test`)
3. `make examples` builds a few examples located in the examples folder. Each of them demonstrates a different feature of the library

## Using the library

Check out the [examples](@ref examples_index) to see some actual usage of the library, by looking at some code :).

## Configuration

The server configuration is done using the `::http::ServerBuilder`.

### Default configuration

- public is the folder from which the server is run in the command-line
- default error page:

```html
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Error {0}</title>
    <style>
        body {{
            font-family: sans-serif;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            margin: 0;
            background-color: #f9f9f9;
            color: #333;
            text-align: center;
        }}
        h1 {{ margin: 0; font-size: 8rem; color: #e74c3c; }}
        p {{ margin: 0; font-size: 3rem; color: #666; }}
    </style>
</head>
<body>
    <div>
        <h1>{0}</h1>
        <p>{1}</p>
    </div>
</body>
</html>
```

- Directory listing is disabled
- Port: **8080**
- Server is **dual-stack**, so it supports both IPv4 and IPv6 connections
- Limits:
  - Headers: 8 KiB
  - Bodies: 4 MiB
  - Request Target: 8 KiB
- Timeout: 5 seconds
- Thread count: number of logical processors

### Logging settings

Logging is the only feature of the library that is not enabled via `http::ServerBuilder`, but through `::logger`.
