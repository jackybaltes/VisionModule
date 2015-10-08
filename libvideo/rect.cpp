#include "rect.h"

Rect::Rect( Point const & tl, Point const & br )
{
  topLeft() = tl;
  bottomRight() = br;
}

std::ostream & 
operator<<( std::ostream & os, Rect const & rect )
{
  return os << '[' << rect.topLeft().x() << ',' << rect.topLeft().y() << "-" << rect.bottomRight().x() << ',' << rect.bottomRight().y() << ']';
}

