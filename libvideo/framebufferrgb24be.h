
#ifndef _FRAME_BUFFER_RGB24BE_H_
#define _FRAME_BUFFER_RGB24BE_H_

#include "framebufferrgb24.h"

class FrameBufferRGB24BE : public FrameBufferRGB24
{
 public:
  FrameBufferRGB24BE();
  virtual inline ~FrameBufferRGB24BE() { };

  virtual void getPixel( void * p, RawPixel * pixel ) const;
  virtual void setPixel( void * p, RawPixel const pixel );

 public:
  virtual unsigned int ConvertToJpeg( uint8_t * buffer, unsigned int maxSize, unsigned int quality );

};

#endif /* _FRAME_BUFFER_RGB24BE_H_ */
