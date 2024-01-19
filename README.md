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

## Dependencies:
- [boost/beast](https://github.com/boostorg/beast): HTTP and WebSocket built on Boost.Asio in C++11

- [nlohmann/json](https://github.com/nlohmann/json): JSON library for C++.

- [pqxx](https://github.com/jtv/libpqxx): C++ library for interacting with PostgreSQL databases.

## Note:

I used [mockaroo](https://www.mockaroo.com/) for mock sql data, simply use their service to create the mock_data table and change the columns ```first_name``` ```last_name``` to ```username``` ```password```.
