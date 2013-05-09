// $Id$
// Some basic image processing functions
// Jacky Baltes <jacky@cs.umanitoba.ca> Mon Nov 10 22:20:41 CST 2003

#ifndef _IMAGEPROCESSING_H_
#define _IMAGEPROCESSING_H_

#include <list>

#include "pixel.h"
#include "point.h"
#include "colourdefinition.h"
#include "rect.h"
#include "visionobject.h"

class FloodFillState;
class FrameBuffer;
class IntegralImage;

class ImageProcessing {  
 public:

  enum ErrorCode
  {
    NO_ERROR = 0,
    OUT_OF_BOUNDS,
    INVALID_SEED_POINT,
    STACK_OVERFLOW
  };

  static void segmentScanLines( FrameBuffer * frame, FrameBuffer * outFrame, unsigned int threshold, unsigned int minLength, unsigned int maxLength, unsigned int minSize, RawPixel const & mark, unsigned int subsample, ColourDefinition const * target );
  static void SegmentColours( FrameBuffer * frame, FrameBuffer * outFrame, unsigned int threshold, unsigned int minLength, unsigned int minSize, unsigned int subSample, ColourDefinition const & target, RawPixel const & mark, std::list<VisionObject> & results );

  static void medianFilter( FrameBuffer const * frame, FrameBuffer * outFrame, unsigned int subSample = 1 );

  static enum ErrorCode doFloodFill( FrameBuffer * frame,
				     FrameBuffer * outFrame,
				     Point p,
				     RawPixel seed, 
				     unsigned int threshold, 
				     ColourDefinition const * target,
				     unsigned int subSample,
				     FloodFillState * state );
  
  static void drawBresenhamLine( FrameBuffer * frame, Point const start, Point const end, RawPixel const colour );
  static void drawBresenhamLine( FrameBuffer * frame, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, RawPixel const colour );
  
  static void drawRectangle( FrameBuffer * frame, Rect const & rect, RawPixel const & colour );

  static void calcIntegralImage( FrameBuffer const * frame, IntegralImage * image );

  static void swapColours( FrameBuffer const * inFrame, FrameBuffer * outFrame, Rect bbox, unsigned int subSample, ColourDefinition const & find, RawPixel const & replace );

  static void calcQuadTreeDecomposition( FrameBuffer const * inFrame, FrameBuffer * outFrame, IntegralImage const * integralImage );

  static void convertBuffer( FrameBuffer const * frame, FrameBuffer * outFrame, unsigned int subSample = 1);

};


#endif
