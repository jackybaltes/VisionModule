// $Id: framebufferrgb32.cpp,v 1.1.1.1 2004/09/16 23:12:05 cvs Exp $
// Jacky Baltes <jacky@cs.umanitoba.ca>
// Sun May 16 12:10:19 CDT 2004
//

#include "framebufferrgb32.h"
#include "geometry.h"
#include <inttypes.h>
#include "pixel.h"
#include <assert.h>

FrameBufferRGB32::FrameBufferRGB32()
{
  _type = FrameBuffer::RGB32;
  bytesPerPixel = 4;
}

void
FrameBufferRGB32::getPixel( void * ptr, RawPixel * pixel ) const
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
FrameBufferRGB32::setPixel( void * ptr, RawPixel const pixel )
{
  assert( ptr != 0 );
  assert( ptr >= buffer );
  assert( ptr < buffer + frameSize );
  uint8_t *p = static_cast<uint8_t *>( ptr );

  *p++ = pixel.blue;
  *p++ = pixel.green;
  *p++ = pixel.red;
}

