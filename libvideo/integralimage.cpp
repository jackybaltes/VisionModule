#include <vector>

#include "pixel.h"
#include "framebuffer.h"
#include "imageprocessing.h"
#include "point.h"
#include "floodfillstate.h"
#include "integralimage.h"

IntegralImage::IntegralImage( unsigned int width, unsigned height )
  : width( width ),
    height( height )
{
  buffer = new uint64_t[width * height];
  if ( buffer == 0 )
    {
      std::cerr << __PRETTY_FUNCTION__ << " unable to allocate integral image buffer of size " << width * height << std::endl;
    }
}

IntegralImage::~IntegralImage( )
{
  if ( buffer != 0 )
    {
      delete[] buffer;
    }
}

void 
ImageProcessing::calcIntegralImage( FrameBuffer const * frame, IntegralImage * image )
{
  FrameBufferIterator it( frame );
  uint64_t * ip;
  RawPixel pixel;

  if ( ( frame->width > image->width ) || ( frame->height > image->height ) )
    {
      std::cerr << __PRETTY_FUNCTION__ << " image " << image->width << 'x' << image->height << " incompatible with frame " << frame->width << 'x' << frame->height << std::endl;
      return;
    }
  
   // Initialize first entry
  frame->getPixel( 0, 0, &pixel );
  image->buffer[0 * image->width + 0 ] = pixel.intIntensity();

  // First row
  it.goPosition( 0, 1 );
  ip = & image->buffer[ 0 * image->width + 1 ];
  for( unsigned int j = 1; j < frame->width; j++, it.goRight(), ip++ )
    {
      it.getPixel( & pixel );
      *ip = *(ip - 1) + pixel.intIntensity();
    }

  for( unsigned int i = 1; i < frame->height; i++ )
    {
      // Do the first column
      it.goPosition( i, 0 );
      it.getPixel( & pixel );

      ip = & image->buffer[ i * image->width + 0 ];
      *ip = *( ip - image->width ) + pixel.intIntensity();
      ip++;
      for( unsigned int j = 1; j < frame->width; j++, it.goRight(), ip++ )
	{
	  it.getPixel( &pixel );
	  *ip = *( ip - image->width ) - *(ip - image->width - 1 ) + *( ip - 1 ) + pixel.intIntensity();
	}
    }
}

void
IntegralImage::toFrame( FrameBuffer * frame )
{
  FrameBufferIterator it( frame );
  RawPixel pixel;
  uint64_t * ip;
  // Add 1 to avoid divide by 0 for an all black image
  uint64_t scale = buffer[ ( frame->height - 1 ) * this->width  + frame->width - 1 ] + 1;

#ifdef DEBUG
  std::cout << __PRETTY_FUNCTION__ << " with scale " << scale << std::endl;
#endif

  unsigned long brightness;

  for( unsigned int i = 0; i < frame->height; i++ )
    {
      it.goPosition( i, 0 );
      ip = & buffer[i * width + 0];
      for( unsigned int j = 0; j < frame->width; j++, it.goRight(), ip++ )
	{				
	  brightness = ( *ip * 256 ) / scale;
	  pixel.red = brightness;
	  pixel.green = brightness;
	  pixel.blue = brightness;

	  it.setPixel( pixel );
	}
    }
}

uint64_t
IntegralImage::avgColour( Rect const & rect ) const
{
  unsigned int tlx = const_cast<Rect &>(rect).topLeft().x();
  unsigned int tly = const_cast<Rect &>(rect).topLeft().y();
  unsigned int brx = const_cast<Rect &>(rect).bottomRight().x() - 1;
  unsigned int bry = const_cast<Rect &>(rect).bottomRight().y() - 1;

  return buffer[ width * bry + brx ] - buffer[ width * bry + tlx ] - buffer[ width * tly + brx ] + buffer[ width * tly + tlx ];
}
