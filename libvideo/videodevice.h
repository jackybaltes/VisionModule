// $Id: videodevice.h,v 1.1.1.1 2004/09/16 23:12:05 cvs Exp $
// Superclass to encapsulate different video capturing devices.
// In particular V4L and V4L2 devices. FreeBSD maybe later
//

#ifndef _VIDEODEVICE_H_
#define _VIDEODEVICE_H_

#include <string>

// Forward declarations
class FrameBuffer;

class VideoDevice
{
public:
  VideoDevice( std::string driver, std::string devname, std::string input, std::string standard, unsigned int fps, unsigned int width, unsigned int height, unsigned int depth, unsigned int numBuffers );
  virtual ~VideoDevice( );

  virtual bool getError( void ) const;
  virtual int startCapture( void ) = 0;
  virtual int stopCapture( void ) = 0;
  virtual void nextFrame( FrameBuffer * * frame ) = 0;
  virtual int releaseCurrentBuffer( void ) = 0;
  virtual bool isInterlaced( void ) = 0;
  virtual bool isRunning( void ) const;
  
  virtual int GetBrightness( void );
  virtual int SetBrightness( unsigned int val );
  virtual int GetContrast( void );
  virtual int SetContrast( unsigned int val );
  virtual int GetSaturation( void );
  virtual int SetSaturation( unsigned int val );
  virtual int GetSharpness( void );
  virtual int SetSharpness( unsigned int val );
  virtual int GetGain( void );
  virtual int SetGain( unsigned int val );

  int GetWidth() const;
  int GetHeight() const;

 protected:
  std::string devname;

 public:
  std::string GetDeviceName( void ) const;

 protected:
  unsigned int depth;

 public:
  unsigned int GetDepth( void ) const;

 protected:
  std::string driver;
  std::string input;
  std::string standard;
  unsigned int fps;
  unsigned int width;
  unsigned int height;
  unsigned int numBuffers;

  bool error;
  bool running;
};

#endif
