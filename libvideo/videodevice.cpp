// $Id: videodevice.cpp,v 1.1.1.1 2004/09/16 23:12:05 cvs Exp $
// Jacky Baltes <jacky@cs.umanitoba.ca>

#include "videodevice.h"

VideoDevice::VideoDevice(std::string driver, std::string devname, std::string input, std::string standard, unsigned int fps, unsigned int width, unsigned int height, unsigned int depth, unsigned int numBuffers )
  :
  driver( driver ),
  devname( devname ),
  input( input ),
  standard( standard ),
  fps( fps ),
  width( width ),
  height( height ),
  depth( depth ),
  numBuffers( numBuffers ),
  error( true ),
  running( false )
{
}

VideoDevice::~VideoDevice()
{

}

bool 
VideoDevice::getError( void ) const
{
  return error;
}

int 
VideoDevice::getWidth( void ) const
{
  return width;
}

int 
VideoDevice::getHeight( void ) const
{
  return height;
}

bool 
VideoDevice::isRunning( void ) const
{
  return running;
}
