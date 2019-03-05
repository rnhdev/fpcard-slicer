cmake_minimum_required(VERSION 2.8)
project(fpcard_slicer)

set(CMAKE_CXX_STANDARD 11)
set(NBIS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/third_party/nbis)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter -std=c++11 -Wall  -Wno-reorder -Wno-deprecated-declarations")

link_directories(${NBIS_PATH}/exports/lib)

set(INCLUDE_PATH
    include
    third_party/lodepng)

set(SRC
    include/parse_arguments.h
    include/image.h
    include/slicer.h
    src/image.cpp
    src/slicer.cpp
    src/parse_arguments.cpp)

set(LODEPNG_SRC
    third_party/lodepng/lodepng.cpp
    third_party/jpeg/jpeg.cpp
    third_party/jpeg/jpeg.h)

add_executable(fpcard_slicer ${SRC} ${LODEPNG_SRC} src/main.cpp)
target_include_directories(fpcard_slicer PRIVATE ${INCLUDE_PATH})

target_link_libraries(fpcard_slicer m png jpeg)


