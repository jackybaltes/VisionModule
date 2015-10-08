// $Id: videodevice.cpp,v 1.1.1.1 2004/09/16 23:12:05 cvs Exp $
// Jacky Baltes <jacky@cs.umanitoba.ca>

#include "videodevice.h"

VideoDevice::VideoDevice(std::string driver, std::string devname, std::string input, std::string standard, unsigned int fps, unsigned int width, unsigned int height, unsigned int depth, unsigned int numBuffers )
  :
  devname( devname ),
  depth( depth ),
  driver( driver ),
  input( input ),
  standard( standard ),
  fps( fps ),
  width( width ),
  height( height ),
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
VideoDevice::GetWidth( void ) const
{
  return width;
}

int 
VideoDevice::GetHeight( void ) const
{
  return height;
}

unsigned int 
VideoDevice::GetDepth( void ) const
{
  return depth;
}

bool 
VideoDevice::isRunning( void ) const
{
  return running;
}

int 
VideoDevice::GetBrightness( void )
{
  return -1;
}

int 
VideoDevice::SetBrightness( unsigned int val )
{
  return -1;
}

int 
VideoDevice::GetContrast( void )
{
  return -1;
}

int 
VideoDevice::SetContrast( unsigned int val )
{
  return -1;
}

int 
VideoDevice::GetSaturation( void )
{
  return -1;
}

int 
VideoDevice::SetSaturation( unsigned int val )
{
  return -1;
}

int 
VideoDevice::GetSharpness( void )
{
  return -1;
}

int 
VideoDevice::SetSharpness( unsigned int val )
{
  return -1;
}

int 
VideoDevice::GetGain( void )
{
  return -1;
}

int 
VideoDevice::SetGain( unsigned int val )
{
  return -1;
}

std::string
VideoDevice::GetDeviceName( void ) const
{
  return devname;
}
