#ifndef _FLOODFILLSTATE_H_
#define _FLOODFILLSTATE_H_

#include "point.h"
#include "rect.h"
#include "pixel.h"

class FloodFillState
{
 public:
  FloodFillState();
  
  void addPoint( unsigned int const x, unsigned int const y, RawPixel const & pix );
  void addPoint( Point const p, RawPixel const & pix );
  
  Rect & bBox( void ) { return _bBox; };

  inline unsigned int size( void ) const { return _size; };
  inline unsigned int sumX( void ) const { return _sumX; };
  inline unsigned int sumY( void ) const { return _sumY; };

  inline void setSize( unsigned int size ) { _size = size; };
  inline void setSumX( unsigned int sumX ) { _sumX = sumX; };
  inline void setSumY( unsigned int sumY ) { _sumY = sumY; };

  RawPixel averageColour( void ) const;

  void initialize( void );
  
 private:        // Data
  
  unsigned int _size;
  unsigned int _sumX;
  unsigned int _sumY;

  Rect _bBox;
  
  unsigned int _avgRed;
  unsigned int _avgGreen;
  unsigned int _avgBlue;
};

#endif /* _FLOODFILLSTATE_H_ */
