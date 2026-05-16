cmake_minimum_required(VERSION 3.10)
project(lzw_compression CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(OpenCV REQUIRED)

add_executable(${PROJECT_NAME}
        src/main.cpp
        src/lzw.cpp
        src/image_io.cpp
        include/image_io.h
        include/lzw.h
)

target_include_directories(${PROJECT_NAME} PRIVATE include ${OpenCV_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenCV_LIBS})