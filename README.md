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

## Benchmark
```
This is ApacheBench, Version 2.3 <$Revision: 1879490 $>
Copyright 1996 Adam Twiss, Zeus Technology Ltd, http://www.zeustech.net/
Licensed to The Apache Software Foundation, http://www.apache.org/

Benchmarking localhost (be patient)
Completed 1000 requests
Completed 2000 requests
Completed 3000 requests
Completed 4000 requests
Completed 5000 requests
Completed 6000 requests
Completed 7000 requests
Completed 8000 requests
Completed 9000 requests
Completed 10000 requests
Finished 10000 requests


Server Software:        Boost.Beast/351
Server Hostname:        localhost
Server Port:            8080

Document Path:          /api?=authenticated_products
Document Length:        0 bytes

Concurrency Level:      10
Time taken for tests:   1.352 seconds
Complete requests:      10000
Failed requests:        0
Non-2xx responses:      10000
Total transferred:      1050000 bytes
HTML transferred:       0 bytes
Requests per second:    7394.44 [#/sec] (mean)
Time per request:       1.352 [ms] (mean)
Time per request:       0.135 [ms] (mean, across all concurrent requests)
Transfer rate:          758.22 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.2      0       8
Processing:     0    1   0.4      1      12
Waiting:        0    1   0.4      1       8
Total:          1    1   0.5      1      14

Percentage of the requests served within a certain time (ms)
  50%      1
  66%      1
  75%      1
  80%      2
  90%      2
  95%      2
  98%      2
  99%      2
 100%     14 (longest request)
```

## Dependencies:
- [boost/beast](https://github.com/boostorg/beast): HTTP and WebSocket built on Boost.Asio in C++11

- [nlohmann/json](https://github.com/nlohmann/json): JSON library for C++.

- [pqxx](https://github.com/jtv/libpqxx): C++ library for interacting with PostgreSQL databases.

## Note:

I used [mockaroo](https://www.mockaroo.com/) for mock sql data, simply use their service to create the mock_data table and change the columns ```first_name``` ```last_name``` to ```username``` ```password```.
