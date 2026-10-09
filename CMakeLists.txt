cmake_minimum_required(VERSION 3.20)
project(log_task_service LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(log_parser src/parser.cpp)
target_include_directories(log_parser
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_compile_options(log_parser PRIVATE -Wall -Wextra -Wpedantic)
target_compile_features(log_parser PUBLIC cxx_std_17)

add_executable(log_cli src/main.cpp)
target_link_libraries(log_cli PRIVATE log_parser)
target_compile_options(log_cli PRIVATE -Wall -Wextra -Wpedantic)