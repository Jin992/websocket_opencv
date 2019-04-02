cmake_minimum_required(VERSION 3.1)
project(websocket_test)


set(CMAKE_CXX_STANDARD 11)
#-Wpedantic -Wshadow -Wundef -Wno-missing-field-initializers -Werror
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra ")

include_directories(websocketpp)

find_package(Boost 1.58 COMPONENTS system regex  filesystem REQUIRED)
find_package( OpenCV REQUIRED )

include_directories(${PROJECT_SOURCE_DIR})


add_executable(websocket_server main.cpp server_src/base_server.cpp server_src/base_server.h)
target_link_libraries(websocket_server  ${Boost_LIBRARIES} ${OpenCV_LIBS} -pthread  -lavformat -lavcodec -lavdevice -lavutil -lswscale  )
add_executable(websocket_client client.cpp client_src/websocket_endpoint.cpp client_src/connection_metadata.cpp client_src/x264encoder.cpp client_src/x264encoder.h client_src/x264decoder.cpp client_src/x264decoder.h)
target_link_libraries(websocket_client  ${Boost_LIBRARIES} ${OpenCV_LIBS} -pthread -lavformat -lavcodec -lavdevice -lavutil -lswscale -lx264)

