#include "pixel.h"
#include "framebuffer.h"

void
segmentScanLines( FrameBuffer * frame )
{
  FrameBufferIterator it( frame );
  RawPixel prev;
  RawPixel cPixel;

  for( unsigned int row = 0; row < frame->height; row++ )
    {
      it.goPosition( row, 0 );
      it.getRawPixel( & prev );
      it.goRight();
      for( unsigned int col = 0; col < frame->width; col++, it.goRight() )
	{
	  it.getPixel( & cPixel );
	  if ( cPixel.intensity() < 100 )
	    {
	      it.setPixel( RawPixel( 0, 255, 255 ) );
	    }
	}
    }
}
