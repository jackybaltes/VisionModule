/*
 * videostream.cpp
 * 
 * Jacky Baltes <jacky@cs.umanitoba.ca> Thu Dec 13 01:09:15 CST 2012
 */

#include <iostream>
#include <sstream>
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
#include "jpeg_utils.h"
#include "httpd.h"

#include "../libvideo/framebuffer.h"
#include "../libvideo/framebufferrgb24.h"
#include "../libvideo/framebufferrgb24be.h"
#include "../libvideo/colourdefinition.h"
#include "../libvideo/pixel.h"
#include "../libvideo/imageprocessing.h"

#define WWW_FOLDER          "./www/"

using namespace std;

struct Command const VideoStream::Commands[] = 
  {
    VideoStream::CommandProcessingMode, 
    VideoStream::CommandUpdateColour, 
    
    { NULL }
  };

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
  : done( true ),
    mode( VideoStream::Raw )
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
  FrameBuffer * outFrame = 0;
  unsigned int subSample = 1;

  device->startCapture( );
  
  device->nextFrame( & cameraFrame );

  if ( cameraFrame != 0 )
    {
      outFrame = new FrameBufferRGB24BE( );
      outFrame->initialize( cameraFrame->width, cameraFrame->height );
    }

  colours.clear();

  colours.push_back( ColourDefinition("Red", 
				      Pixel(60,60,60,-50,60,128,0,0,0),
				      Pixel(255,255,128,0,180,180,255,255,255)) );
  colours.push_back( ColourDefinition( "Green",
				       Pixel(0,100,0,-255,-255,-255,0,0,0),
				       Pixel(128,255,128,0,255,0,255,255,255) ) );
  colours.push_back( ColourDefinition( "Blue",
				       Pixel(0,0,100,-255,-255,-255,0,0,0),
				       Pixel(255,255,255,255,0,0,255,255,255) ) );
  
  RawPixel marks[] = { RawPixel(255,0,0), RawPixel(0,255,255), RawPixel(0,0,255) };

  done = false;
  while( ! done )
    {
      device->nextFrame( & cameraFrame );
      if ( cameraFrame != 0 )
	{
	  ProcessFrame( mode, cameraFrame, outFrame, subSample, colours, marks );
	}
      device->releaseCurrentBuffer();
      sendImage( outFrame );
    }

  if ( outFrame != 0 )
    {
      delete outFrame;
    }
  device->stopCapture();
}

void
VideoStream::ProcessFrame( enum ProcessType ptype, 
			   FrameBuffer * frame, 
			   FrameBuffer * outFrame, 
			   unsigned int subSample, 
			   std::vector<ColourDefinition> colours,
			   RawPixel marks[]
			   )
{
  outFrame->fill( RawPixel( 0, 0, 0 ) );
  
  if ( (  ptype == Raw ) || ( ptype == ShowColours ) )
    {
      ImageProcessing::convertBuffer( frame, outFrame, subSample );
    }

  if ( ptype == ShowColours )
    {
      for( unsigned int i = 0; i < colours.size(); i++ )
	{
	  ImageProcessing::swapColours( outFrame, 0,
					Rect( Point(0,0), Point(outFrame->width, outFrame->height) ), 
					subSample,
					colours[i], 
					marks[i] );
	}
    }

  if ( ptype == SegmentColours )
    {
      for( unsigned int i = 0; i < colours.size(); i++ )
	{
	  ImageProcessing::SegmentColours( frame, outFrame,
					   50,5,10,
					   subSample,
					   colours[i], 
					   marks[i] );
	}
    }


  //  unsigned int threshold = mainWindow->sbThreshold->value();
  //unsigned int minimumLineLength = mainWindow->sbMinimumLineLength->value();
  //unsigned int maximumLineLength = mainWindow->sbMaximumLineLength->value();
  
  //unsigned int minimumSize = mainWindow->sbMinimumSize->value();
	      
  if ( ptype == Scanlines )
    {
      //      ImageProcessing::segmentScanLines( frame, outFrame, threshold, minimumLineLength, maximumLineLength, minimumSize, mark, subSample, 0 );
    }
  
  if ( ptype == Segmentation )
    {
      //ColourDefinition target;
      //    getColourDefinition( & target );
      
      //ImageProcessing::segmentScanLines( frame, outFrame, threshold, minimumLineLength, maximumLineLength, minimumSize, mark, subSample, & target );
    }
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

#if 0
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
#endif

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

int
VideoStream::CommandProcessingMode( VideoStream * video, char const * command, char * response, unsigned int respLength )
{
  int ret = COMMAND_ERR_COMMAND;
  char const * s;
  enum ProcessType processMode;

  response[0] = '\0';

  if ( ( s = strstr(command, "command=processingmode") ) != NULL )
    {
      if ( ( s = strstr(s, "mode=") ) != NULL ) 
	{
	  s = s + strlen("mode=");
	  if ( ! strncmp("raw",s, strlen("raw") ) )
	    {
	      video->SetMode(Raw);
	      
	      strncpy(response, "processingmode: raw", respLength - 1);
	      response[respLength-1] = '\0';

	      ret = COMMAND_ERR_OK;
	    }
	  else if ( ! strncmp("showcolours", s, strlen("showcolours") ) )
	    {
	      video->SetMode(ShowColours);

	      strncpy(response, "processingmode: showcolours", respLength - 1);
	      response[respLength-1] = '\0';

	      ret = COMMAND_ERR_OK;
	    }
	  else if ( ! strncmp("segmentcolours", s, strlen("segmentcolours") ) )
	    {
	      video->SetMode(SegmentColours);
	      
	      strncpy(response, "processingmode: segmentcolours", respLength - 1 );
	      response[respLength-1] = '\0';

	      ret = COMMAND_ERR_OK;
	    }
	  else
	    {
	      strncpy(response, "processingmode: ERR_PARAMETER", respLength - 1);
	      response[respLength - 1] = '\0';
	      ret = COMMAND_ERR_PARAMETER;
	    }
	}
    }
  return ret;
}

int
VideoStream::CommandUpdateColour( VideoStream * video, char const * command, char * response, unsigned int respLength )
{
  int ret = COMMAND_ERR_COMMAND;
  char const * s;
  enum ProcessType processMode;

  char name[256];
  unsigned int redmin;
  unsigned int greenmin;
  unsigned int bluemin;
  unsigned int redmax;
  unsigned int greenmax;
  unsigned int bluemax;
  int redgreenmin;
  int redbluemin;
  int greenbluemin;
  int redgreenmax;
  int redbluemax;
  int greenbluemax;
  unsigned int redratiomin;
  unsigned int greenratiomin;
  unsigned int blueratiomin;
  unsigned int redratiomax;
  unsigned int greenratiomax;
  unsigned int blueratiomax;
  
  response[0] = '\0';

  if ( ( s = strstr(command, "command=updatecolour") ) != NULL )
    {
      s = s + strlen("command=updatecolour");
      unsigned int conv;

      if ( ( conv = sscanf(s, "& { %[^&] & %d & %d & %d & %d & %d & %d & %d & %d & %d & %d & %d & %d & %d & %d & %d & %d & %d & %d }",
		  name, 
		  &redmin, &greenmin, & bluemin, 
		  &redmax, &greenmax, & bluemax, 
		  &redgreenmin, &redbluemin, & greenbluemin, 
		  &redgreenmax, &redbluemax, & greenbluemax, 
		  &redratiomin, &greenratiomin, & blueratiomin, 
		  &redratiomax, &greenratiomax, & blueratiomax)
	     ) == 19 )
	{
	  DBG("update colour %s\n", name);

	  ColourDefinition colour( name,
				   Pixel( redmin, greenmin, bluemin,
					  redgreenmin, redbluemin, greenbluemin,
					  redratiomin, greenratiomin, blueratiomin ),
				   Pixel( redmax, greenmax, bluemax,
					  redgreenmax, redbluemax, greenbluemax,
					  redratiomax, greenratiomax, blueratiomax ) 
				   );
	  video->UpdateColour( colour );

	  strncpy(response,"colour OK", respLength - 1 );
	  response[respLength-1] = '\0';
	  ret = COMMAND_ERR_OK;
	}
      else
	{
	  strncpy(response,"colour BAD", respLength - 1 );
	  response[respLength-1] = '\0';
	  ret = COMMAND_ERR_PARAMETER;
	}
    }
  return ret;
}

enum VideoStream::ProcessType
VideoStream::GetMode( void ) const
{
  return mode;
}

void
VideoStream::SetMode( enum ProcessType m )
{
  mode = m;
}

void
VideoStream::UpdateColour( ColourDefinition const col )
{
  for( vector<ColourDefinition>::iterator i = colours.begin();
       i != colours.end();
       ++i)
    {
      if ( (*i).name == col.name )
	{
	  (*i).min = col.min;
	  (*i).max = col.max;
	  break;
	}
    }
}
