cmake_minimum_required(VERSION 3.20)
project(AVPlay)

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/build/generators")

set(CMAKE_PREFIX_PATH "G:/devtools/QT/6.8.3/msvc2022_64/lib/cmake/Qt6" ${CMAKE_PREFIX_PATH})

find_package(Qt6 COMPONENTS Widgets Core OpenGL OpenGLWidgets Multimedia REQUIRED)
find_package(OpenGL REQUIRED)
find_package(ffmpeg REQUIRED)
find_package(glm REQUIRED)
find_package(stb REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

include_directories(./src)
include_directories(./include)
include_directories(./qt)
include_directories(./3rdparty/InspireFace/include)

set(HEADERS
    qt/MainWindow.h
    qt/view/OpenGLView.h
    qt/UI/PlayerWidget.h
)

add_executable(app
    src/VideoFilter/StickerFilter.cpp
    src/Writer/FileWriter.cpp
    src/Writer/Muxer.cpp
    src/Writer/VideoEncoder.cpp
    src/Writer/AudioEncoder.cpp
    ${HEADERS}
    qt/Main.cpp
    qt/MainWindow.cpp
    qt/view/OpenGLView.cpp
    qt/UI/PlayerWidget.cpp
    src/Core/TaskPool.cpp
    src/Engine/Player.cpp
    src/Engine/AudioPipeline.cpp
    src/Engine/AudioSpeaker.cpp
    src/Engine/VideoDisplayView.cpp
    src/VideoFilter/VideoFilter.cpp
    src/VideoFilter/StickerFilter.cpp
    src/VideoFilter/InvertFilter.cpp
    src/VideoFilter/GrayFilter.cpp
    src/VideoFilter/FlipVerticalFilter.cpp
    src/Engine/GLContext.cpp
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
    stb::stb
    Qt6::Core
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    Qt6::Multimedia
    opengl32
)

