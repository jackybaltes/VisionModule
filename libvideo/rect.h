#ifndef _RECT_H_
#define _RECT_H_

#include <iostream>

#include "point.h"

class Rect
{
 public:
  Rect( Point const & topLeft, Point const & bottomRight );
  inline Point const & topLeft( void ) const { return _topLeft; };
  inline Point const & bottomRight( void ) const { return _bottomRight; };

  inline Point & topLeft( void ) { return _topLeft; };
  inline Point & bottomRight( void ) { return _bottomRight; };
  
  inline unsigned int width( void ) const { return bottomRight().x() - topLeft().x(); };
  
  inline unsigned int height( void ) const { return bottomRight().y() - topLeft().y(); };
  
  inline unsigned int size( void ) const {  return width() * height(); };

  friend std::ostream & operator<<( std::ostream & os, Rect const & p );

 private:
  Point _topLeft;
  Point _bottomRight;
};

#endif /* _RECT_H_ */
