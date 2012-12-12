// $Id: framebufferrgb32x.cpp,v 1.1.2.2 2004/10/26 04:53:49 cvs Exp $
// Jacky Baltes <jacky@cs.umanitoba.ca>
// Sun May 16 12:10:19 CDT 2004
//

#include "framebufferrgb32x.h"
#include "geometry.h"
#include <inttypes.h>
#include "pixel.h"
#include <assert.h>

#ifdef DEBUG
#   include <iostream>
#endif

void
FrameBufferRGB32X::getPixel( void * ptr, RawPixel * pixel ) const
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
FrameBufferRGB32X::setPixel( void * ptr, RawPixel const pixel )
{
  assert( ptr != 0 );
  assert( ptr >= buffer );
  assert( ptr < buffer + frameSize );
  uint8_t *p = static_cast<uint8_t *>( ptr );

  (*(p++)) = pixel.blue;
  (*(p++)) = pixel.green;
  (*(p++)) = pixel.red;
  (*p)   = (uint8_t)pixel.intIntensity();
#ifdef DEBUG
  if(pixel.blue == 255 && pixel.red == 0 && pixel.green == 0)
    {
      uint8_t *p = static_cast<uint8_t *>( ptr );  
      std::cout << "Pixel r,g,b,i " << (unsigned int)(*(p++)) << "," << (unsigned int)(*(p++)) << "," <<  (unsigned int)(*(p++)) << "," << (unsigned int)*p << std::endl;
    }  

#endif
}

#if 0
unsigned int 
FrameBufferRGB32X::getIntensity( void * ptr )
{
  uint32_t * p = static_cast<uint32_t *>(ptr);;
  return (*p) & 0x000000FF;
}

void 
FrameBufferRGB32X::mashPixel(void * ptr, const RawPixel pixel )
{
  // mash: an efficient average. of two pixels.

#ifdef DEBUG
  uint8_t * px = (uint8_t *) ptr;
  std::cout << "FrameBufferRGB32X::mashPixel\n";
  std::cout << "this pixel b,g,r,i:   " << (unsigned int) *(px++) << "," << (unsigned int) *(px++) << "," << (unsigned int) *(px++) << "," << (unsigned int) *(px) << "\n";
  std::cout << "passed pixel b,g,r,i: " << pixel.blue << "," << pixel.green << "," << pixel.red << "," << pixel.intIntensity() << "\n";
#endif
  uint32_t * pix, passedPixel;
  uint8_t * passedPtr = (uint8_t *)&passedPixel;
  
  // build the passed pixel
  // we shift the whole series right one and mask to handle oververflow and then
  // there is no need to shift back as it is an average.
  passedPixel = 0;
  assert( ptr != 0 );
  assert( ptr >= buffer );
  assert( ptr < buffer + frameSize );
  //  uint8_t *p = static_cast<uint8_t *>( ptr );
  pix = static_cast<uint32_t*>( ptr );

  if( (*pix) == 0 )
    {
      // if the pixel is zero, we simply set the pixel
      setPixel( ptr, pixel );
#ifdef DEBUG
      std::cout << "setting not mashing\n";
#endif
    }
  else
    {
      *passedPtr++ = pixel.blue;
      *passedPtr++ = pixel.green;
      *passedPtr++ = pixel.red;
      *passedPtr =   pixel.intIntensity();
      passedPixel = (passedPixel & 0xfefefeff) >> 1;
      *pix = ((*pix) & 0xfefefeff) >> 1;
      *pix = (*pix) + passedPixel;
    }

#ifdef DEBUG
  uint8_t * px = (uint8_t *) ptr;
  std::cout << "result     b,g,r,i:   " << (unsigned int) *(px++) << "," << (unsigned int) *(px++) << "," << (unsigned int) *(px++) << "," << (unsigned int) *(px) << "\n";
#endif
}

unsigned int 
FrameBufferRGB32X::getBlurred3x3Intensity(void * ptr) const
{
  //  std::cout << "FrameBufferRGB32X::getBlurred3x3Intensity\n";
  uint8_t * p = (uint8_t*)ptr; 
  p = p + 3;
  uint avg = 0;
  
  //  std::cout << "avg: " << avg << "\n";
  avg += *( p - bytesPerLine - bytesPerPixel);
  //  std::cout << "avg: " << avg << "\n";
  avg += *( p - bytesPerLine                );
  //  std::cout << "avg: " << avg << "\n";
  avg += *( p - bytesPerLine + bytesPerPixel);
  //  std::cout << "avg: " << avg << "\n";
  avg += *( p                - bytesPerPixel);
  //  std::cout << "avg: " << avg << "\n";
  avg += *( p                + bytesPerPixel);
  //  std::cout << "avg: " << avg << "\n";
  avg += *( p + bytesPerLine - bytesPerPixel);
  //  std::cout << "avg: " << avg << "\n";
  avg += *( p + bytesPerLine                );
  //  std::cout << "avg: " << avg << "\n";
  avg += *( p + bytesPerLine + bytesPerPixel);

  return avg / 9;


}
#endif
                   
