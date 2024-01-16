# beast-gres

## Description

This repository contains a simple asynchronous HTTP server built with Boost.Beast that interacts with a PostgreSQL database using the pqxx library. The server provides basic API endpoints for executing SQL queries.

## Usage

To use this application, follow these steps:

1. Ensure you have a PostgreSQL database running.
1. Update the connection details in the PgConnectionPool constructor within main.cpp.
1. Build the application using an appropriate compiler.
1. Run the compiled executable.

## Endpoints:

- **GET /api?=customers**: Execute SELECT sql query.

    ```
    $ curl http://localhost:8080/api?=customers

    [{"email":"bbannerman0@cocolog-nifty.com","id":1,"password":"Bannerman","username":"Boycey"},{"email":"rradborne1@wordpress.org","id":2,"password":"Radborne","username":"Roman"},
    ...
    {"email":"bcelandrq@census.gov","id":999,"password":"Celand","username":"Billi"},
    {"email":"rblaverrr@dmoz.org","id":1000,"password":"Blaver","username":"Rube"}]
    ```

- **POST /api?=login&username=username&password=password**: Execute SELECT sql query with WHERE clause and session storage
    ```
    $ curl -X POST "http://localhost:8080/api?=login" -d "username=Boycey&password=Bannerman"
    $ Authentication successful
    ```


## Dependencies:
- [boost/beast](https://github.com/boostorg/beast): HTTP and WebSocket built on Boost.Asio in C++11

- [nlohmann/json](https://github.com/nlohmann/json): JSON library for C++.

- [pqxx](https://github.com/jtv/libpqxx): C++ library for interacting with PostgreSQL databases.

## Note: