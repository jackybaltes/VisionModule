// $Id: framebuffer.h,v 1.1.1.1.2.13 2005/03/05 21:56:44 cvs Exp $
//

#ifndef _FRAME_BUFFER_H_
#define _FRAME_BUFFER_H_

#include <sys/time.h>
#include <time.h>

#include <unistd.h>
#undef min
#undef max
#include <ostream>
#include <string>
#include <inttypes.h>
#include <assert.h>
#include "pixel.h"
#include "geometry.h"
#include "rect.h"

// Forward declarations

class FrameBuffer
{
 public:

  enum FrameBufferType
    {
      UNKNOWN = 0,
      RGB32 = 1,
      RGB565 = 2,
      RGB24 = 3,
      RGB24BE = 4,
      BAYER = 5,
      WORLD_FRAME
    };

  FrameBuffer( );
  FrameBuffer( std::string const filename );

  void initFromOther(const FrameBuffer & other);
  virtual void initialize( unsigned int width, unsigned int height, uint8_t * buffer = 0 );
  virtual ~FrameBuffer();

  void outToPPM( std::ostream & os );
  void outToPPM( std::string const filename );
  bool inFromPPM( std::istream & is );
  bool inFromPPM( std::string const filename );

  void setTimestamp();

  virtual void getPixel( int row, int col, RawPixel * pixel ) const;
  virtual void getPixel( void * ptr, RawPixel * pixel ) const = 0;
  virtual void getPixel( void * ptr, Pixel * pixel ) const;

  virtual void getBlurred3x3Pixel( void * ptr, Pixel * pixel ) const;
  virtual void getBlurred3x3Pixel( void * ptr, RawPixel * pixel ) const;

  virtual unsigned int getBlurred3x3Intensity(void * ptr) const;

  virtual void getMedianPixel( void * ptr, Pixel * pixel ) const;
  virtual void getMedianPixel( void * ptr, RawPixel * pixel ) const;

  virtual void getNormalizedPixel( void * ptr, RawPixel * pixel ) const;

  virtual void getMosaicedPixel( unsigned int row, unsigned int col, RawPixel * pixel ) const;

  virtual unsigned int getIntensity(void * ptr);
  virtual unsigned int getIntensity(unsigned int row, unsigned int col);

  void convolution5x5( void * p, double const matrix[ 5 ][ 5 ], double const c_divisor, double const c_offset, RawPixel * pixel );
  void convolution5x5( void * p, double const matrix[ 5 ][ 5 ], double const c_divisor, double const c_offset, Pixel * pixel );

  void convolution5x5( double const matrix[ 5 ][ 5 ], double const c_divisor, double const c_offset );

  virtual void setPixelAt( unsigned int row, unsigned int col, RawPixel const pixel );
  virtual void setPixel( unsigned int row, unsigned int col, RawPixel const pixel );
  virtual void setPixel( void * ptr, RawPixel const pixel ) = 0;

  virtual bool isValidPoint( unsigned int x, unsigned int y ) const;
  virtual bool isEdge( void * pptr, double threshold ) const;

  void setMinimum( Pixel const & p );
  void setMaximum( Pixel const & p );

  Pixel getMinimum( void ) const;
  Pixel getMaximum( void ) const;

  FrameBuffer & operator=( FrameBuffer const & frame );

  void drawBorder( RawPixel const & p );
  void drawCenter( RawPixel const & p );

  void fill( RawPixel const p );
  void fill( Rect const & rect, RawPixel const pix );

  unsigned char * getPixelAddr( int col, int row ) const { return buffer + row * bytesPerLine + col * bytesPerPixel; }

  inline enum FrameBufferType type( void ) const { return _type; };

  inline unsigned int refCount( void ) { return _refCount; };
  inline void setRefCount( unsigned int count ) { _refCount = count; };
  inline unsigned int incRefCount( void  ) { _refCount++; return _refCount; };
  inline unsigned int decRefCount( void  ) { _refCount--; return _refCount; };

  static FrameBuffer* createWorkingFrame(FrameBuffer* other);

  void clear( RawPixel const pix );

 public:

  uint8_t * buffer;
  unsigned int width;
  unsigned int height;
  unsigned int bytesPerPixel;
  unsigned int bytesPerLine;
  unsigned int fieldNo;
  unsigned int absFrameNo;
  bool interlaced;
  unsigned int fieldsPerSecond;
  struct timeval t;
  long delay;
  bool owner;
  unsigned int frameSize;
 private:
  Pixel min;
  Pixel max;
  volatile unsigned int _refCount;

 protected:
  enum FrameBufferType _type;
};

class FrameBufferIterator
{  
 public:

  enum ErrorCode
  {
    NO_ERROR = 0,
    OUT_OF_BOUNDS = 1
  };

  inline FrameBufferIterator( FrameBuffer const * frame, unsigned int row = 0, unsigned int column = 0 );
  inline FrameBufferIterator( FrameBufferIterator const & other);
  inline void getPixel( RawPixel * pixel, int offset = 0 );
  inline void getPixel( Pixel * pixel, int offset = 0 );

  inline void getBlurred3x3Pixel( RawPixel * pixel, int offset = 0 );
  inline void getBlurred3x3Pixel( Pixel * pixel, int offset = 0 );


  inline void setPixel( RawPixel const pixel, int offset = 0 );

  inline enum ErrorCode goRight( int steps = 1 );
  inline enum ErrorCode goLeft( int steps = 1 );
  inline enum ErrorCode goDown( int steps = 1 );
  inline enum ErrorCode goUp( int steps = 1 );
  inline enum ErrorCode goRow( unsigned int y );
  inline enum ErrorCode goCol( unsigned int x );
  inline enum ErrorCode goPosition( unsigned int row, unsigned int col );
  inline unsigned int getRow() const;
  inline unsigned int getColumn() const;
  inline unsigned int getIntensity() const;
  inline unsigned int getBlurred3x3Intensity() const;

  void convolution5x5( double const matrix[ 5 ][ 5 ], double const c_divisor, double const c_offset, RawPixel * pixel );
  void getMosaicedPixel( RawPixel * pixel );
  void getMedianPixel( RawPixel * pixel );
  void getMedianPixel( Pixel * pixel );
  void getNormalizedPixel( RawPixel * pixel ) const;
  

  inline void * getPixelPtr( void ) const;

 private:
  FrameBuffer * frame;
  uint8_t * pixelPtr;
  unsigned int row;
  unsigned int column;
};


// Inline Functions

inline
FrameBufferIterator::FrameBufferIterator( FrameBuffer const * frame, unsigned int row, unsigned int column ) : frame( 0 ), pixelPtr ( 0 ), row( row ), column( column )
{
  assert( ( frame != 0 ) && ( frame->buffer != 0 ) );
  
  this->frame = const_cast<FrameBuffer *>( frame );
  row = Geometry::clamp(0,row,frame->height - 1);
  column = Geometry::clamp(0,column,frame->width - 1);

  if ( frame != 0 )
    {
      pixelPtr = frame->getPixelAddr( column, row );
#ifdef XX_DEBUG
      std::cout << "New FrameBufferIterator.\n";
      std::cout << "Row: " << row << " Col: " << column << std::endl;
      std::cout << "Buffer: " << (void *) frame->buffer << std::endl;
      std::cout << "pixelPtr: " << (void *) pixelPtr << std::endl;
#endif
    }
}

inline void
FrameBufferIterator::getPixel( RawPixel * pixel, int offset )
{
#ifdef XX_DEBUG
  std::cout << "FBI::getPixel, frame: " << (void*)frame << ", pixelPtr: " << (void * ) pixelPtr << ", offset: " << offset << ", ptr+offset" << (void *)(pixelPtr + offset) << std::endl;
#endif
  frame->getPixel( pixelPtr + offset, pixel );
}

inline void
FrameBufferIterator::getBlurred3x3Pixel( RawPixel * pixel, int offset )
{
  frame->getBlurred3x3Pixel( pixelPtr + offset, pixel );
}

inline void
FrameBufferIterator::getPixel( Pixel * pixel, int offset )
{
  frame->getPixel( pixelPtr + offset, pixel );
}

inline void
FrameBufferIterator::getBlurred3x3Pixel( Pixel * pixel, int offset )
{
  frame->getBlurred3x3Pixel( pixelPtr + offset, pixel );
}

inline void
FrameBufferIterator::setPixel( RawPixel const pixel, int offset )
{
  frame->setPixel( pixelPtr + offset, pixel );
}

inline enum FrameBufferIterator::ErrorCode
FrameBufferIterator::goRight( int steps )
{
  enum FrameBufferIterator::ErrorCode ret = NO_ERROR;
  while( steps-- > 0 )
    {
      if ( column < frame->width - 1 )
	{
	  pixelPtr = pixelPtr + frame->bytesPerPixel;
	  column = column + 1;
	}
      else
	{
	  ret = OUT_OF_BOUNDS;
	  break;
	}
    }
  return ret;
}

inline enum FrameBufferIterator::ErrorCode
FrameBufferIterator::goLeft( int steps )
{
  enum FrameBufferIterator::ErrorCode ret = NO_ERROR;
  while ( steps-- > 0 )
    {
      if ( column > 0 )
	{
	  pixelPtr = pixelPtr - frame->bytesPerPixel;
	  column = column - 1;
	}
      else
	{
	  ret = OUT_OF_BOUNDS;
	  break;
	}
    }
  return ret;
}

inline enum FrameBufferIterator::ErrorCode
FrameBufferIterator::goDown( int steps )
{
  enum FrameBufferIterator::ErrorCode ret = NO_ERROR;
  while( steps-- > 0 )
    {
      if ( row < frame->height - 1 )
	{
	  pixelPtr = pixelPtr + frame->bytesPerLine;
	  row = row + 1;
	}
      else
	{
	  ret = OUT_OF_BOUNDS;
	  break;
	}
    }
  return ret;
}

inline enum FrameBufferIterator::ErrorCode
FrameBufferIterator::goUp( int steps )
{
  enum FrameBufferIterator::ErrorCode ret = NO_ERROR;
  while( steps-- > 0 )
    {
      if ( row > 0 )
	{
	  pixelPtr = pixelPtr - frame->bytesPerLine;
	  row = row - 1;
	}
      else
	{
	  ret = OUT_OF_BOUNDS;
	  break;
	}
    }
  return ret;
}

inline enum FrameBufferIterator::ErrorCode
FrameBufferIterator::goRow( unsigned int y )
{
  return goPosition( y, 0 );
}

inline enum FrameBufferIterator::ErrorCode
FrameBufferIterator::goCol( unsigned int x )
{
  return goPosition( 0, x );
}

inline enum FrameBufferIterator::ErrorCode
FrameBufferIterator::goPosition( unsigned int row, unsigned int col )
{
  enum FrameBufferIterator::ErrorCode result = NO_ERROR;

  if ( col >= frame->width )
    {
      result = OUT_OF_BOUNDS;
    }
  else if ( row >= frame->height )
    {
      result = OUT_OF_BOUNDS;
    }

  this->column = Geometry::clamp( 0, col, frame->width - 1 );
  this->row = Geometry::clamp( 0, row, frame->height - 1 );
  pixelPtr = frame->buffer + (frame->bytesPerLine * this->row) + (frame->bytesPerPixel * this->column);

  return result;
}

inline void *
FrameBufferIterator::getPixelPtr( void ) const
{
  return pixelPtr;
}

inline unsigned int 
FrameBufferIterator::getRow() const
{
  return row;
}

inline unsigned int 
FrameBufferIterator::getColumn() const
{
  return column;
}

inline 
FrameBufferIterator::FrameBufferIterator( const FrameBufferIterator & other)
{
  frame = other.frame;
  pixelPtr = other.pixelPtr;
  row = other.row;
  column = other.column;
}

inline unsigned int 
FrameBufferIterator::getIntensity() const
{
  return frame->getIntensity(pixelPtr);
}

inline unsigned int 
FrameBufferIterator::getBlurred3x3Intensity() const
{
  return frame->getBlurred3x3Intensity(pixelPtr);
}


#endif

