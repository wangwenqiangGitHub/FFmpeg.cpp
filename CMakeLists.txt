cmake_minimum_required(VERSION 3.0)

project(ffmpeg_api)

set(CMAKE_CXX_FLAGS "-std=c++17 -g -Wall -pthread ")
set(CMAKE_C_FLAGS "-std=c98 -g -Wall -pthread ")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	)
# link_directories(
# 	{CMAKE_CURRENT_SOURCE_DIR}/libjpeg
# 	)
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
	jpeg
	)

# target_include_directories(${PROJECT_NAME} PUBLIC libjpeg)
