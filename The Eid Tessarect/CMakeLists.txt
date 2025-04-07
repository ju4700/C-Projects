cmake_minimum_required(VERSION 3.20)
project(EidMubarakTesseract)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE
        OpenGL::GL
        OpenGL::GLU
        GLUT::GLUT
)