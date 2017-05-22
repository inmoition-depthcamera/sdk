CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

# INMOTION Depth Camera Lib
SET(SDK_NAME idc)

# define project will set CMAKE_CXX_FLAGS to NULL
PROJECT (${SDK_NAME})

SET(SRCDIR .)
SET(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")

INCLUDE_DIRECTORIES(${SRCDIR})

IF (CMAKE_SYSTEM_NAME MATCHES "Windows")

    file(GLOB_RECURSE SRC_LIST
        "${SRCDIR}/depth_camera_cmd.cpp"
        "${SRCDIR}/depth_camera_uvc.cpp"
        "${SRCDIR}/depth_frame.cpp"
        "${SRCDIR}/filter/*.cpp"
        "${SRCDIR}/os/cmd_interface.cpp"
        "${SRCDIR}/os/cmd_interface_win.cpp"
        "${SRCDIR}/os/uvc_interface.cpp"        
        "${SRCDIR}/os/uvc_interface_direct_show.cpp")

    ADD_LIBRARY(${SDK_NAME}_shared SHARED ${SRC_LIST})
    ADD_LIBRARY(${SDK_NAME}_static STATIC ${SRC_LIST})

    set_target_properties(${SDK_NAME}_shared PROPERTIES OUTPUT_NAME ${SDK_NAME})
    set_target_properties(${SDK_NAME}_static PROPERTIES OUTPUT_NAME ${SDK_NAME})

ELSE()

    option(USE_LIBUDEV "ON to use libudev to get uvc related command port" ON)
	SET(CMAKE_CXX_FLAGS "-Wall -O2 -Werror ${CMAKE_CXX_FLAGS}")
    file(GLOB_RECURSE SRC_LIST
        "${SRCDIR}/depth_camera_cmd.cpp"
        "${SRCDIR}/depth_camera_uvc.cpp"
        "${SRCDIR}/depth_frame.cpp"
        "${SRCDIR}/filter/*.cpp"
        "${SRCDIR}/os/cmd_interface.cpp"
        "${SRCDIR}/os/cmd_interface_linux.cpp"
        "${SRCDIR}/os/uvc_interface.cpp"
        "${SRCDIR}/os/uvc_interface_v4l.cpp")
        
    ADD_LIBRARY(${SDK_NAME}_shared SHARED ${SRC_LIST})
    ADD_LIBRARY(${SDK_NAME}_static STATIC ${SRC_LIST})

    set_target_properties(${SDK_NAME}_shared PROPERTIES OUTPUT_NAME ${SDK_NAME})
    set_target_properties(${SDK_NAME}_static PROPERTIES OUTPUT_NAME ${SDK_NAME})

    TARGET_LINK_LIBRARIES(${SDK_NAME}_shared pthread)
    TARGET_LINK_LIBRARIES(${SDK_NAME}_static pthread)

    IF(USE_LIBUDEV)
        ADD_DEFINITIONS(-DUSE_UDEV)
        TARGET_LINK_LIBRARIES(${SDK_NAME}_shared udev)
        TARGET_LINK_LIBRARIES(${SDK_NAME}_static udev)
    ENDIF()

ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")



