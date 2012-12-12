
#ifndef _FRAME_BUFFER_RGB24_H_
#define _FRAME_BUFFER_RGB24_H_

#include "framebuffer.h"

class FrameBufferRGB24 : public FrameBuffer
{
 public:
  FrameBufferRGB24();
  virtual ~FrameBufferRGB24() { };
  virtual void getPixel( void * p, RawPixel * pixel ) const;
  virtual void setPixel( void * p, RawPixel const pixel );
};

#endif
