cmake_minimum_required(VERSION 3.27)
project(AlcoWindow CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED on)

include_directories("inc")
set(INC_FILES "AlcoWindow.hpp")
set(SRC_FILES "AlcoWindow.cpp")

add_library(${PROJECT_NAME} STATIC inc/${INC_FILES} src/${SRC_FILES})
