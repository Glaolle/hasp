cmake_minimum_required(VERSION 3.10)

project(usbhaspinfo)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")

add_executable(${PROJECT_NAME} EnumKeys.c HaspError.c HaspInfo.c HaspType.c KeyValue.c LicStatus.c)

#include_directories(".")
target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/libhasplnx.a)
