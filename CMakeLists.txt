CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

project(VisionModule)

set (VERSION_MAJOR 0)
set (VERSION_MINOR 2)

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Debug CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++0x -DDEBUG -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++0x -Wall")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -Wall")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")

find_package(Boost)

configure_file (
   "${PROJECT_SOURCE_DIR}/src/config.h.in"
   "${PROJECT_BINARY_DIR}/src/config.h"
)

ADD_EXECUTABLE( vision_module
  libvideo/colourdefinition.cpp
  libvideo/floodfillstate.cpp
  libvideo/framebuffer.cpp
  libvideo/framebufferbayer.cpp
  libvideo/framebufferrgb24.cpp
  libvideo/framebufferrgb24be.cpp
  libvideo/framebufferrgb32.cpp
  libvideo/framebufferrgb565.cpp
  libvideo/geometry.cpp
  libvideo/imageprocessing.cpp
  libvideo/integralimage.cpp
  libvideo/jpeg_utils.cpp
  libvideo/pixel.cpp
  libvideo/quadtreedecomposition.cpp
  libvideo/rect.cpp
  libvideo/v4l2device.cpp
  libvideo/filedevice.cpp
  libvideo/videodevice.cpp
  libvideo/visionobject.cpp
  src/configuration.cpp
  src/globals.cpp
  src/httpd.cpp
  src/httpdthread.cpp
  src/httpdserverthread.cpp
  src/httpdclientthread.cpp
  src/main.cpp
  src/serial.cpp
  src/videostream.cpp
  src/udpvisionserver.cpp
)

ADD_DEFINITIONS(
  -DENABLE_V4L2
  -DENABLE_FILE
)

INCLUDE_DIRECTORIES(
  ${PROJECT_SOURCE_DIR}/../libvideo/
)

TARGET_LINK_LIBRARIES(vision_module 
   "boost_program_options"
   "boost_system"
   "boost_thread"
   "v4l2"
   "avcodec" "avformat" "avutil" "swscale" "z" "bz2"
#   "dc1394"
   "pthread"
   "jpeg"
)

ADD_CUSTOM_COMMAND(
  TARGET  vision_module
  POST_BUILD
  COMMAND rm -fr ${CMAKE_BINARY_DIR}/www
  COMMENT Removing www directory
  COMMAND cp -aR ${CMAKE_SOURCE_DIR}/www ${CMAKE_BINARY_DIR}/www
  COMMENT Copying www directories
)

ADD_EXECUTABLE( visionclient
  src/visionclientmain.cpp
)

target_link_libraries(visionclient
   "boost_program_options"
   "boost_system"
   "boost_thread"
   "v4l2"
#   "avcodec" "avformat" "avutil" "swscale" "z" "bz2"
#   "dc1394"
   "pthread"
   "jpeg")
