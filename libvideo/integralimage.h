#ifndef _INTEGRAL_IMAGE_H_
#define _INTEGRAL_IMAGE_H_

#include "inttypes.h"
#include "rect.h"

class FrameBuffer;

class IntegralImage
{
 public:
  IntegralImage( unsigned int width, unsigned int height );
  ~IntegralImage();
  
  uint64_t avgColour( Rect const & rect ) const;

  void toFrame( FrameBuffer * frame );

  unsigned int width;
  unsigned int height;

  uint64_t * buffer;
};

#endif /* _INTEGRAL_IMAGE_H_ */
