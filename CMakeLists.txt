CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

#set(CMAKE_C_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG} -DDEBUG)
#set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG )

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
  libvideo/pixel.cpp
  libvideo/quadtreedecomposition.cpp
  libvideo/rect.cpp
  libvideo/v4l2device.cpp
  libvideo/videodevice.cpp
  src/main.cpp
  src/videostream.cpp
  src/httpd.cpp
  src/jpeg_utils.cpp
)

ADD_DEFINITIONS(
  -DENABLE_V4L2
  -DDEBUG
)

INCLUDE_DIRECTORIES(
  ${PROJECT_SOURCE_DIR}/../libvideo/
)

TARGET_LINK_LIBRARIES(vision_module 
   -lboost_program_options 
   -lv4l2 
   -lavcodec -lavformat -lavutil -lswscale -lz -lbz2 
   -ldc1394 
   -lpthread
   -ljpeg
)

ADD_CUSTOM_COMMAND(
  TARGET  vision_module
  POST_BUILD
  COMMAND rm -fr ${CMAKE_BINARY_DIR}/www
  COMMENT Removing www directory
  COMMAND cp -aR ${CMAKE_SOURCE_DIR}/www ${CMAKE_BINARY_DIR}/www
  COMMENT Copying www directories
)
