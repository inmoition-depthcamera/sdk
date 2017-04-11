CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(PROJECT_NAME inmotion_dc_sdk)

# define project will set CMAKE_CXX_FLAGS to NULL
PROJECT (${PROJECT_NAME})

option(USE_LIBUDEV "ON to use libudev to get uvc related command port" ON)
option(USE_LIBUVC "ON to use libuvc to replace v4l2 to get uvc data from camera" OFF)

SET(SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}/source)
SET(CMAKE_CXX_FLAGS "-O2 -Wall -std=c++11 -Werror ${CMAKE_CXX_FLAGS}")

IF (CMAKE_SYSTEM_NAME MATCHES "Linux")

    file(GLOB_RECURSE SRC_LIST
        "${SRCDIR}/../msvc/*.cpp"
        "${SRCDIR}/depth_camera_cmd.cpp"
        "${SRCDIR}/depth_camera_uvc.cpp"
        "${SRCDIR}/filter/*.cpp"
        "${SRCDIR}/os/cmd_interface.cpp"
        "${SRCDIR}/os/cmd_interface_linux.cpp"
        "${SRCDIR}/os/uvc_interface.cpp"
        "${SRCDIR}/os/uvc_interface_v4l.cpp"
    )

    IF(USE_LIBUVC)
        SET(SRC_LIST SRC_LIST ";${SRCDIR}/os/uvc_interface_libuvc.cpp")
        add_definitions(-DUVC_DRIVER_LIBUVC)
    ENDIF()

    INCLUDE_DIRECTORIES(${SRCDIR})
    ADD_EXECUTABLE(${PROJECT_NAME} ${SRC_LIST})

    

    TARGET_LINK_LIBRARIES(${PROJECT_NAME} pthread)

    IF(USE_LIBUDEV)
        add_definitions(-DUSE_UDEV)
        TARGET_LINK_LIBRARIES(${PROJECT_NAME} udev)
    ENDIF()

    IF(USE_LIBUVC)
        TARGET_LINK_LIBRARIES(${PROJECT_NAME} libuvc)
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
        "${SRCDIR}/os/uvc_interface_direct_show.cpp"
        "${SRCDIR}/os/uvc_interface_libuvc.cpp"
    )

    IF(USE_LIBUVC)
        SET(SRC_LIST SRC_LIST ";${SRCDIR}/os/uvc_interface_libuvc.cpp")
        add_definitions(-DUVC_DRIVER_LIBUVC)
    ENDIF()

    INCLUDE_DIRECTORIES(${SRCDIR})
    ADD_EXECUTABLE(${PROJECT_NAME} ${SRC_LIST})

    IF(USE_LIBUVC)
        TARGET_LINK_LIBRARIES(${PROJECT_NAME} libuvc)
    ENDIF()

ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")



