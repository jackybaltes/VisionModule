#include "floodfillstate.h"
#include "point.h"
#include "geometry.h"

FloodFillState::FloodFillState( )
  : _bBox( Point( 65536, 65536 ), Point( 0, 0 ) )
{
  initialize();
}

void
FloodFillState::initialize( void )
{
  _sumX = 0;
  _sumY = 0;

  _size = 0;

  _avgRed = 0;
  _avgGreen = 0;
  _avgBlue = 0;

  bBox().topLeft() = Point(65536, 65536);
  bBox().bottomRight() = Point(0,0);
}

void
FloodFillState::addPoint( Point const p, RawPixel const & pix )
{
  addPoint( p.x(), p.y(), pix );
}

void
FloodFillState::addPoint( unsigned int const x, unsigned int const y, RawPixel const & pix )
{
  if ( x < bBox().topLeft().x() )
    {
      bBox().topLeft().setX( x );
    }

  if ( y < bBox().topLeft().y() )
    {
      bBox().topLeft().setY( y );
    }

  if ( x > bBox().bottomRight().x() )
    {
      bBox().bottomRight().setX( x );
    }

  if ( y > bBox().bottomRight().y() )
    {
      bBox().bottomRight().setY( y );
    }

  _avgRed = _avgRed + pix.red;
  _avgGreen = _avgGreen + pix.green;
  _avgBlue = _avgBlue + pix.blue;

  _size = _size + 1;
  _sumX = _sumX + x;
  _sumY = _sumY + y;
}

RawPixel
FloodFillState::averageColour( void ) const
{
  unsigned int red;
  unsigned int green;
  unsigned int blue;

  if ( _size > 0 )
    {
      red = _avgRed / _size;
      green = _avgGreen / _size;
      blue = _avgBlue / _size;
    }
  else
    {
      red = 0;
      green = 0;
      blue = 0;
    }

  red = Geometry::clamp( 0, red, 255 );
  green = Geometry::clamp( 0, green, 255 );
  blue = Geometry::clamp( 0, blue, 255 );

  return RawPixel( red, green, blue );
}
