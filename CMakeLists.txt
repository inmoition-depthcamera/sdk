CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(PROJECT_NAME inmotion_dc_sdk_test)

# define project will set CMAKE_CXX_FLAGS to NULL
PROJECT (${PROJECT_NAME})

SET(SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}/source)
SET(CMAKE_CXX_FLAGS "-Wall -std=c++11 -Werror ${CMAKE_CXX_FLAGS}")

INCLUDE_DIRECTORIES(${SRCDIR})

IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
    option(USE_LIBUDEV "ON to use libudev to get uvc related command port" ON)
    file(GLOB_RECURSE SRC_LIST
        "${SRCDIR}/../msvc/*.cpp"
        "${SRCDIR}/depth_camera_cmd.cpp"
        "${SRCDIR}/depth_camera_uvc.cpp"
        "${SRCDIR}/filter/*.cpp"
        "${SRCDIR}/os/cmd_interface.cpp"
        "${SRCDIR}/os/cmd_interface_linux.cpp"
        "${SRCDIR}/os/uvc_interface_v4l.cpp"
        "${SRCDIR}/os/uvc_interface.cpp")

    ADD_EXECUTABLE(${PROJECT_NAME} ${SRC_LIST})
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} pthread)
    IF(USE_LIBUDEV)
        ADD_DEFINITIONS(-DUSE_UDEV)
        TARGET_LINK_LIBRARIES(${PROJECT_NAME} udev)
    ENDIF()

ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Windows")

    file(GLOB_RECURSE SRC_LIST
        "${SRCDIR}/../msvc/*.cpp"
        "${SRCDIR}/depth_camera_cmd.cpp"
        "${SRCDIR}/depth_camera_uvc.cpp"
        "${SRCDIR}/filter/*.cpp"
        "${SRCDIR}/os/cmd_interface.cpp"
        "${SRCDIR}/os/uvc_interface.cpp"
        "${SRCDIR}/os/cmd_interface_win.cpp"
        "${SRCDIR}/os/uvc_interface_direct_show.cpp")

    ADD_EXECUTABLE(${PROJECT_NAME} ${SRC_LIST})

ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")



