#include <string>

#include "visionobject.h"
#include "pixel.h"
#include "rect.h"

VisionObject::VisionObject( std::string _type, unsigned int _size, unsigned int _x, unsigned int _y, RawPixel _average, Rect _bBox )
  : type(_type), 
    size(_size), 
    x(_x), 
    y(_y), 
    average(_average), 
    bBox(_bBox)
{
}

std::ostream &
operator<<(std::ostream & os, VisionObject const & vo )
{
  os << vo.type << ":" << vo.size << ":" << "(" << vo.x << "," << vo.y << ")" << vo.bBox;
  return os;
}
