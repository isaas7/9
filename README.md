# beast-gres

## Description

This repository contains a simple asynchronous HTTP server built with Boost.Beast that interacts with a PostgreSQL database using the pqxx library. The server provides basic API endpoints for executing SQL queries.

## Usage

To use this application, follow these steps:

1. Ensure you have a PostgreSQL database running.
1. Update the connection details in the PgConnectionPool constructor within main.cpp.
1. Build the application using an appropriate compiler.
1. Run the compiled executable.

## Features:

### `main.cpp`:

- Initializes a `PgConnectionPool` with connection details and schema.
- Configures a Boost.Beast HTTP server to listen on a specified address, port, and number of threads.
- Starts the server to handle incoming HTTP requests.

### `handler.cpp`:

- Implements request handling for `/api/user/login` and `/api/user/register` endpoints.
- Responds with appropriate HTTP status codes and messages based on request processing.

### `listener.cpp`:

- Listens for incoming connections using Boost.Asio.
- Accepts incoming connections and creates a `session` for each connection.

### `session.cpp`:

- Manages the execution of sessions for incoming connections.
- Reads HTTP requests asynchronously and triggers request processing.
- Implements request processing using `handle_request` and sends responses accordingly.
- Handles session closure and cleanup.

### `pgconnectionpool.cpp`:

- Implements a PostgreSQL connection pool using pqxx.
- Provides methods for querying the database, including username and password validation.
- Manages session creation and checks for active sessions before allowing logins.

## Dependencies:
- [boost/beast](https://github.com/boostorg/beast): HTTP and WebSocket built on [Boost.Asio](https://github.com/boostorg/asio) in C++11

- [nlohmann/json](https://github.com/nlohmann/json): JSON library for C++.

- [pqxx](https://github.com/jtv/libpqxx): C++ library for interacting with PostgreSQL databases.

- [spdlog](https://github.com/gabime/spdlog): Fast C++ logging library

