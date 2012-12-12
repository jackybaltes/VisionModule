// $Id: framebufferrgb565.cpp,v 1.1.1.1.2.2 2004/10/05 19:38:39 cvs Exp $
// Jacky Baltes <jacky@cs.umanitoba.ca>
// Sun May 16 12:10:19 CDT 2004
//

#include "framebufferrgb565.h"
#include "geometry.h"
#include <inttypes.h>
#include "pixel.h"
#include <assert.h>

#ifdef DEBUG
#include <iostream>
#endif

FrameBufferRGB565::FrameBufferRGB565()
{
  _type = FrameBuffer::RGB565;
  bytesPerPixel = 2;
}

void
FrameBufferRGB565::getPixel( void * ptr, RawPixel * pixel ) const
{


#ifdef DEBUG
  if ( ( ptr < buffer ) || ( ptr >= buffer + frameSize ) || ( ptr == 0 ) )
    { 
      std::cerr << "FrameBufferRGB565::getPixel ptr " << ptr << ", buffer " << ( void * )buffer << ", framezize " << frameSize << " buffer end " << (void *)( buffer + frameSize ) << std::endl; 
    }
#endif
  assert( ptr != 0 );
  assert( ptr >= buffer );
  assert( ptr < buffer + frameSize );

  uint16_t * p = static_cast< uint16_t * >( ptr );

  pixel->blue = ( *p & 0x001f ) << 3;
  pixel->green = ( *p & 0x07c0 ) >> 3;
  pixel->red = ( *p & 0xf800 ) >> 8;
}

/* Changed to 5x5 blurring, change names later */
void
FrameBufferRGB565::getBlurred3x3Pixel( void * ptr, RawPixel * pixel ) const
{
  uint8_t * p0 = static_cast< uint8_t * >( ptr );
  int const offsets[ ]  = 
    { 
      -     bytesPerLine -     bytesPerPixel, 
      -     bytesPerLine                    ,
      -     bytesPerLine +     bytesPerPixel, 

                         -     bytesPerPixel, 
      /* Point itself is taken care of below */
                         +     bytesPerPixel, 

      +     bytesPerLine -     bytesPerPixel, 
      +     bytesPerLine                      ,
      +     bytesPerLine +     bytesPerPixel, 

    };

  uint32_t val = static_cast<uint32_t>( * ( static_cast< uint16_t *>( ptr ) ) );
  for( unsigned int i = 0; i < sizeof( offsets ) / sizeof( offsets[ 0 ] ); i++ )
    {
      uint8_t *p = p0 + offsets[ i ];
#ifdef DEBUG
      if ( ( p == 0 ) || ( p < buffer ) || ( p >= buffer + frameSize ) )
	{
	  std::cerr << "FrameBufferRGB565::getBlurred3x3Pixel ptr " << ptr << "p " << p << ", buffer " << (void *) buffer << ", frameSize " << frameSize << std::endl;
	}
#endif
      assert( p != 0 );
      assert( p >= buffer );
      assert( p < buffer + frameSize );

      val = ( val & 0xf7df ) + ( static_cast<uint32_t>( * ( (uint16_t *)( p ) ) ) & 0xf7df );
      val = ( val >> 1 );
    }
  
  pixel->blue = ( val & 0x001f ) << 3;
  pixel->green = ( val & 0x07c0 ) >> 3;
  pixel->red = ( val & 0xf800 ) >> 8;
}

#if 0
/* Changed to 5x5 blurring, change names later */
void
FrameBufferRGB565::getBlurred5x5Pixel( void * ptr, RawPixel * pixel ) const
{
  uint8_t * p0 = static_cast< uint8_t * >( ptr );
  int const offsets[24]  = 
    { 
      - 2 * bytesPerLine - 2 * bytesPerPixel /* Top Left  */, 
      - 2 * bytesPerLine -     bytesPerPixel /* Top       */, 
      - 2 * bytesPerLine                     ,
      - 2 * bytesPerLine +     bytesPerPixel /* Top Right */, 
      - 2 * bytesPerLine + 2 * bytesPerPixel,

      -     bytesPerLine - 2 * bytesPerPixel /* Top Left  */, 
      -     bytesPerLine -     bytesPerPixel /* Top       */, 
      -     bytesPerLine                                    ,
      -     bytesPerLine +     bytesPerPixel /* Top Right */, 
      -     bytesPerLine + 2 * bytesPerPixel /* Top Right */, 

                         - 2 * bytesPerPixel /* Top Left  */, 
                         -     bytesPerPixel /* Top       */, 

                         +     bytesPerPixel /* Top Right */, 
                         + 2 * bytesPerPixel /* Top Right */, 

      +     bytesPerLine - 2 * bytesPerPixel /* Top Left  */, 
      +     bytesPerLine -     bytesPerPixel /* Top       */, 
      +     bytesPerLine                                    ,
      +     bytesPerLine +     bytesPerPixel /* Top Right */, 
      +     bytesPerLine + 2 * bytesPerPixel /* Top Right */, 

      + 2 * bytesPerLine - 2 * bytesPerPixel /* Top Left  */, 
      + 2 * bytesPerLine -     bytesPerPixel /* Top       */, 
      + 2 * bytesPerLine                                    ,
      + 2 * bytesPerLine +     bytesPerPixel /* Top Right */, 
      + 2 * bytesPerLine + 2 * bytesPerPixel /* Top Right */, 
    };

  uint32_t val = static_cast<uint32_t>( * ( static_cast< uint16_t *>( ptr ) ) );
  for( int i = 0; i < 24; i++ )
    {
      val = ( val & 0xf7df ) + ( static_cast<uint32_t>( * ( (uint16_t *)( p0 + offsets[ i ] ) ) ) & 0xf7df );
      val = ( val >> 1 );
    }
  
  pixel->blue = ( val & 0x001f ) << 3;
  pixel->green = ( val & 0x07c0 ) >> 3;
  pixel->red = ( val & 0xf800 ) >> 8;
}
#endif

void
FrameBufferRGB565::setPixel( void * ptr, RawPixel const pixel )
{
  uint16_t * p = static_cast< uint16_t * >( ptr );

  *p = ( ( pixel.red & 0xf8 ) << 8 ) | ( ( pixel.green & 0xfc ) << 2 ) | ( ( pixel.blue & 0xf8 ) >> 3 );
}

