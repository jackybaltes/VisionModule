// $Id: pixel.cpp,v 1.1.1.1.2.6 2005/01/26 01:21:55 cvs Exp $
//

#include "pixel.h"
#include "math.h"
#include <inttypes.h>

RawPixel::RawPixel( int r, int g, int b ) : red( r ), green( g ), blue ( b )
{

}

#if 0
RawPixel::RawPixel( double alpha, double beta, double radius )
{
  red = static_cast<int>(cos( alpha + 45.0 / 180.0 * M_PI ) * radius );
  green = static_cast<int>(sin( alpha + 45.0 / 180.0 * M_PI ) * radius );
  blue = static_cast<int>(cos( beta + 45.0 / 180.0 * M_PI ) * radius );
}
#endif

unsigned int RawPixel::sumSquaredError (RawPixel const & p) const
{
  int errR, errG, errB;
  errR = red - p.red;
  errR *= errR;
  errG = green - p.green;
  errG *= errG;
  errB = blue - p.blue;
  errB *= errB;
  return errR + errG + errB;
}

void RawPixel::medianShift(const RawPixel & p)
{
  if(p.red > red)
    red++;
  else if(p.red < red)
    red--;

  if(p.blue > blue)
    blue++;
  else if(p.blue < blue)
    blue--;

  if(p.green > green)
    green++;
  else if(p.green < green)
    green--;
  
}

RawPixel::RawPixel( double hue )
{
  double v = 1.0;
  double s = 1.0;

  hue = hue / 60.0;			// sector 0 to 5

  int i = static_cast<int>( floor( hue ) );
  double f = hue - i;			// factorial part of h
  double p = v * ( 1 - s );
  double q = v * ( 1 - s * f );
  double t = v * ( 1 - s * ( 1 - f ) );
  double r, g, b;

  switch( i ) 
    {
    case 0:
      r = v;
      g = t;
      b = p;
      break;
    case 1:
      r = q;
      g = v;
      b = p;
      break;
    case 2:
      r = p;
      g = v;
      b = t;
      break;
    case 3:
      r = p;
      g = q;
      b = v;
      break;
    case 4:
      r = t;
      g = p;
      b = v;
      break;
    default:		// case 5:
      r = v;
      g = p;
      b = q;
      break;
    }
  red = static_cast<int>( rint( r * 255.0 ) );
  green = static_cast<int>( rint( g * 255.0 ) );
  blue = static_cast<int>( rint( b * 255.0 ) );
}

double
RawPixel::hypot( RawPixel const & p )
{
  return sqrt( componentSquare( p ) );
}

unsigned int
RawPixel::componentSquare( RawPixel const & p ) const
{
  int rdiff = this->red - p.red;
  int gdiff = this->green - p.green;
  int bdiff = this->blue - p.blue;

  return rdiff * rdiff + gdiff * gdiff + bdiff * bdiff;
}

double
RawPixel::diffIntensity( RawPixel const & p ) const
{
  double i1 = intensity( );
  double i2 = p.intensity( );
  double diff =  i1 - i2;
  return diff * diff;
}

double
RawPixel::intensity( void ) const
{
  // return 0.299 * red + 0.587 * green + 0.114 * blue;
  return ( red + green + blue ) / 3.0;
}

unsigned int
RawPixel::intIntensity( void ) const
{
  return ( red + green + blue ) / 3;
}

void 
RawPixel::mashPixel(const RawPixel & other)
{
  if(red+blue+green == 0)
    {
      red = other.red;
      blue = other.blue;
      green = other.green;
    }
  else
    {
      red = (other.red + red) >> 1;
      blue = (other.blue + blue) >> 1;
      green = (other.green + green) >> 1;
    }
}

double
RawPixel::diffMax( RawPixel const & p ) const
{
  int m1 = max( );
  int m2 = p.max( );
  double diff = m1 - m2;
  return diff * diff;
}

unsigned int
RawPixel::max( void ) const
{
  unsigned int max = red;

  if ( green > max )
    max = green;
  if ( blue > max )
    max = blue;
  return max;
}

RawPixel const
operator+( RawPixel const & p1, RawPixel const & p2 )
{
  return RawPixel( p1.red + p2.red, p1.green + p2.green, p1.blue + p2.blue );
}

RawPixel &
RawPixel::operator+=( RawPixel const & p1 )
{
  red = red + p1.red;
  green = green + p1.green;
  blue = blue + p1.blue;
  return *this;
}

void
RawPixel::adjustContrast( RawPixel min, RawPixel max )
{
  red = ( red * 255 ) / ( max.red - min.red + 1 );
  green = ( green * 255 ) / ( max.green - min.green + 1 );
  red = ( blue * 255 ) / ( max.blue - min.blue + 1 );
}

RawPixel const
operator/
( RawPixel const & p, double const n )
{
  return RawPixel( static_cast<int>( p.red / n ), static_cast<int>( p.green / n ), static_cast<int>( p.blue / n ) );
}

RawPixel const  
operator*( RawPixel const & p, double const n )
{
  return RawPixel( static_cast<int>(p.red * n), static_cast<int>(p.green * n), static_cast<int>(p.blue * n ) );
}

RawPixel::operator uint32_t( void )
{
  return ( ( red & 0xff ) << 16 ) | ( ( green & 0xff ) << 8 ) | ( blue & 0xff );
}

double
RawPixel::hue( )
{
  double hue = 0.0;

  unsigned int mmin = getMin( );
  unsigned int mmax = getMax( );

  int delta = mmax - mmin;
  
  if ( delta != 0 )
    {
      if( red == mmax )
	{
	  hue = ( green - blue ) / delta;	
	}
      else if( green == mmax )
	{
	  hue = 2 + ( blue - red ) / delta;	// between cyan & yellow
	}
      else
	{
	  hue = 4 + ( red - green ) / delta;	// between magenta & cyan
	}
    }
  hue = hue * 60;
  if ( hue < 0 )
    {
      hue = hue + 360.0;
    }
  return hue;
}

int
RawPixel::getMax()
{
  unsigned int max;
  max = red;
  if ( green > max )
    {
      max = green;
    }
  if ( blue > max )
    {
      max = blue;
    }
  return max;
}

int
RawPixel::getMin()
{
  unsigned int min;
  min = red;
  if ( green < min )
    {
      min = green;
    }
  if ( blue < min )
    {
      min = blue;
    }
  return min;
}

void
RawPixel::polarCoordinates( double * alpha, double * beta, double * radius )
{
  *alpha = atan2( green, red );
  *beta = atan2( green, blue );
  *radius = sqrt( red * red + green * green + blue * blue );
}

void
RawPixel::sct( double * alpha, double * beta, double * radius )
{
  polarCoordinates( alpha, beta, radius );
  *alpha = * alpha - 45.0/180.0 * M_PI;
  *beta = * beta - 45.0/180.0 * M_PI;;
  *radius = sqrt( red * red + green * green + blue * blue );
}

void
RawPixel::setMinimum( RawPixel const & p )
{
  if ( p.red < red )
    {
      red = p.red;
    }
  if ( p.green < green )
    {
      green = p.green;
    }
  if ( p.blue < blue )
    {
      blue = p.blue;
    }
}

void
RawPixel::setMaximum( RawPixel const & p )
{
  if ( p.red > red )
    {
      red = p.red;
    }
  if ( p.green > green )
    {
      green = p.green;
    }
  if ( p.blue > blue )
    {
      blue = p.blue;
    }
}

Pixel::Pixel( int r, int g, int b ) : RawPixel( r, g, b )
{
  update( );
}

Pixel::Pixel( int r, int g, int b, int rg, int rb, int gb, int rr, int gr, int br ) : RawPixel( r, g, b ), red_green( rg ), red_blue( rb ), green_blue( gb ), red_ratio( rr ), green_ratio( gr ), blue_ratio( br )
{
}

void Pixel::update( void )
{
  red_green = red - green;
  red_blue = red - blue;
  green_blue = green - blue;

  int sum = 1 + red + green + blue;

  if ( sum < 6 )
    {
      red_ratio = 0;
      green_ratio = 0;
      blue_ratio = 0;
    }
  else if ( sum < 672 )
    {
      red_ratio = ( red * 255 ) / sum;
      green_ratio = ( green * 255 ) / sum;
      blue_ratio = ( blue * 255 ) / sum;
    }
  else
    {
      red_ratio = 255;
      green_ratio = 255;
      blue_ratio = 255;
    }
}

Pixel const
operator+( Pixel const & p1, Pixel const & p2 )
{
  return Pixel( p1.red + p2.red, p1.green + p2.green, p1.blue + p2.blue, p1.red_green + p2.red_green, p1.red_blue + p2.red_blue, p1.green_blue + p2.green_blue, p1.red_ratio + p2.red_ratio, p1.green_ratio + p2.green_ratio, p1.blue_ratio + p2.blue_ratio );
}

Pixel &
Pixel::operator+=( Pixel const & p1 )
{
  red = red + p1.red;
  green = green + p1.green;
  blue = blue + p1.blue;
  
  red_green = red_green + p1.red_green;
  red_blue = red_blue + p1.red_blue;
  green_blue = green_blue + p1.green_blue;

  red_ratio = red_ratio + p1.red_ratio;
  green_ratio = green_ratio + p1.green_ratio;
  blue_ratio = blue_ratio + p1.blue_ratio;

  return *this;
}

Pixel const 
operator/( Pixel const & p, double const n )
{
  return Pixel( static_cast<int>(p.red / n), static_cast<int>(p.green / n), static_cast<int>(p.blue / n) );
}

Pixel const 
operator*( Pixel const & p, double const n )
{
  return Pixel( static_cast<int>(p.red * n), static_cast<int>(p.green * n), static_cast<int>(p.blue * n) );
}


unsigned int
Pixel::componentSquare( Pixel const & p )
{
  return componentSquareValue( p ) + componentSquareDiff( p ) + componentSquareRatio( p );
}

unsigned int
Pixel::componentSquareValue( Pixel const & p )
{
  return RawPixel::componentSquare( (RawPixel const & ) p );
}

unsigned int
Pixel::componentSquareDiff( Pixel const & p )
{
  int rg_diff = this->red_green - p.red_green;
  int rb_diff = this->red_blue - p.red_blue;
  int gb_diff = this->green_blue - p.green_blue;
  
  return rg_diff * rg_diff + rb_diff * rb_diff + gb_diff * gb_diff;
}

unsigned int
Pixel::componentSquareRatio( Pixel const & p )
{
  int rr_diff = this->red_ratio - p.red_ratio;
  int gr_diff = this->green_ratio - p.green_ratio;
  int br_diff = this->blue_ratio - p.blue_ratio;
  
  return rr_diff * rr_diff + gr_diff * gr_diff + br_diff * br_diff;
}

void
Pixel::setMinimum( Pixel const & p )
{
  RawPixel::setMinimum( p );
  if ( p.red_green < red_green )
    {
      red_green = p.red_green;
    }
  if ( p.red_blue < red_blue )
    {
      red_blue = p.red_blue;
    }
  if ( p.green_blue < green_blue )
    {
      green_blue = p.green_blue;
    }

  if ( p.red_ratio < red_ratio )
    {
      red_ratio = p.red_ratio;
    }
  if ( p.green_ratio < green_ratio )
    {
      green_ratio = p.green_ratio;
    }
  if ( p.blue_ratio < blue_ratio )
    {
      blue_ratio = p.blue_ratio;
    }
}

void
Pixel::setMaximum( Pixel const & p )
{
  RawPixel::setMaximum( p );
  if ( p.red_green > red_green )
    {
      red_green = p.red_green;
    }
  if ( p.red_blue > red_blue )
    {
      red_blue = p.red_blue;
    }
  if ( p.green_blue > green_blue )
    {
      green_blue = p.green_blue;
    }

  if ( p.red_ratio > red_ratio )
    {
      red_ratio = p.red_ratio;
    }
  if ( p.green_ratio > green_ratio )
    {
      green_ratio = p.green_ratio;
    }
  if ( p.blue_ratio > blue_ratio )
    {
      blue_ratio = p.blue_ratio;
    }
}

std::ostream & 
operator<<(std::ostream & os, Pixel const & pixel)
{
  os << pixel.red << " " << pixel.green << " " << pixel.blue << " ";
  os << pixel.red_green << " " << pixel.red_blue << " " << pixel.green_blue << " ";
  os << pixel.red_ratio << " " << pixel.green_ratio << " " << pixel.blue_ratio;
  return os;
}

std::istream & 
operator>>( std::istream & is, Pixel & pixel )
{
  is >> pixel.red >> pixel.green >> pixel.blue;
  is >> pixel.red_green >> pixel.red_blue >> pixel.green_blue;
  is >> pixel.red_ratio >> pixel.green_ratio >> pixel.blue_ratio;
  return is;
}

std::ostream &
operator<<( std::ostream & os, RawPixel & p )
{
  os << "[" << p.red << "," << p.green << "," << p.blue << "]";
  return os;
}

Pixel & 
Pixel::operator=( RawPixel const & p )
{
  if ( this != & p )
    {
      red = p.red;
      green = p.green;
      blue = p.blue;
      update( );
    }
  return * this;
}

Pixel::Pixel( const Pixel & p) :
  RawPixel(p),
  red_green(p.red_green),
  red_blue(p.red_blue),
  green_blue(p.green_blue),
  red_ratio(p.red_ratio),
  green_ratio(p.green_ratio),
  blue_ratio(p.blue_ratio)
{
  // 0
}

RawPixel::RawPixel( const RawPixel & p) :
  red(p.red),
  green(p.green),
  blue(p.blue)
{
  //0
}

