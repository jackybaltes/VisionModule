
#include "framebufferrgb24be.h"
#include "geometry.h"
#include <inttypes.h>
#include "pixel.h"
#include <assert.h>
#include "jpeg_utils.h"

FrameBufferRGB24BE::FrameBufferRGB24BE()
{
  _type = FrameBuffer::RGB24BE;
  bytesPerPixel = 3;
}

void
FrameBufferRGB24BE::getPixel( void * ptr, RawPixel * pixel ) const
{
#ifndef NDEBUG
  assert( ptr != 0 );
  assert( ptr >= buffer );
  assert( ptr < buffer + frameSize );
#endif
  uint8_t *p = static_cast<uint8_t *>( ptr );

  pixel->red = *p++;
  pixel->green = *p++;
  pixel->blue = *p++;
}

void
FrameBufferRGB24BE::setPixel( void * ptr, RawPixel const pixel )
{
  assert( ptr != 0 );
  assert( ptr >= buffer );
  assert( ptr < buffer + frameSize );
  uint8_t *p = static_cast<uint8_t *>( ptr );

  *p++ = pixel.red;
  *p++ = pixel.green;
  *p++ = pixel.blue;
}

unsigned int
FrameBufferRGB24BE::ConvertToJpeg( uint8_t * buffer, unsigned int maxSize, unsigned int quality )
{
  unsigned int size = jpeg_utils::compress_fb_to_jpeg(this, buffer, frameSize, quality );
  return size;
}
