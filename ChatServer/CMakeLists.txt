cmake_minimum_required(VERSION 3.20)

project(websocket_demo LANGUAGES CXX)

add_executable(websocket_demo main.cpp Session.cpp Server.cpp)

target_compile_features(websocket_demo PRIVATE cxx_std_17)

target_compile_definitions(websocket_demo PRIVATE
    _WIN32_WINNT=0x0601
)

target_include_directories(websocket_demo PRIVATE
    D:/boost_1_90_0
)
