cmake_minimum_required(VERSION 3.20)
project(AVPlay)

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/build/generators")

set(CMAKE_PREFIX_PATH "G:/devtools/QT/6.8.3/msvc2022_64/lib/cmake/Qt6" ${CMAKE_PREFIX_PATH})

find_package(Qt6 COMPONENTS Widgets OpenGL REQUIRED)
find_package(ffmpeg REQUIRED)
find_package(glm REQUIRED)

include_directories(./src)
include_directories(./include)


add_executable(app
    src/Utils/GLUtils.cpp
    src/Engine/VideoPipeline.cpp
    src/Reader/DeMuxer.cpp
    src/Reader/AudioDecoder.cpp
    src/Reader/VideoDecoder.cpp
    src/Reader/FileReader.cpp
    src/Core/SyncNotifier.cpp
    src/Engine/AVSynchronizer.cpp
)

target_link_libraries(app PRIVATE 
    ffmpeg::ffmpeg 
    glm::glm
    Qt6::Widgets
    Qt6::OpenGL
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)