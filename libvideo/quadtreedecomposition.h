#ifndef _QUAD_TREE_DECOMPOSITION_H_
#define _QUAD_TREE_DECOMPOSITION_H_

#include "rect.h"

class IntegralImage;
class FrameBuffer;

class QuadTreeDecomposition 
{
 public:
  static void calcQuadTreeDecomposition( Rect const & rect, IntegralImage const * image, FrameBuffer * outFrame );
};

#endif /* _QUAD_TREE_DECOMPOSITION_H_ */
