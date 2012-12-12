#include <iostream>

#include "integralimage.h"
#include "quadtreedecomposition.h"
#include "framebuffer.h"
#include "imageprocessing.h"

static
unsigned int abs( int x )
{
  int res = x;

  if ( x < 0 )
    {
      res = -x;
    }
  return res;
}

void
QuadTreeDecomposition::calcQuadTreeDecomposition( Rect const & rect, IntegralImage const * image, FrameBuffer * outFrame )
{
  Point tl = const_cast<Rect &>(rect).topLeft();
  Point br = const_cast<Rect &>(rect).bottomRight();

  unsigned int tlx = tl.x();
  unsigned int tly = tl.y();
  unsigned int brx = br.x();
  unsigned int bry = br.y();
  
  unsigned int cx = ( tlx + brx ) / 2;
  unsigned int cy = ( tly + bry ) / 2;

  Rect QTL( Point( tlx, tly ), Point( cx, cy ) );
  Rect QTR( Point( cx, tly ), Point( brx, cy ) );
  Rect QBL( Point( tlx, cy ), Point( cx, bry ) );
  Rect QBR( Point( cx, cy ), Point( brx, bry ) );
  
  unsigned int avgAll = image->avgColour( rect ) / rect.size();
  unsigned int avgQTL = image->avgColour( QTL ) / QTL.size();
  unsigned int avgQTR = image->avgColour( QTR ) / QTR.size();
  unsigned int avgQBL = image->avgColour( QBL ) / QBL.size();
  unsigned int avgQBR = image->avgColour( QBR ) / QBR.size();

#ifdef DEBUG
  std::cout << __PRETTY_FUNCTION__;
  std::cout << " avg All " << rect << "=" << avgAll;
  std::cout << " avg TL " << QTL << "=" << avgQTL;
  std::cout << " avg TR " << QTR << "=" << avgQTR;
  std::cout << " avg BL " << QBL << "=" << avgQBL;
  std::cout << " avg BR " << QBL << "=" << avgQBL;
  std::cout << std::endl;
#endif

  if ( ( abs( avgAll - avgQTL ) > 20 ) ||
       ( abs( avgAll - avgQTR ) > 20 ) ||
       ( abs( avgAll - avgQBL ) > 20 ) ||
       ( abs( avgAll - avgQBR ) > 20 )  )
    {
      if ( QTL.size() > 64 ) 
	{
	  calcQuadTreeDecomposition( QTL, image, outFrame );
	}      
      if ( QTR.size() > 64 ) 
	{
	  calcQuadTreeDecomposition( QTR, image, outFrame );
	}      
      if ( QBL.size() > 64 ) 
	{
	  calcQuadTreeDecomposition( QBL, image, outFrame );
	}      
      if ( QBR.size() > 64 ) 
	{
	  calcQuadTreeDecomposition( QBR, image, outFrame );
	}      
    }
  else
    {
      std::cout << "Found larger quadtree rect " << rect << std::endl;
      //      outFrame->fill( rect, Pixel( avgAll, avgAll, avgAll ) );
      ImageProcessing::drawRectangle( outFrame, rect, RawPixel( 255, 0, 0 ) );
    }
}

