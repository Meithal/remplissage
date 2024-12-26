cmake_minimum_required(VERSION 3.29)
project(fenetrage_remplissage C)

set(CMAKE_C_STANDARD 23)

add_executable(fenetrage_remplissage main.c glad/src/glad.c)

target_include_directories(fenetrage_remplissage PRIVATE glad/include)
target_include_directories(fenetrage_remplissage PRIVATE glfw/include)

add_subdirectory(glfw)
target_link_libraries(fenetrage_remplissage PRIVATE glfw)