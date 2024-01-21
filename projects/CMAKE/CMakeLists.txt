cmake_minimum_required(VERSION 3.11)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

project(rguilayout C)

find_package(Raylib REQUIRED)

# Directory Variables
set(RGUILAYOUT_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../src)
set(RGUILAYOUT_HEADERS "")
set(RGUILAYOUT_SOURCES "")

# rguilayout
file(GLOB RGUILAYOUT_HEADERS ${RGUILAYOUT_ROOT}/*.h)
file(GLOB RGUILAYOUT_SOURCES ${RGUILAYOUT_ROOT}/*.c)
file(GLOB EXTERNAL_HEADERS ${RGUILAYOUT_ROOT}/external/*.h)
file(GLOB EXTERNAL_SOURCES ${RGUILAYOUT_ROOT}/external/*.c)

add_executable(rguilayout
    ${EXTERNAL_HEADERS}
    ${RGUILAYOUT_HEADERS}
    ${RGUILAYOUT_SOURCES}
    ${EXTERNAL_SOURCES}
)

set_target_properties(rguilayout PROPERTIES SOURCE_DIR ${RGUILAYOUT_ROOT})

target_include_directories(
    rguilayout PRIVATE
    ${RGUILAYOUT_ROOT}/external
)

target_link_libraries(
    rguilayout PRIVATE
    raylib
)