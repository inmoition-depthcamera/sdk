CMAKE_MINIMUM_REQUIRED(VERSION 3.3)

PROJECT (depth_frame_grabber)
SET(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
SET(SE_SRCDIR ./simple_grabber)
ADD_EXECUTABLE(depth_frame_grabber ${SE_SRCDIR}/depth_frame_grabber.cpp)
TARGET_LINK_LIBRARIES(depth_frame_grabber idc)

if (UNIX)
SET(CMAKE_CXX_FLAGS "-Wall -Werror ${CMAKE_CXX_FLAGS}")
TARGET_LINK_LIBRARIES(depth_frame_grabber pthread)
endif()


PROJECT (full_example)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
SET(FE_SRCDIR ./full_example)
file(GLOB_RECURSE FE_SRC_LIST
    "${FE_SRCDIR}/imgui/*.cpp"
    "${FE_SRCDIR}/full_example.cpp"
    "${FE_SRCDIR}/model_view.cpp"
    "${FE_SRCDIR}/points_cloud.cpp")
INCLUDE_DIRECTORIES(${FE_SRCDIR} ${FE_SRCDIR}/imgui)
ADD_EXECUTABLE(full_example ${FE_SRC_LIST})
TARGET_LINK_LIBRARIES(full_example idc glfw ${OPENGL_gl_LIBRARY} )

if (UNIX)
SET(CMAKE_CXX_FLAGS "-Wall -Werror ${CMAKE_CXX_FLAGS}")
TARGET_LINK_LIBRARIES(full_example pthread)
endif()

PROJECT (depth_frame_grabber_cmd_video)
SET(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
SET(SE_SRCDIR ./simple_grabber_cmd_video)
ADD_EXECUTABLE(depth_frame_grabber_cmd_video ${SE_SRCDIR}/depth_frame_grabber_cmd_video.cpp)
TARGET_LINK_LIBRARIES(depth_frame_grabber_cmd_video idc)

if (UNIX)
SET(CMAKE_CXX_FLAGS "-Wall -Werror ${CMAKE_CXX_FLAGS}")
TARGET_LINK_LIBRARIES(depth_frame_grabber_cmd_video pthread)
endif()

PROJECT (full_example_cmd_video)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
SET(FE_SRCDIR ./full_example_cmd_video)
file(GLOB_RECURSE FE_SRC_LIST
    "${FE_SRCDIR}/imgui/*.cpp"
    "${FE_SRCDIR}/full_example_cmd_video.cpp"
    "${FE_SRCDIR}/model_view.cpp"
    "${FE_SRCDIR}/points_cloud.cpp")
INCLUDE_DIRECTORIES(${FE_SRCDIR} ${FE_SRCDIR}/imgui)
ADD_EXECUTABLE(full_example_cmd_video ${FE_SRC_LIST})
TARGET_LINK_LIBRARIES(full_example_cmd_video idc glfw ${OPENGL_gl_LIBRARY} )

if (UNIX)
SET(CMAKE_CXX_FLAGS "-Wall -Werror ${CMAKE_CXX_FLAGS}")
TARGET_LINK_LIBRARIES(full_example_cmd_video pthread)
endif()

