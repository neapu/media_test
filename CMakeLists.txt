cmake_minimum_required(VERSION 3.20)
project(media_test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(logger)

set(FFMPEG_LIBS avcodec avdevice avfilter avformat avutil swscale)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.cpp include/*.h)
add_executable(${PROJECT_NAME} ${SRC_FILES})
if(APPLE)
target_link_directories(${PROJECT_NAME} PRIVATE /usr/local/lib)
target_include_directories(${PROJECT_NAME} PRIVATE /usr/local/include)
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE ${FFMPEG_LIBS} Logger)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/logger)
