#ifndef _FRAME_BUFFER_RGB32_X_H_
#define _FRAME_BUFFER_RGB32_X_H_

#include "framebuffer.h"

class FrameBufferRGB32X : public FrameBuffer
{
 public:
  void getPixel( void * p, RawPixel * pixel ) const;
  void setPixel( void * p, RawPixel const pixel );
//  unsigned int getIntensity (void * p);
//  inline unsigned int getIntensity(unsigned int row, unsigned int col){return FrameBuffer::getIntensity(row,col);}
//  void mashPixel(void * ptr, const RawPixel pixel );
//  unsigned int getBlurred3x3Intensity(void * ptr) const;
};

#endif
