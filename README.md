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

- **POST /api?=login&username=username&password=password** 

Submit a post request with username and password and receive session token upon success.  
    
```
$ curl -i --location --request POST 'http://localhost:8080/api?=login&username=Boycey&password=Bannerman' | grep -i authorization
% Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                Dload  Upload   Total   Spent    Left  Speed
100    25  100    25    0     0  15974      0 --:--:-- --:--:-- --:--:-- 25000
Authorization: Bearer session_1705475948932646768
```

- **GET /api?=authenticated_products** 

Submit a get request with a valid session token and the product data is retrieved from the database.  

```
$ curl --location 'http://localhost:8080/api?=authenticated_products' \
--header 'Authorization: Bearer session_1705475948932646768'
```

Response
```
[{"id":1,"name":"Product 1","price":8.36,"quantity":"38"},{"id":2,"name":"Product 2","price":302.53,"quantity":"4"},
...
{"id":99,"name":"Product 99","price":344.62,"quantity":"99"},{"id":100,"name":"Product 100","price":251.06,"quantity":"39"}]
```


## Dependencies:
- [boost/beast](https://github.com/boostorg/beast): HTTP and WebSocket built on Boost.Asio in C++11

- [nlohmann/json](https://github.com/nlohmann/json): JSON library for C++.

- [pqxx](https://github.com/jtv/libpqxx): C++ library for interacting with PostgreSQL databases.

## Note:

I used [mockaroo](https://www.mockaroo.com/) for mock sql data, simply use their service to create the mock_data table and change the columns ```first_name``` ```last_name``` to ```username``` ```password```.
