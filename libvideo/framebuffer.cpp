// $Id: framebuffer.cpp,v 1.1.1.1.2.6 2005/02/06 00:44:02 cvs Exp $
//

#include "framebuffer.h"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <string>
#include <sys/time.h>
#include <time.h>
#include "pixel.h"
#include <inttypes.h>
#include "geometry.h"

#include <assert.h>
#include "framebufferrgb24be.h"
#include "framebufferrgb24.h"
#include "framebufferrgb32.h"
#include "framebufferrgb565.h"
#include "framebufferbayer.h"

using namespace std;

static void
insertionSortPixels( RawPixel * pixels, int n, RawPixel pix );

FrameBuffer::FrameBuffer( ) :
  buffer( 0 ),
  width( 0 ),
  height( 0 ),
  bytesPerPixel( 0 ),
  bytesPerLine( 0 ),
  absFrameNo( 0 ),
  owner( false ),
  frameSize( 0 ),
  min( Pixel( 255, 255, 255 ) ),
  max( Pixel(   0,   0,   0 ) ),
  _refCount( 0 ),
  _type( UNKNOWN )
{
}

FrameBuffer::FrameBuffer( std::string const filename ) :
  buffer( 0 ),
  width( 0 ),
  height( 0 ),
  owner( false ),
  frameSize( 0 ),
  min( Pixel( 255, 255, 255 ) ),
  max( Pixel(   0,   0,   0 ) ),
  _refCount( 0 )
{
  inFromPPM(filename);
}

bool FrameBuffer::inFromPPM( std::string const filename )
{
  bool good = true;
  std::ifstream file( filename.c_str() );
  if ( ! file )
    {
      std::cerr << "Framebuffer::FrameBuffer(" << filename << ") unable to open input file\n";
      good = false;
    }
  else
    {
      if ( inFromPPM( file ) )
	{
	  std::cerr << "Framebuffer::FrameBuffer(" << filename << ") inFromPPM failed\n";
	  good = false;
	}
    }
  return good;
}


FrameBuffer::~FrameBuffer()
{
  while ( refCount() > 0 )
    {
#ifdef DEBUG
      std::cerr << __PRETTY_FUNCTION__ << " deleting frame number " << absFrameNo << " with reference count " << refCount() << endl;
#endif      
    }

  if ( ( owner ) && ( buffer != 0 ) )
    {
      //free ( buffer );
      delete[] buffer;
    }
  buffer = 0;
}

void
FrameBuffer::getPixel( int row, int column, RawPixel * pixel ) const
{
  void * p = buffer + row * bytesPerLine + column * bytesPerPixel;
  getPixel( p, pixel );
}

void
FrameBuffer::getBlurred3x3Pixel( void * ptr, RawPixel * out ) const
{
  uint8_t * p = static_cast< uint8_t * > ( ptr );
  RawPixel pixel;
  RawPixel avg;
  RawPixel o;

  getPixel( p, &avg );
  getPixel( p - bytesPerLine - bytesPerPixel, &pixel );
  avg = avg + pixel;
  getPixel( p - bytesPerLine                , &pixel );
  avg = avg + pixel;
  getPixel( p - bytesPerLine + bytesPerPixel, &pixel );
  avg = avg + pixel;

  getPixel( p                - bytesPerPixel, &pixel );
  avg = avg + pixel;
  getPixel( p                + bytesPerPixel, &pixel );
  avg = avg + pixel;

  getPixel( p + bytesPerLine - bytesPerPixel, &pixel );
  avg = avg + pixel;
  getPixel( p + bytesPerLine                , &pixel );
  avg = avg + pixel;
  getPixel( p + bytesPerLine + bytesPerPixel, &pixel );
  avg = avg + pixel;

  *out = avg / 9.0;
}

unsigned int 
FrameBuffer::getBlurred3x3Intensity(void * ptr) const
{
  RawPixel p;
  getBlurred3x3Pixel(ptr, &p);
  return p.intIntensity();
}

void
FrameBuffer::setPixel( unsigned int row, unsigned int column, RawPixel const pixel )
{
  if ( ( row < height ) && ( column < width ) )
    {
      void * p = buffer + row * bytesPerLine + column * bytesPerPixel;
      setPixel( p, pixel );
    }
}

void FrameBuffer::setPixelAt( unsigned int row, unsigned int col, RawPixel const pixel )
{
  setPixel(row,col,pixel);
}

unsigned int FrameBuffer::getIntensity(unsigned int row, unsigned int col)
{
  uint intensity = 0;
  if ( ( row < height ) && ( col < width ) )
    {
      void * p = (void *)(buffer + (row * bytesPerLine) + (col * bytesPerPixel));
      intensity = getIntensity(p);
    }
  return intensity;
}

void FrameBuffer::outToPPM( std::string const filename )
{
  std::ofstream file( filename.c_str() );
  if ( ! file )
    {
      std::cerr << "FrameBuffer::outToPPM(" << filename << ") unable to open output file\n";
      std::exit( 1 );
    }
  outToPPM( file );
}

void FrameBuffer::outToPPM( std::ostream & os )
{
  os << "P6\n";
  os << width << " " << height << '\n';
  os << "255\n";
  for( unsigned int i = 0; i < height; i++ )
    {
      for( unsigned int j = 0; j < width; j++ )
	{
	  RawPixel p;
	  getPixel( i, j, &p );
	  os.put( p.red );
	  os.put( p.green );
	  os.put( p.blue );
	}
    }
}

bool FrameBuffer::inFromPPM( std::istream & is )
{
  int w, h, depth;
  std::string type;
  
  is >> type;
  
  if ( type != "P6" )
    {
      return true;
    }
  
  is >> w >> h;
  is >> depth;
  
#ifdef DEBUG
  std::cout << "FrameBuffer::inFromPPM read in frame of size " << w << 'x' << h << 'x' << depth << std::endl;
#endif
  
  if ( depth != 255 )
    {
      return true;
    }
  
  this->width = w;
  this->height = h;
  
  if ( ( this->buffer == 0 ) || ( this->frameSize < (unsigned int)(3 * w * h ) ) )
    {
      if ( ( this->buffer != 0 ) && ( this->frameSize < (unsigned int)(3 * w * h ) ) && ( this->owner ) )
	{
	  free( this->buffer );
	  this->buffer = 0;
	  this->owner = false;
	  this->frameSize = 0;
	}
      if ( ( buffer = ( unsigned char * ) malloc( 3 * w * h ) ) == 0 )
	{
	  std::cerr << "FrameBuffer::inFromPPM failed to allocate memory\n";
	  return true;
	}
      owner = true;
      frameSize = 3 * width * height;
    }
  
  bytesPerPixel = 3;
  bytesPerLine = width * bytesPerPixel;
  
  for( unsigned int i = 0; i < height; i++ )
    {
      for( unsigned int j = 0; j < width; j++ )
	{
	  for( int c = 0; c < 3; c++ )
	    {
	      buffer[ i * bytesPerLine + j * bytesPerPixel + c ] = is.get();
	    }
	}
    }

  fieldNo = 0;
  absFrameNo = 0;
  interlaced = false;
  fieldsPerSecond = 60;
  gettimeofday( & t , NULL );
  delay = 0;

  return false;
}

FrameBuffer & FrameBuffer::operator=( FrameBuffer const & frame )
{
  if ( this != & frame )
    {
      if ( ( this->buffer != 0 ) && ( this->owner ) && ( this->frameSize < frame.frameSize ) )
	{
	  free( this->buffer );
	  this->owner = false;
	  this->frameSize = 0;
	  this->buffer = 0;
	}
      if ( this->buffer == 0 )
	{
	  this->buffer = (unsigned char *) malloc( frame.height * frame.bytesPerLine );
	  this->frameSize = frame.height * frame.bytesPerLine;
	  this->owner = true;
	}
      if ( this->buffer == 0 )
	{
	  std::cerr << "FrameBuffer::operator= failed\n";
	}
      else
	{
	  memcpy( this->buffer, frame.buffer, frame.height * frame.bytesPerLine );
	}
      this->width = frame.width;
      this->height = frame.height;
      this->bytesPerPixel = frame.bytesPerPixel;
      this->bytesPerLine = frame.bytesPerLine;
      this->fieldNo = frame.fieldNo;
      this->absFrameNo = frame.absFrameNo;
      this->interlaced = frame.interlaced;
      this->fieldsPerSecond = frame.fieldsPerSecond;
      memcpy( & (this->t), & frame.t, sizeof( this->t ) );
      this->delay = frame.delay;
      this->_type = frame.type();
    }
  return * this;
}

void
FrameBuffer::getPixel( void * ptr, Pixel * pixel ) const
{
  getPixel( ptr, (RawPixel * ) pixel );
  pixel->update();
}

void
FrameBuffer::getBlurred3x3Pixel( void * ptr, Pixel * pixel ) const
{
  getBlurred3x3Pixel( ptr, (RawPixel * ) pixel );
  pixel->update();
}

void
FrameBuffer::getMedianPixel( void * pixelPtr, RawPixel * pixel ) const
{
  RawPixel sortedPixels[ 9 ];
  int numSorted = 0;
  RawPixel pix;

  for( int i = - 1; i <= 1; i++ )
    {
      for( int j = -1; j <= 1; j++ )
	{
	  getPixel( static_cast<uint8_t *>( pixelPtr ) + bytesPerLine * i + bytesPerPixel * j, &pix );
	  insertionSortPixels( sortedPixels, numSorted, pix );
	  numSorted++;
	}
    }
  *pixel = sortedPixels[ 3 ] + sortedPixels[ 4 ] + sortedPixels[ 5 ];
  *pixel = *pixel / 3.0;
} 

void
FrameBuffer::getMedianPixel( void * pixelPtr, Pixel * pixel ) const
{
  RawPixel sortedPixels[ 9 ];
  int numSorted = 0;
  RawPixel pix;

  for( int i = - 1; i <= 1; i++ )
    {
      for( int j = -1; j <= 1; j++ )
	{
	  getPixel( static_cast<uint8_t *>( pixelPtr ) + bytesPerLine * i + bytesPerPixel * j, &pix );
	  insertionSortPixels( sortedPixels, numSorted, pix );
	  numSorted++;
	}
    }
  *pixel = sortedPixels[ 4 ];
  pixel->update( );
} 

void
FrameBuffer::setMinimum( Pixel const & p )
{
  min = p;
}

void
FrameBuffer::setMaximum( Pixel const & p )
{
  max = p;
}

Pixel
FrameBuffer::getMinimum( void ) const
{
  return min;
}

Pixel
FrameBuffer::getMaximum( void ) const
{
  return max;
}

void
FrameBuffer::initialize( unsigned int width, unsigned int height, uint8_t * buff )
{
  this->width = width;;
  this->height = height;

  bytesPerLine = width * bytesPerPixel;
  fieldNo = 0;
  interlaced = true;
  
  frameSize = height * bytesPerLine;
  if ( buff != 0 )
    {
      this->buffer = buff;
      owner = false;
    }
  else
    {
      this->buffer = new uint8_t[ frameSize ];
      owner = true;
    }
  assert( buffer != 0 );
}

void
FrameBuffer::getMosaicedPixel( unsigned int row, unsigned int col, RawPixel * pixel ) const
{
  FrameBufferIterator iter( ( FrameBuffer * )this, row, col );
  iter.getMosaicedPixel( pixel );
}


void
FrameBuffer::convolution5x5( void * p, double const matrix[ 5 ][ 5 ], double const c_divisor, double const c_offset, RawPixel * pixel )
{
  double red = c_offset;
  double green = c_offset;
  double blue = c_offset;

  RawPixel( pix );
  for( int i = - 2; i <= 2; i++ )
    {
      for( int j = -2; j <= 2; j++ )
	{
	  getPixel( static_cast< uint8_t * >( p ) + bytesPerLine * i + bytesPerPixel * j, &pix );
	  red = red + matrix[i + 2][j + 2] * pix.red;
	  green = green + matrix[i + 2][j + 2] * pix.green;
	  blue = blue + matrix[i + 2][j + 2] * pix.blue;
	}
    }
  red = red / c_divisor;
  green = green / c_divisor;
  blue = blue / c_divisor;
  * pixel = RawPixel( static_cast<int>( red ), static_cast<int>( green ), static_cast<int>( blue ) );
}

bool
FrameBuffer::isValidPoint( unsigned int x, unsigned int y ) const
{
  bool result = false;
  if ( ( x < width ) && ( y < height ) )
    {
      result = true;
    }
  return result;
}


bool 
FrameBuffer::isEdge( void * pptr, double threshold ) const
{
  RawPixel pixel;
  RawPixel prevPixel;
  bool isEdge = false;

  uint8_t * p = static_cast< uint8_t * > ( pptr );

  uint8_t * p1 = p - bytesPerLine - 2 * bytesPerPixel;

  bool isTopEdge = false;
  getPixel( p1, & prevPixel );
  for( int j = -1; j <= 1; j++ )
    {
      getPixel( p1 + j * bytesPerPixel, & pixel );
      if ( pixel.diffMax( prevPixel ) > threshold )
	{
	  isTopEdge = true;
	  break;
	}
      prevPixel = pixel;
    }

  if ( isTopEdge )
    {
      uint8_t * p2 = p + bytesPerLine - 2 * bytesPerPixel;
      
      getPixel( p2, & prevPixel );
      for( int j = -1; j <= 1; j++ )
	{
	  getPixel( p2 + j * bytesPerPixel, & pixel );
	  if ( pixel.diffMax( prevPixel ) > threshold )
	    {
	      isEdge = true;
	      break;
	    }
	  prevPixel = pixel;
	}
    }
  return isEdge;
}

void FrameBuffer::getNormalizedPixel( void * ptr, RawPixel * pixel ) const
{
  getPixel(ptr,pixel);
  int sum = pixel->red + pixel->green + pixel->blue + 1;

  if ( sum < 96 )
    {
      pixel->red = 0;
      pixel->green = 0;
      pixel->blue = 0;
    }
  else if ( sum < 672 )
    {
      pixel->red = ( pixel->red * 255 ) / sum;
      pixel->green = ( pixel->green * 255 ) / sum;
      pixel->blue = ( pixel->blue * 255 ) / sum;
    }
  else
    {
      pixel->red = 255;
      pixel->green = 255;
      pixel->blue = 255;
    }
}

/* FrameBuffer Iterator Methods
 *
 */



void FrameBufferIterator::getNormalizedPixel( RawPixel * pixel ) const
{
  frame->getNormalizedPixel(pixelPtr, pixel);
}

void
FrameBufferIterator::convolution5x5( double const matrix[ 5 ][ 5 ], double const c_divisor, double const c_offset, RawPixel * pixel )
{
  frame->convolution5x5( pixelPtr, matrix, c_divisor, c_offset, pixel );
}

void
FrameBufferIterator::getMosaicedPixel( RawPixel * pixel )
{
  RawPixel pixels[ 3 ][ 3 ];
  int red, green, blue;

  for( int i = - 1; i <= 1; i++ )
    {
      for( int j = -1; j <= 1; j++ )
	{
	  getPixel( &pixels[ i + 1 ][ j + 1 ], frame->bytesPerLine * i + frame->bytesPerPixel * j );
	}
    }
  
  /* Assume RGRGRGRG
   *        GBGBGBGB
   *        RGRGRGRG
   *        GBGBGBGB
   * Bayer pattern
   */

  if ( ( row % 2 == 0 ) && ( column % 2 == 0) )
    {
      red = pixels[ 1 ][ 1 ].red;
      green = ( pixels[ 1 ][ 0 ].green + pixels[ 1 ][ 2 ].green + pixels[ 0 ][ 1 ].green + pixels[ 2 ][ 1 ].green ) / 4;
      blue = ( pixels[ 0 ][ 0 ].blue + pixels[ 0 ][ 2 ].blue + pixels[ 2 ][ 0 ].blue + pixels[ 2 ][ 2 ].blue ) / 4;
    }
  else if ( ( row % 2 == 0 ) && ( column % 2 == 1 ) )
    {
      red = ( pixels[ 1 ][ 0 ].red + pixels[ 1 ][ 2 ].red ) / 2;
      green = pixels[ 1 ][ 1 ].green;
      blue = ( pixels[ 0 ][ 1 ].blue + pixels[ 2 ][ 1 ].blue ) / 2;      
    }
  else if ( ( row % 2 == 1 ) && ( column % 2 == 0 ) )
    {
      red = ( pixels[ 0 ][ 1 ].red + pixels[ 2 ][ 1 ].red ) / 2;
      green = pixels[ 1 ][ 1 ].green;
      blue = ( pixels[ 1 ][ 0 ].blue + pixels[ 1 ][ 2 ].blue ) / 2;      
    }
  else
    {
      red = ( pixels[ 0 ][ 0 ].red + pixels[ 0 ][ 2 ].red + pixels[ 2 ][ 0 ].red + pixels[ 2 ][ 2 ].red ) / 4;
      green = ( pixels[ 0 ][ 1 ].green + pixels[ 1 ][ 0 ].green + pixels[ 1 ][ 2 ].green + pixels[ 2 ][ 1 ].green ) / 4;
      blue = pixels[ 1 ][ 1 ].blue;
    }
  * pixel = RawPixel( red, green, blue );
}

void
FrameBufferIterator::getMedianPixel( RawPixel * pixel )
{
  frame->getMedianPixel( pixelPtr, pixel );
} 

static void
insertionSortPixels( RawPixel * pixels, int n, RawPixel pix )
{
  int in = 0;
  
  while( ( in < n ) && ( pixels[ in ].max() < pix.max() ) )
    {
      in++;
    }
  for( int j = n; j >= in; j-- )
    {
      pixels[ j + 1 ] = pixels[ j ];
    }
  pixels[ in ] = pix;
}

void
FrameBuffer::drawCenter( RawPixel const & p )
{
  unsigned int row = height / 2;
  unsigned int col = width / 2;

  setPixel( row, col, p );
}

void
FrameBuffer::drawBorder( RawPixel const & p )
{
  FrameBufferIterator it( this, 0, 0 );

  it.goRow( 0 );
  for( unsigned int i = 0; i < width; i++, it.goRight( ) )
    {
      it.setPixel( p );
    }

  it.goRow( height - 1 );
  for( unsigned int i = 0; i < width; i++, it.goRight( ) )
    {
      it.setPixel( p );
    }

  it.goCol( 0 );
  for( unsigned int i = 0; i < height; i++, it.goDown( ) )
    {
      it.setPixel( p );
    }

  it.goCol( width - 1 );
  for( unsigned int i = 0; i < height; i++, it.goDown( ) )
    {
      it.setPixel( p );
    }
}



void
FrameBuffer::convolution5x5( double const matrix[ 5 ][ 5 ], double const divisor, double const offset )
{
  RawPixel pixels[ height - 4 ][ width - 4 ];
  FrameBufferIterator it( this );
  for( unsigned int i = 2; i < height - 2; i++ )
    {
      it.goPosition( i, 2 );
      for( unsigned int j = 2; j < width - 2; j++, it.goRight( ) )
	{
	  it.convolution5x5( matrix, divisor, offset, & pixels[ i - 2 ][ j - 2 ] );
	}
    }
  
  for( unsigned int i = 0; i < height - 4; i++ )
    {
      it.goPosition( i + 2, 2 );
      for( unsigned int j = 0; j < width - 4; j++, it.goRight( ) )
	{
	  it.setPixel( pixels[ i ][ j ] );
	}
    }
}

unsigned int 
FrameBuffer::getIntensity(void * ptr)
{
  Pixel p;
  getPixel(ptr,&p);
  return p.intIntensity();
}

// sets the timestamp and the delay.
void FrameBuffer::setTimestamp()
{
  struct timeval now;
  if ( gettimeofday( & now, 0  ) != 0 )
    {
      std::cerr << "Framebuffer::updateFPS gettimeofday failed\n";
      perror( "VideoStream::updateFPS gettimeofday ");
    }

  if ( now.tv_usec >= t.tv_usec )
    {
      delay = (now.tv_usec - t.tv_usec) + (1000000 * ( now.tv_sec - t.tv_sec ));
    }
  else
    {
      delay = now.tv_usec + 1000000 - t.tv_usec + 1000000 * ( now.tv_sec - t.tv_sec - 1);
    }

  memcpy( & t, & now, sizeof( struct timeval ) );
}

FrameBuffer* FrameBuffer::createWorkingFrame(FrameBuffer* other)
{
  FrameBuffer * newBuf =0;
  switch (other->type())
  {
    case FrameBuffer::RGB32:
      newBuf = new FrameBufferRGB32();
    break;
    case FrameBuffer::RGB565:
      newBuf = new FrameBufferRGB565();
    break;
    case FrameBuffer::RGB24:
      newBuf = new FrameBufferRGB24();
    break;
    case FrameBuffer::RGB24BE:
      newBuf = new FrameBufferRGB24BE();
    break;
    case FrameBuffer::BAYER:
      newBuf = new FrameBufferBayer();
      newBuf->initialize( other->height, other->width );
    break;
    default:
    std::cerr << __PRETTY_FUNCTION__ << " Unsupported frame type " << other->type();
  }
  return newBuf;
}

void
FrameBuffer::fill( RawPixel const pix )
{
  fill( Rect( Point(0,0), Point( width, height) ), pix );
}

void
FrameBuffer::fill( Rect const & rect, RawPixel const pix )
{
  FrameBufferIterator it( this );

  for( unsigned int i = rect.topLeft().y(); i < rect.bottomRight().y(); i++ )
    {
      it.goPosition( i, 0 );
      for( unsigned int j = rect.topLeft().x(); j < rect.bottomRight().x(); j++, it.goRight() )
	{
	  it.setPixel( pix );
	}
    }
}

unsigned int
FrameBuffer::ConvertToJpeg( uint8_t * buffer, unsigned int maxSize, unsigned int quality )
{
  return 0;
}
