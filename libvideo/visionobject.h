#ifndef __VISION_OBJECT_H__
#define __VISION_OBJECT_H__

#include <string>
#include <ostream>

#include "pixel.h"
#include "rect.h"

class VisionObject
{
 public:
  VisionObject( std::string name, unsigned int _size, unsigned int _x, unsigned int _y, RawPixel _average, Rect _bBox );
  
  std::string type;
  unsigned int size;
  unsigned int x;
  unsigned int y;

  RawPixel average;
  Rect bBox;

  friend std::ostream & operator<<(std::ostream & os, VisionObject const & vo );
};

#endif /* __VISION_OBJECT_H__ */
