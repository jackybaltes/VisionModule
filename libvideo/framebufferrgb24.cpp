
#include "framebufferrgb24.h"
#include "geometry.h"
#include <inttypes.h>
#include "pixel.h"
#include <assert.h>

FrameBufferRGB24::FrameBufferRGB24()
{
  _type = FrameBuffer::RGB24;
  bytesPerPixel = 3;
}

void
FrameBufferRGB24::getPixel( void * ptr, RawPixel * pixel ) const
{
#ifndef NDEBUG
  assert( ptr != 0 );
  assert( ptr >= buffer );
  assert( ptr < buffer + frameSize );
#endif
  uint8_t *p = static_cast<uint8_t *>( ptr );

  pixel->blue = *p++;
  pixel->green = *p++;
  pixel->red = *p++;
}

void
FrameBufferRGB24::setPixel( void * ptr, RawPixel const pixel )
{
  assert( ptr != 0 );
  assert( ptr >= buffer );
  assert( ptr < buffer + frameSize );
  uint8_t *p = static_cast<uint8_t *>( ptr );

  *p++ = pixel.blue;
  *p++ = pixel.green;
  *p++ = pixel.red;
}

