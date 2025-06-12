cmake_minimum_required(VERSION 3.20)
project(AVPlay)


list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/build/generators")

add_executable(app
    src/Test/decode.cpp
    src/Reader/DeMuxer.cpp
    src/Reader/AudioDecoder.cpp
    src/Reader/VideoDecoder.cpp
    src/Reader/FileReader.cpp
)

include_directories(./src)

find_package(ffmpeg REQUIRED)
if(FFmpeg_FOUND)
    include_directories(${FFmpeg_INCLUDE_DIRS})
endif()

find_package(glm REQUIRED)

target_link_libraries(app PRIVATE ffmpeg::ffmpeg glm::glm)