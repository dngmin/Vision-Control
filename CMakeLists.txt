cmake_minimum_required(VERSION 3.12)
project(Vision-Control)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew")
find_package(OpenCV REQUIRED)
find_package(SFML 3 REQUIRED COMPONENTS Graphics Network)

add_executable(${PROJECT_NAME} src/cpp/main.cpp)

# for Apple
if (APPLE)
    find_library(CORE_GRAPHICS CoreGraphics REQUIRED)
    
    target_link_libraries(${PROJECT_NAME} PRIVATE ${CORE_GRAPHICS})
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenCV_LIBS} SFML::Graphics SFML::Network)