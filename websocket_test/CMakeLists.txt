cmake_minimum_required(VERSION 3.1)
project(websocket_test)

include_directories(websocketpp)

find_package(Boost 1.58 COMPONENTS system regex  filesystem REQUIRED)
include_directories(${PROJECT_SOURCE_DIR})


add_executable(websocket_server main.cpp )
target_link_libraries(websocket_server  ${Boost_LIBRARIES} -pthread)
add_executable(websocket_client client.cpp)
target_link_libraries(websocket_client  ${Boost_LIBRARIES} -pthread)

