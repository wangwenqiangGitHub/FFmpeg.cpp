cmake_minimum_required(VERSION 3.0)

project(ffmpeg_api)

set(CMAKE_CXX_FLAGS "-std=c++17 -g -Wall -pthread ")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	)

file(GLOB SRC
	${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/*/*.c
	${CMAKE_CURRENT_SOURCE_DIR}/*/*.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/*/*/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/*/*/*.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/*/*/*/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/*/*/*/*.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/*/*.h)

add_executable(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME}
	avformat
	avcodec
	avutil
	swresample
	avfilter
	swscale
	avdevice
	)
