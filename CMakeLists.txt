cmake_minimum_required(VERSION 3.10)
project(LocalTransport)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(sender_test src/sender_test.cc)
add_executable(receiver_test src/receiver_test.cc)

add_subdirectory(src)

target_link_libraries(sender_test local_transport)
target_link_libraries(receiver_test local_transport)
