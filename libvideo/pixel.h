// $Id: pixel.h,v 1.1.1.1.2.5 2005/01/23 22:58:51 cvs Exp $
//

#ifndef _PIXEL_H_
#define _PIXEL_H_

#include <inttypes.h>
#include <iostream>

class RawPixel
{
 public:
  RawPixel( int r = 0, int g = 0, int b = 0 );
  RawPixel( double hue );
  RawPixel( const RawPixel & p);
  double hypot( RawPixel const & p );
  double intensity( void ) const;
  unsigned int intIntensity( void ) const;
  unsigned int max( void ) const;
  void mashPixel(const RawPixel & other);
  
  unsigned int componentSquare( RawPixel const & p ) const;
  double diffIntensity( RawPixel const & p ) const;
  double diffMax( RawPixel const & p ) const;
  void adjustContrast( RawPixel min, RawPixel max );
  int getMin( void );
  int getMax( void );
  double hue( void );
  void polarCoordinates( double * alpha, double * beta, double * radius );
  void sct( double * alpha, double * beta, double * radius );
  void setMinimum( RawPixel const & p );
  void setMaximum( RawPixel const & p );
  unsigned int sumSquaredError (RawPixel const & p) const;
  void medianShift(const RawPixel & p);

  RawPixel & operator+=( RawPixel const & p );
  friend RawPixel const operator+( RawPixel const & p1, RawPixel const & p2 );
  friend RawPixel const operator/( RawPixel const & p, double const n );
  friend RawPixel const operator*( RawPixel const & p, double const n );
  friend std::ostream & operator<<( std::ostream & os, RawPixel & p );
  operator uint32_t( void );

  unsigned int red;
  unsigned int green;
  unsigned int blue;
};

class Pixel : public RawPixel
{
 public:

  Pixel( int r = 0, int g = 0, int b = 0 );
  Pixel( int r, int g, int b, int rg, int rb, int gb, int rr, int gr, int br );
  Pixel( const Pixel & rhs);
  void update( void );
  unsigned int componentSquare( Pixel const & p );
  unsigned int componentSquareValue( Pixel const & p );
  unsigned int componentSquareDiff( Pixel const & p );
  unsigned int componentSquareRatio( Pixel const & p );
  void setMinimum( Pixel const & p );
  void setMaximum( Pixel const & p );

  Pixel & operator=( RawPixel const & p );
  Pixel & operator+=( Pixel const & p );

  friend Pixel const operator+( Pixel const & p1, Pixel const & p2 );
  friend Pixel const operator/( Pixel const & p, double const n );
  friend Pixel const operator*( Pixel const & p, double const n );

  friend std::ostream & operator<<(std::ostream & os, Pixel const & pixel);

  friend std::istream & operator>>( std::istream & is, Pixel & pixel );

  int red_green;
  int red_blue;
  int green_blue;

  unsigned int red_ratio;
  unsigned int green_ratio;
  unsigned int blue_ratio;
};

#endif
