cmake_minimum_required(VERSION 3.16)
project(DynamicTaxiDispatchSystem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(taxi_dispatch
    src/main.cpp
    src/DispatchCenter.cpp
    src/Driver.cpp
    src/Passenger.cpp
    src/Location.cpp
    src/Ride.cpp
    src/FareCalculator.cpp
    src/NearestDriverStrategy.cpp
)

target_include_directories(taxi_dispatch PRIVATE include)
