cmake_minimum_required(VERSION 3.10)

project(minimal_egl_glad)
find_package(OpenGL REQUIRED COMPONENTS OpenGL EGL)

add_library(glad STATIC glad/src/glad.c)
target_include_directories(glad PUBLIC glad/include)
target_link_libraries(glad PUBLIC ${CMAKE_DL_LIBS})

add_executable(main src/main.cpp)
target_link_libraries(main PRIVATE glad OpenGL::GL OpenGL::EGL)