CMAKE_MINIMUM_REQUIRED(VERSION 3.3)

PROJECT (depth_frame_grabber)
SET(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
ADD_EXECUTABLE(depth_frame_grabber depth_frame_grabber.cpp)
TARGET_LINK_LIBRARIES(depth_frame_grabber idc)

if (UNIX)
SET(CMAKE_CXX_FLAGS "-Wall -Werror ${CMAKE_CXX_FLAGS}")
TARGET_LINK_LIBRARIES(depth_frame_grabber pthread)
endif()


PROJECT (full_example)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
ADD_EXECUTABLE(full_example full_example.cpp points_cloud.cpp)
TARGET_LINK_LIBRARIES(full_example idc glfw ${OPENGL_gl_LIBRARY} )

if (UNIX)
SET(CMAKE_CXX_FLAGS "-Wall -Werror ${CMAKE_CXX_FLAGS}")
TARGET_LINK_LIBRARIES(full_example pthread)
endif()