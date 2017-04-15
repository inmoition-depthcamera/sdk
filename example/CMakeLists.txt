CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

PROJECT (depth_frame_grabber)
SET(CMAKE_CXX_FLAGS "-Wall -O2 -std=c++11 -Werror ${CMAKE_CXX_FLAGS}")
ADD_EXECUTABLE(depth_frame_grabber depth_frame_grabber.cpp)
TARGET_LINK_LIBRARIES(depth_frame_grabber pthread idc)


PROJECT (depth_frame_displayer)
find_package(OpenCV REQUIRED)
ADD_EXECUTABLE(depth_frame_displayer depth_frame_displayer.cpp)
target_link_libraries(depth_frame_displayer ${OpenCV_LIBS} pthread idc)