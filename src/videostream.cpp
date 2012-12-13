/*
 * mjpg_streamer.cpp
 *
 *  Created on: 2011. 1. 4.
 *      Author: zerom
 */

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <string>

#if defined(ENABLE_DC1394)
#   include "dc1394device.h"
#endif

#if defined(ENABLE_FILE)
#   include "filedevice.h"
#endif

#if defined(ENABLE_V4L2)
#   include "../libvideo/v4l2device.h"
#endif

#include "videostream.h"

#ifdef DEBUG
#include <iostream>
using namespace std;
#endif

#include <arpa/inet.h>
#include <linux/videodev2.h>

#include "videostream.h"
#include "../streamer/jpeg_utils.h"

#include "../libvideo/framebuffer.h"
#include "../libvideo/framebufferrgb24.h"

#define WWW_FOLDER          "./www/"

using namespace std;

VideoStream::VideoStream(string driver,
			 string name,
			 string input,
			 string standard,
			 unsigned int fps,
			 unsigned int width,
			 unsigned int height,
			 unsigned int depth,
			 unsigned int numBuffers
			 )
  : done( true )
{
  if (0)
    {
    }
#if defined(ENABLE_V4L2)
  else if ( driver == "V4L2" )
    {
      device = new V4L2Device( name, input, standard, fps, width, height, depth , numBuffers );
    }
#endif
#if defined(ENABLE_DC1394)
  else if ( driver == "DC1394" )
    {
      device = new DC1394Device( name, input, standard, bayer, fps, width, height, depth, numBuffers );
    }
#endif
#if defined(ENABLE_FILE)
  else if ( driver == "File" )
    {
      device = new FileDevice( name, fps, width, height, depth );
    }
#endif
  else
    {
      std::cerr << __FILE__ << __LINE__ << ":" << __PRETTY_FUNCTION__ 
		<< "Unknown video capture device " << driver << std::endl;
      std::exit( 1 );
    }

  if ( device == 0 )
    {
      std::cerr << "ERROR: unable to create video device";
      perror(":");
      std::exit( EXIT_FAILURE );
    }
  else if ( device->getError() )
    {
      std::cerr << "ERROR: video device creation failed, error code = " << endl;
      perror(":");
      std::exit( EXIT_FAILURE );
    }
  
  if ( gettimeofday( & prev, 0  ) != 0 )
    {
      std::cerr << "videostream gettimeofday failed\n";
      perror( "videoStream gettimeofday ");
    }
}

void
VideoStream::run( )
{
  FrameBuffer * cameraFrame = 0;
  
  device->startCapture( );
  
  device->nextFrame( & cameraFrame );

  done = false;
  while( ! done )
    {
      device->releaseCurrentBuffer();
      device->nextFrame( & cameraFrame );
      sendImage( cameraFrame );
    }

  device->stopCapture();
}

void
VideoStream::run_trampoline( VideoStream * vs )
{
  vs->run();
}

VideoStream::~VideoStream()
{
  if ( device != 0 )
    {
      device->stopCapture();
      delete device;
    }
  // TODO Auto-generated destructor stub
}

int 
VideoStream::input_cmd(in_cmd_type cmd, int value)
{
  int res = 0;
  
  fprintf(stderr, "input_cmd:%d, %d", cmd, value);
  
  pthread_mutex_lock(&controls_mutex);
  
  switch(cmd) {
  default:
    res = -1;
  }
  
  pthread_mutex_unlock(&controls_mutex);
  
  return res;
}

int 
VideoStream::sendImage(FrameBuffer * img)
{
  if(HTTPD::ClientRequest == true)
    {
      std::cout << "Framebuffer type " << img->type() << std::endl;
      if(img->type() == FrameBuffer::RGB24BE)
	{
	  //	  memcpy(img->buffer, img->buffer, img->frameSize);
	  
	  pthread_mutex_lock(&global.db);
	  
	  if(img->bytesPerPixel == 3)
	    {
	      global.size = jpeg_utils::compress_fb_to_jpeg(img, global.buf, img->frameSize, 80);
	    }
	  else
	    {
	      std::cerr << "Unable to handle this pixel size" << std::endl;
	    }
	}
      else
	{
	  std::cerr << "Unable to deal with this image type" << std::endl;
	}
      pthread_cond_broadcast(&global.db_update);
      pthread_mutex_unlock(&global.db);
      HTTPD::ClientRequest = false;
    }
  else
    {
      pthread_mutex_lock(&global.db);
      pthread_cond_broadcast(&global.db_update);
      pthread_mutex_unlock(&global.db);
    }
  return 0;
}
  
void * 
VideoStream::server_thread(void * arg)
{
  HTTPD::server_thread(arg);
  return NULL;
}

