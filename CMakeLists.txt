CMAKE_MINIMUM_REQUIRED(VERSION 2.8)


# define project will set CMAKE_CXX_FLAGS to NULL
PROJECT (inmotion_dc_sdk)

SET(SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}/source)
SET(CMAKE_CXX_FLAGS "-O2 -Wall -std=c++11 -Werror ${MY_CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS}")
message("CXX FLAGS: ${CMAKE_CXX_FLAGS}")
# Set Source Files
file(GLOB_RECURSE SRC_LIST
"${SRCDIR}/../msvc/*.cpp"
"${SRCDIR}/depth_camera_cmd.cpp"
"${SRCDIR}/depth_camera_uvc.cpp"
"${SRCDIR}/filter/*.cpp"
"${SRCDIR}/os/cmd_interface.cpp"
"${SRCDIR}/os/cmd_interface_linux.cpp"
"${SRCDIR}/os/uvc_interface.cpp"
"${SRCDIR}/os/uvc_interface_v4l.cpp"
"${SRCDIR}/os/uvc_interface_libuvc.cpp"
)

INCLUDE_DIRECTORIES(${SRCDIR})

ADD_EXECUTABLE(inmotion_dc_sdk ${SRC_LIST})

TARGET_LINK_LIBRARIES(inmotion_dc_sdk pthread)
