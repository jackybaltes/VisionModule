#ifndef __FRAME_BUFFER_RGB24_BE_BAYER_H__
#define __FRAME_BUFFER_RGB24_BE_BAYER_H__

#include "framebuffer.h"

class FrameBufferBayer : public FrameBuffer
{
  private:
  uint8_t * red;
  uint8_t * green;
  uint8_t * blue;
  unsigned int * frameSeen;
  bool initialized;
 public:
  FrameBufferBayer();
  virtual ~FrameBufferBayer();

  virtual void initialize( unsigned int width, unsigned int height, uint8_t * buffer = 0 );
  virtual void getPixel( void * ptr, RawPixel * pixel ) const;
  virtual void setPixel( void * ptr, RawPixel const pixel );
};

#endif
