CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

PROJECT (depth_frame_grabber)
SET(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
ADD_EXECUTABLE(depth_frame_grabber depth_frame_grabber.cpp)
TARGET_LINK_LIBRARIES(depth_frame_grabber idc)

IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
SET(CMAKE_CXX_FLAGS "-Wall -O2 -Werror ${CMAKE_CXX_FLAGS}")
TARGET_LINK_LIBRARIES(depth_frame_grabber pthread)
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")

PROJECT (depth_frame_displayer)
SET(OpenCV_DIR "D:/Programs/opencv/build/install")
find_package(OpenCV REQUIRED)
ADD_EXECUTABLE(depth_frame_displayer depth_frame_displayer.cpp)
TARGET_LINK_LIBRARIES(depth_frame_displayer ${OpenCV_LIBS} idc)

IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
SET(CMAKE_CXX_FLAGS "-Wall -O2 -Werror ${CMAKE_CXX_FLAGS}")
TARGET_LINK_LIBRARIES(depth_frame_displayer pthread)
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")

PROJECT (depth_frame_displeyer_3d)
ADD_EXECUTABLE(depth_frame_displeyer_3d depth_frame_displeyer_3d.cpp)
TARGET_LINK_LIBRARIES(depth_frame_displeyer_3d ${OpenCV_LIBS} idc glfw gl)

IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
SET(CMAKE_CXX_FLAGS "-Wall -O2 -Werror ${CMAKE_CXX_FLAGS}")
TARGET_LINK_LIBRARIES(depth_frame_displeyer_3d pthread)
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")