#include <iostream>
#include <list>

#include "pixel.h"
#include "framebuffer.h"
#include "imageprocessing.h"
#include "point.h"
#include "floodfillstate.h"
#include "colourdefinition.h"
#include "integralimage.h"
#include "quadtreedecomposition.h"
#include "visionobject.h"

unsigned int const STACK_SIZE = ( 320 * 320 );

unsigned int const BRIGHT_PIXEL = 256;
unsigned int const DARK_PIXEL = 4;

enum ImageProcessing::ErrorCode
ImageProcessing::doFloodFill( FrameBuffer * frame,
			      FrameBuffer * outFrame,
			      Point p,
			      RawPixel seed, 
			      unsigned int threshold, 
			      ColourDefinition const * colour,
			      unsigned int subSample,
			      FloodFillState * state )
{
  Pixel pixel;
  Pixel outPixel;
  Pixel neighbour;
  unsigned int diff;
  //int ret;
  FrameBufferIterator iter( frame, 0, 0 );
  FrameBufferIterator outIter( outFrame, 0, 0 );
  Point stack[ STACK_SIZE ];
  Point c;
  enum ErrorCode err = NO_ERROR;

  // Use stackIndex = 0 as underflow check
  unsigned int stackIndex = 1;

#ifndef Q_NODEBUG
  //  std::cout << __PRETTY_FUNCTION__ << " called at (" << p.x() << ',' << p.y() << ")" << " with colour " << seed << std::endl;
#endif  

  //  state->initialize();
  
  // outFrame->fill( RawPixel(0,0,0) ); // Set to black.
  
  // If the seed is black, the flood fill will get stuck in an
  // infinite loop.  So, we skip floodfills on black seeds.
  
  if( ( seed.red == 0 ) && ( seed.green == 0 ) && ( seed.blue == 0) )
    {
      // Fudge the seed to be not all black.
      seed.blue = 1;
    }
        
  if( iter.goPosition( p.y(), p.x() ) || outIter.goPosition( p.y(), p.x() ) )
    {
      err = INVALID_SEED_POINT;
      goto exit;
    }
  
  // Push the initial point onto the stack

  stack[stackIndex++] = p;
  while( stackIndex > 1 )
    {
      if ( stackIndex >= STACK_SIZE - 4 )
	{
	  std::cerr << __PRETTY_FUNCTION__ << " ERROR: possible stack overflow " << stackIndex << std::endl;
	  err = STACK_OVERFLOW;
	  goto exit;
	}
      c = stack[--stackIndex];
#ifndef Q_NODEBUG
      //      std::cout << " processing point (" << c.x() << ',' << c.y() << ")" << std::endl;
#endif  

      if( iter.goPosition( c.y(), c.x() ) || outIter.goPosition( c.y(), c.x() ) )
	{
	  err = OUT_OF_BOUNDS;
	  goto exit;
	}
     
      iter.getPixel( & pixel, 0 );
      outIter.getPixel( & outPixel, 0 );
      
      //      if ( ! ImageProcessing::isChecked( outPixel ) )
      if ( ( outPixel.red == 0 ) && ( outPixel.green == 0 ) && ( outPixel.blue == 0 ) )
	{
	  if ( ( ( pixel.red < BRIGHT_PIXEL) && ( pixel.red > DARK_PIXEL ) ) && ( pixel.green > DARK_PIXEL ) && ( pixel.blue > DARK_PIXEL ) )
	    {
	      state->addPoint( c, pixel );
	      
	      outIter.setPixel( seed, 0 );
	      iter.setPixel( seed, 0 );
	      if ( c.x() >= subSample ) 
		{
		  iter.getPixel( &neighbour, -subSample * frame->bytesPerPixel );
		  diff = pixel.diffIntensity( neighbour );
		  if ( ( diff <= threshold ) && ( (colour == 0 ) || colour->isMatch( pixel ) ) )
		    {
		      stack[stackIndex++] = Point( c.x() - subSample, c.y() );
		    }
		}
				
	      if ( c.x() < (frame->width - 1 - subSample) ) 
		{
		  iter.getPixel( &neighbour, + subSample * frame->bytesPerPixel );
		  diff = pixel.diffIntensity( neighbour );
		  if ( ( diff <= threshold ) && ( (colour == 0 ) || colour->isMatch( pixel ) ) )
		    {
		      stack[stackIndex++] = Point( c.x() + subSample, c.y() );
		    }
		}
	      
	      if ( c.y() >= subSample ) 
		{
		  iter.getPixel( &neighbour, - subSample * frame->bytesPerLine );
		  diff = pixel.diffIntensity( neighbour );
		  if ( ( diff <= threshold ) && ( (colour == 0 ) || colour->isMatch( pixel ) ) )
		    {
		      stack[stackIndex++] = Point( c.x(), c.y() - subSample );
		    }
		}
				
	      if ( c.y() < (frame->height - 1 - subSample) ) 
		{
		  iter.getPixel( &neighbour, + subSample * frame->bytesPerLine );
		  diff = pixel.diffIntensity( neighbour );
		  if ( ( diff <= threshold ) && ( (colour == 0 ) || colour->isMatch( pixel ) ) )
		    {
		      stack[stackIndex++] = Point( c.x(), c.y() + subSample );
		    }
		}
	    }
	  else
	    {
	      // mark the pixel as checked
	      outIter.setPixel( seed, 0 );
	      iter.setPixel( seed, 0 );
	    }
	}
    }
  state->setSumX( state->sumX() * subSample * subSample );
  state->setSumY( state->sumY() * subSample * subSample );
  state->setSize( state->size() * subSample * subSample );
  
 exit:
  return err;
}

void
ImageProcessing::segmentScanLines( FrameBuffer * frame, 
				   FrameBuffer * outFrame, 
				   unsigned int threshold, 
				   unsigned int minLength,
                                   unsigned int maxLength,
				   unsigned int minSize,
                                   RawPixel const & mark,
				   unsigned int subSample,
				   ColourDefinition const * target )
{
  FrameBufferIterator it( frame );
  RawPixel pPixel;
  RawPixel cPixel;
  unsigned int startLine;
  int diff;
  unsigned int avgRed, avgGreen, avgBlue;
  unsigned int len;
  FloodFillState state;

  for( unsigned int row = 0; row < frame->height; row = row + subSample )
    {
      it.goPosition( row, 0 );

      it.getPixel( & pPixel ); // Extend to the left

      startLine = 0;

      avgRed = 0;
      avgGreen = 0;
      avgBlue = 0;
      
      it.goPosition(row, subSample);

      for( unsigned int col = subSample; col <= frame->width + subSample; col = col + subSample, it.goRight( subSample ) )
	{
	  if ( col < frame->width )
	    {
	      it.getPixel( & cPixel );
	      diff = cPixel.diffIntensity( pPixel );
	      if ( diff < 0 )
		{
		  diff = -diff;
		}
	    }

	  if ( ( static_cast<unsigned int>(diff) > threshold ) || ( col >= static_cast<unsigned int>( frame->width - 1 ) ) )
	    {
	      len = col - startLine;

              if ( ( len >= minLength ) && (len <= maxLength) )
		{
		  Pixel colour( (subSample * avgRed ) / len,(subSample * avgGreen ) / len,(subSample * avgBlue ) / len);
		  
		  if ( target == 0 )
		    {
		      FrameBufferIterator out( outFrame, row, startLine );
		      for( unsigned int i = startLine; i < col; i++, out.goRight() )
			{
			  out.setPixel( colour );
			}
		    }
		  else if (target->isMatch( colour ) ) 
		    {
		      state.initialize();
		      doFloodFill( frame, outFrame, Point( startLine, row), colour, threshold, target, 1, & state );
		      
#ifdef XX_DEBUG
		      if ( state.size() > minSize )
			{
			  std::cout << "Flood fill returns size " << state.size() << std::endl;
			}
#endif
		      if ( state.size() > minSize )
			{
			  unsigned int tlx = state.bBox().topLeft().x();
			  unsigned int tly = state.bBox().topLeft().y();
			  unsigned int brx = state.bBox().bottomRight().x();
			  unsigned int bry = state.bBox().bottomRight().y();
			  
			  drawBresenhamLine( outFrame, tlx, tly, tlx, bry, mark );
			  drawBresenhamLine( outFrame, tlx, bry, brx, bry, mark );
			  drawBresenhamLine( outFrame, brx, bry, brx, tly, mark );
			  drawBresenhamLine( outFrame, brx, tly, tlx, tly, mark );

			  drawBresenhamLine( frame, tlx, tly, tlx, bry, mark );
			  drawBresenhamLine( frame, tlx, bry, brx, bry, mark );
			  drawBresenhamLine( frame, brx, bry, brx, tly, mark );
			  drawBresenhamLine( frame, brx, tly, tlx, tly, mark );
			  //		      swapColours( outFrame, 0, state.bBox(), 1, ColourDefinition( Pixel(colour), Pixel(colour) ), state.averageColour() );
			}
		    }
		}

	      avgRed = 0;
	      avgGreen = 0;
	      avgBlue = 0;
	      
	      startLine = col;
	    }
	  else
	    {
	      avgRed = avgRed + cPixel.red;
	      avgGreen = avgGreen + cPixel.green;
	      avgBlue = avgBlue + cPixel.blue;
	    }
	  pPixel = cPixel;
	}
    }
}

void
ImageProcessing::SegmentColours( FrameBuffer * frame, 
				 FrameBuffer * outFrame, 
				 unsigned int threshold, 
				 unsigned int minLength,
				 unsigned int minSize,
				 unsigned int subSample,
				 ColourDefinition const & target,
				 RawPixel const & mark,
				 std::vector<VisionObject> & results
				 )
{
  FrameBufferIterator it( frame );
  FrameBufferIterator oit( outFrame );
  Pixel cPixel;
  RawPixel oPixel;
  //  unsigned int len;
  FloodFillState state;
  unsigned int count;
  
  for( unsigned int row = 0; row < frame->height; row = row + subSample )
    {
      it.goPosition(row, subSample);
      oit.goPosition(row, subSample);

      count = 0;
      for( unsigned int col = subSample; col < frame->width; col = col + subSample, it.goRight( subSample ),oit.goRight( subSample ) )
	{
	  oit.getPixel( & oPixel );
	  if ( oPixel == RawPixel( 0, 0, 0 ) )
	    {
	      it.getPixel( & cPixel );
	      
	      if ( target.isMatch( cPixel ) )
		{
		  count++;
		}
	      else
		{
		  count = 0;
		}
	      
	      if ( count >= minLength )
		{
		  state.initialize();
		  doFloodFill( frame, outFrame, Point( col, row), 
			       cPixel, threshold, & target, 
			       subSample, & state );
		  
#ifdef XX_DEBUG
		  if ( state.size() > minSize )
		    {
		      std::cout << "Flood fill returns size " << state.size() << std::endl;
		    }
#endif
		  if ( state.size() > minSize )
		    {
		      unsigned int tlx = state.bBox().topLeft().x();
		      unsigned int tly = state.bBox().topLeft().y();
		      unsigned int brx = state.bBox().bottomRight().x();
		      unsigned int bry = state.bBox().bottomRight().y();
		      
		      drawBresenhamLine( outFrame, tlx, tly, tlx, bry, mark );
		      drawBresenhamLine( outFrame, tlx, bry, brx, bry, mark );
		      drawBresenhamLine( outFrame, brx, bry, brx, tly, mark );
		      drawBresenhamLine( outFrame, brx, tly, tlx, tly, mark );

		      drawBresenhamLine( frame, tlx, tly, tlx, bry, mark );
		      drawBresenhamLine( frame, tlx, bry, brx, bry, mark );
		      drawBresenhamLine( frame, brx, bry, brx, tly, mark );
		      drawBresenhamLine( frame, brx, tly, tlx, tly, mark );
		      //		      swapColours( outFrame, 0, state.bBox(), 1, ColourDefinition( Pixel(colour), Pixel(colour) ), state.averageColour() );
		      VisionObject vo( target.name, state.size(), state.x(), state.y(), state.averageColour(), state.bBox() );

		      std::vector<VisionObject>::iterator i;

		      for( i = results.begin();
			   i != results.end();
			   ++i)
			{
			  if ( (*i).size < vo.size )
			    {
			      break;
			    }
			}
		      results.insert(i, vo );
		    }
		  count = 0;
		}
	    }
	  else
	    {
	      count = 0;
	    }
	}
    }
}

void
ImageProcessing::convertBuffer( FrameBuffer const * frame, 
				FrameBuffer * outFrame, 
				unsigned int subSample )
{
  FrameBufferIterator it( frame );
  FrameBufferIterator oit( outFrame );
  RawPixel pPixel;

  for( unsigned int row = 0; row < frame->height; row = row + subSample )
    {
      it.goPosition( row, 0 );
      oit.goPosition(row, 0 );

      for( unsigned int col = subSample; col < frame->width; col = col + subSample, it.goRight( subSample ), oit.goRight( subSample ) )
	{
	  it.getPixel( & pPixel );
	  oit.setPixel( pPixel );
	}
    }
}

void
ImageProcessing::medianFilter( FrameBuffer const * frame, FrameBuffer * outFrame, unsigned int subSample )
{
  int red, green, blue;
  RawPixel pixel;
  FrameBufferIterator iter( frame, 0, 0 );
  RawPixel setPixel;
  
  if (outFrame == 0 )
    {
      outFrame = const_cast<FrameBuffer *> ( frame );
    }
  
  FrameBufferIterator outIter( outFrame, 0, 0 );
  
  for( unsigned int i = 0; i < frame->height - 1; i += subSample )
    {
      iter.goPosition( i, 0 );
      outIter.goPosition( i, 0 );
      
      for( unsigned int j = 0; j < frame->width - 1; j += subSample, iter.goRight( subSample ), outIter.goRight( 1 ) )
	{
	  iter.getPixel( & pixel );
	  red = pixel.red;
	  green = pixel.green;
	  blue = pixel.blue;
          
	  iter.getPixel( & pixel, frame->bytesPerPixel );
	  red = red + pixel.red;
	  green = green + pixel.green;
	  blue = blue + pixel.blue;
          
	  iter.getPixel( & pixel, frame->bytesPerLine );
	  red = red + pixel.red;
	  green = green + pixel.green;
	  blue = blue + pixel.blue;
          
	  iter.getPixel( & pixel, frame->bytesPerLine + frame->bytesPerPixel );
	  red = red + pixel.red;
	  green = green + pixel.green;
	  blue = blue + pixel.blue;
          
	  setPixel = RawPixel( red/4, green/4, blue/4 );
	  outIter.setPixel( setPixel, 0 );
	}
    }
}

void
ImageProcessing::swapColours( FrameBuffer const * frame, FrameBuffer * outFrame, Rect bbox, unsigned int subSample, ColourDefinition const & find, RawPixel const & replace )
{
  Pixel pixel;
  unsigned int tlx = bbox.topLeft().x();
  unsigned int tly = bbox.topLeft().y();

  unsigned int brx = bbox.bottomRight().x();
  unsigned int bry = bbox.bottomRight().y();

  FrameBufferIterator iter( frame, 0, 0 );
  
  if (outFrame == 0 )
    {
      outFrame = const_cast<FrameBuffer *> ( frame );
    }
  
  FrameBufferIterator outIter( outFrame, 0, 0 );
  
  for( unsigned int i = tly; i < bry; i += subSample )
    {
      iter.goPosition( i, 0 );
      outIter.goPosition( i, 0 );
      
      for( unsigned int j = tlx; j < brx; j += subSample, iter.goRight( subSample ), outIter.goRight( subSample ) )
	{
	  iter.getPixel( & pixel );
	  if ( find.isMatch( pixel ) )
	    {
	      pixel = replace;
	    } 
	  outIter.setPixel( pixel );
	}
    }
}

void
ImageProcessing::drawBresenhamLine( FrameBuffer * frame, Point const start, Point const end, RawPixel const colour )
{
  drawBresenhamLine( frame, start.x(), start.y(), end.x(), end.y(), colour );
}

void
ImageProcessing::drawBresenhamLine( FrameBuffer * frame, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, RawPixel const colour )
{
  unsigned int x, y;
  int dx, dy;
  int incx, incy;
  int balance;
  
  if (x2 >= x1)
    {
      dx = x2 - x1;
      incx = 1;
    }
  else	
    {
      dx = x1 - x2;	
      incx = -1;
    }
  
  if (y2 >= y1)
    {
      dy = y2 - y1;
      incy = 1;
    }
  else
    {
      dy = y1 - y2;
      incy = -1;
    }
  
  x = x1;
  y = y1;
  
  if (dx >= dy)
    {
      dy <<= 1;
      balance = dy - dx;
      dx <<= 1;
      
      while (x != x2)
	{
	  frame->setPixel( y, x, colour );
	  if (balance >= 0)
	    {
	      y += incy;
	      balance -= dx;
	    }
	  balance += dy;
	  x += incx;
	} 
      frame->setPixel( y, x, colour );
    }
  else
    {
      dx <<= 1;
      balance = dx - dy;
      dy <<= 1;
      
      while (y != y2)
	{
	  frame->setPixel( y, x, colour );;
	  if (balance >= 0)
	    {
	      x += incx;
	      balance -= dy;
	    }
	  balance += dx;
	  y += incy;
	} 
      frame->setPixel( y, x, colour );
    }
}

void
ImageProcessing::drawRectangle( FrameBuffer * frame, Rect const & rect, RawPixel const & colour )
{
  Point tl = const_cast<Rect &>(rect).topLeft();
  Point br = const_cast<Rect &>(rect).bottomRight();

  unsigned int tlx = tl.x();
  unsigned int tly = tl.y();
  unsigned int brx = br.x();
  unsigned int bry = br.y();

  drawBresenhamLine( frame, tlx, tly, brx, tly, colour );
  drawBresenhamLine( frame, brx, tly, brx, bry, colour );
  drawBresenhamLine( frame, brx, bry, tlx, bry, colour );
  drawBresenhamLine( frame, tlx, bry, tlx, tly, colour );
}

void
ImageProcessing::calcQuadTreeDecomposition( FrameBuffer const * frame, FrameBuffer * outFrame, IntegralImage const * integralImage )
{
#ifdef DEBUG
  std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
  IntegralImage const * myIntegralImage;
  
  if ( integralImage == 0 )
    {
      IntegralImage * im = new IntegralImage( frame->width, frame->height );
      ImageProcessing::calcIntegralImage( frame, im );
      myIntegralImage = im;
    }
  else
    {
      myIntegralImage = integralImage;
    }
  Rect rect( Point(0,0), Point(frame->width, frame->height ) );

  QuadTreeDecomposition::calcQuadTreeDecomposition( rect, myIntegralImage, outFrame );

  if ( ( integralImage == 0 ) && ( myIntegralImage != 0 ) )
    {
      delete myIntegralImage;
    }
}
