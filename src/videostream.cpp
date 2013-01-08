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

#include "serial.h"

using namespace std;

struct Command const VideoStream::Commands[] = 
  {
    VideoStream::CommandProcessingMode, 
    VideoStream::CommandUpdateColour, 
    VideoStream::CommandVideoControl,
    VideoStream::CommandQueryColour,
    VideoStream::CommandQueryColourList,
    
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
			 unsigned int numBuffers,
			 unsigned int subSample
			 )
  : done( true ),
    mode( VideoStream::Raw ),
    subSample( subSample )
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

  device->startCapture( );
  
  device->nextFrame( & cameraFrame );

  if ( cameraFrame != 0 )
    {
      outFrame = new FrameBufferRGB24BE( );
      outFrame->initialize( cameraFrame->width, cameraFrame->height );
    }

  colours.clear();

  colours.push_back( ColourDefinition("red", 
				      Pixel(60,60,60,-50,60,128,0,0,0),
				      Pixel(255,255,128,0,180,180,255,255,255)) );
  colours.push_back( ColourDefinition( "green",
				       Pixel(0,100,0,-255,-255,-255,0,0,0),
				       Pixel(128,255,128,0,255,0,255,255,255) ) );
  colours.push_back( ColourDefinition( "blue",
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
	  std::list<VisionObject> results;
	  results.clear();

	  ImageProcessing::SegmentColours( frame, outFrame,
					   50,5,10,
					   subSample,
					   colours[i], 
					   marks[i],
					   results );
	  DBG("Segmentation found %d results", results.size() );
	  for( std::list<VisionObject>::iterator i = results.begin();
	       i != results.end();
	       ++i)
	    {
	      DBG("Found %s size %d at %d,%d\n", (*i).type.c_str(), (*i).size, (*i).x, (*i).y );
	      if ( server.conf.serial != 0 )
		{
		  std::ostringstream o;
		  VisionObject vo = (*i);
		  o << vo;
		  DBG("Serial output: %s\n", o.str().c_str() );
		  server.conf.serial->write(o.str().c_str() );
		}
	    }
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

int 
VideoStream::sendImage(FrameBuffer * img)
{
  if(HTTPD::ClientRequest == true)
    {

#ifdef DEBUG
      std::cout << "Framebuffer type " << img->type() << std::endl;
#endif
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

  // Make sure that these match the ProcessType enum above.
static char const * processTypeStrings[] = 
  {
    "raw",
    "showcolours",
    "segmentcolours",
    "scanlines",
    "segmentation"
  };

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
	  if ( ! strncmp("query",s,strlen("query") ) )
	    {
	      ret = COMMAND_ERR_OK;
	    }
	  else if ( ! strncmp("raw",s, strlen("raw") ) )
	    {
	      video->SetMode(Raw);
	      ret = COMMAND_ERR_OK;
	    }
	  else if ( ! strncmp("showcolours", s, strlen("showcolours") ) )
	    {
	      video->SetMode(ShowColours);
	      ret = COMMAND_ERR_OK;
	    }
	  else if ( ! strncmp("segmentcolours", s, strlen("segmentcolours") ) )
	    {
	      video->SetMode(SegmentColours);
	      ret = COMMAND_ERR_OK;
	    }
	  else
	    {
	      strncpy(response, "processingmode: ERR_PARAMETER", respLength - 1);
	      response[respLength - 1] = '\0';
	      ret = COMMAND_ERR_PARAMETER;
	    }
	}

      if ( ret == COMMAND_ERR_OK ) 
	{
	  unsigned int m = static_cast<unsigned int>( video->GetMode() );
	  char const * ms;

	  if ( ( m >= 0 ) && ( m < ( sizeof(processTypeStrings)/sizeof(processTypeStrings[0] ) ) ) )
	    {
	      ms = processTypeStrings[m];
	    }
	  snprintf(response, respLength - 1, "processingmode=%s", ms );
	  response[respLength-1] = '\0';
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

int
VideoStream::CommandQueryColour( VideoStream * video, char const * command, char * response, unsigned int respLength )
{
  int ret = COMMAND_ERR_COMMAND;
  char const * s;
  char const * start;
  size_t len;

  std::string name;

  response[0] = '\0';

  if ( ( s = strstr(command, "command=querycolour") ) != NULL )
    {
      s = s + strlen("command=querycolour");

      if ( ( s = strstr(s,"colour=") ) != NULL ) 
	{
	  s = s + strlen("colour=");
	  start = s;
	  len = 0;
	  while( ( *s != '\0') && ( *s != '&' ) )
	    {
	      s++;
	      len++;
	    }
	  
	  std::string name( start, len );

	  ColourDefinition * col = video->GetColour( name );
	  if ( col != 0 )
	    {
	      snprintf(response,respLength-1, "colour={%s&%s&%d&%d&%d&%d&%d&%d&%d&%d&%d&%d&%d&%d&%d&%d&%d&%d&%d&%d}",
		       col->name.c_str(),
		       col->name.c_str(),

		       col->min.red,
		       col->min.green,
		       col->min.blue,

		       col->max.red,
		       col->max.green,
		       col->max.blue,

		       col->min.red_green,
		       col->min.red_blue,
		       col->min.green_blue,

		       col->max.red_green,
		       col->max.red_blue,
		       col->max.green_blue,

		       col->min.red_ratio,
		       col->min.green_ratio,
		       col->min.blue_ratio,

		       col->max.red_ratio,
		       col->max.green_ratio,
		       col->max.blue_ratio
		       );
	      response[respLength-1] = '\0';
	      ret = COMMAND_ERR_OK;
	    }
	  else
	    {
	      strncpy(response,"colour UNKNOWN", respLength - 1 );
	      response[respLength-1] = '\0';
	      ret = COMMAND_ERR_PARAMETER;
	    }
	}
    }
  return ret;
}

int
VideoStream::CommandQueryColourList( VideoStream * video, char const * command, char * response, unsigned int respLength )
{
  int ret = COMMAND_ERR_COMMAND;
  char const * s;
  char const * start;
  size_t len;

  std::string name;

  response[0] = '\0';

  if ( ( s = strstr(command, "command=querycolourlist") ) != NULL )
    {
      s = s + strlen("command=querycolourlist");
      std::string list;

      for( vector<ColourDefinition>::iterator i = video->colours.begin();
	   i != video->colours.end();
	   ++i)
	{
	  if ( i != video->colours.begin() )
	    {
	      list = list + "&";
	    }
	  list = list + (*i).name;
	}

      snprintf(response,respLength-1, "colourlist={%s}", list.c_str() );
      response[respLength-1] = '\0';
      ret = COMMAND_ERR_OK;
    }
  else
    {
      strncpy(response,"colourlist UNKNOWN", respLength - 1 );
      response[respLength-1] = '\0';
      ret = COMMAND_ERR_PARAMETER;
    }
  return ret;
}

// Must match the enum VideoControl

char const * VideoControlStrings[] = 
  {
    "illegal control",
    "brightness",
    "hue",
    "saturation",
    "contrast",
    "sharpness",
    "gain"
  };


/* Assume all legal values are positive. -1 specifices the default value */
int
VideoStream::CommandVideoControl( VideoStream * video, char const * command, char * response, unsigned int respLength )
{
  int ret = COMMAND_ERR_COMMAND;
  char const * s;
  enum VideoControl vcontrol = IllegalControl;
  int val = -2;
  char const * control;

  response[0] = '\0';

  if ( ( s = strstr(command, "command=videocontrol") ) != NULL )
    {
      if ( ( s = strstr(s, "control=") ) != NULL ) 
	{
	  s = s + strlen("control=");
	  control = 0;
	  for(unsigned int i = 0; i < sizeof(VideoControlStrings)/sizeof(VideoControlStrings[0] ); ++i )
	    {
	      if ( ! strncmp(VideoControlStrings[i],s,strlen(VideoControlStrings[i]) ) )
		{
		  control = VideoControlStrings[i];
		  vcontrol = static_cast<enum VideoControl>( i );
		  s = s + strlen(control);
		  break;
		}
	    }
	  
	  if ( control != 0 )
	    {
	      if ( ( s = strstr(s, "value=") ) != NULL )
		{
		  s = s + strlen("value=");
		  if ( ! strncmp("query", s, strlen("query" ) ) )
		    {
		      val = -2;
		      ret = COMMAND_ERR_OK;
		    }
		  else if ( sscanf(s, "%d", & val ) != 1 )
		    {
		      ret = COMMAND_ERR_PARAMETER;
		      val = -2;
		    }
		}
	      
	      if ( val >= -1 ) 
		{
		  int err;
		  switch( vcontrol )
		    {
		    case Brightness:
		      err = video->device->SetBrightness( val );
		      if ( err >= 0 )
			{
			  ret = COMMAND_ERR_OK;
			}
		      break;
		    case Contrast:
		      err = video->device->SetContrast( val );
		      if ( err >= 0 )
			{
			  ret = COMMAND_ERR_OK;
			}
		      break;
		    case Saturation:
		      err = video->device->SetSaturation( val );
		      if ( err >= 0 )
			{
			  ret = COMMAND_ERR_OK;
			}
		      break;
		    case Sharpness:
		      err = video->device->SetSharpness( val );
		      if ( err >= 0 )
			{
			  ret = COMMAND_ERR_OK;
			}
		      break;
		    case Gain:
		      err = video->device->SetGain( val );
		      if ( err >= 0 )
			{
			  ret = COMMAND_ERR_OK;
			}
		      break;
		    default:
		      ret = COMMAND_ERR_COMMAND;
		      break;
		    }
		}
	      
	      if ( ret == COMMAND_ERR_OK )
		{
		  int v;

		  switch( vcontrol )
		    {
		    case Brightness:
		      v = video->device->GetBrightness( );
		      if ( v < 0 )
			{
			  ret = COMMAND_ERR_COMMAND;
			}
		      break;
		    case Contrast:
		      v = video->device->GetContrast( );
		      if ( v < 0 )
			{
			  ret = COMMAND_ERR_COMMAND;
			}
		      break;
		    case Saturation:
		      v = video->device->GetSaturation( );
		      if ( v < 0 )
			{
			  ret = COMMAND_ERR_COMMAND;
			}
		      break;
		    case Sharpness:
		      v = video->device->GetSharpness( );
		      if ( v < 0 )
			{
			  ret = COMMAND_ERR_COMMAND;
			}
		      break;
		    case Gain:
		      v = video->device->GetGain( );
		      if ( v < 0 )
			{
			  ret = COMMAND_ERR_COMMAND;
			}
		      break;
		    default:
		      ret = COMMAND_ERR_COMMAND;
		      break;
		    }

		  if ( v >= 0 )
		    {
		      snprintf(response, respLength - 1, "control=%s&value=%d", control, v );
		    }
		}
	    }
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

ColourDefinition *
VideoStream::GetColour( std::string const & name ) 
{
  ColourDefinition * col = 0;
  for( vector<ColourDefinition>::iterator i = colours.begin();
       i != colours.end();
       ++i)
    {
      if ( (*i).name == name )
	{
	  return & (*i);
	}
    }
  return col;
}

