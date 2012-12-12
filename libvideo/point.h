#ifndef _POINT_H_
#define _POINT_H_

#include <math.h>

class Point
{
 public:
  inline Point( unsigned int x = 0, unsigned int y = 0 ) { _x = x; _y = y; };

  inline unsigned int y( void ) const { return _y; };

  inline unsigned int x( void ) const { return _x; };
  
  inline void setX( unsigned int x ) { _x = x; };

  inline void setY( unsigned int y ) { _y = y; };

  inline double hypot( void ) const { return ::hypot( x(), y() ); }; 

 private:
  unsigned int _x;
  unsigned int _y;
};


#endif /* _POINT_H_ */
