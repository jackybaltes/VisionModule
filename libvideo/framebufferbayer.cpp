#include "framebufferbayer.h"
FrameBufferBayer::FrameBufferBayer()
{
  _type = FrameBuffer::BAYER;
  bytesPerPixel = 1;
  initialized =0;
}


FrameBufferBayer::~FrameBufferBayer()
{
  if(initialized)
  {
    delete [] red;
    delete  [] green;
    delete  [] blue;
    delete  [] frameSeen;
  }
}

void FrameBufferBayer::initialize( unsigned int width, unsigned int height, uint8_t * buff )
{
  if(!initialized)
  {
    FrameBuffer::initialize( width, height, buff );

    initialized =1;
    int frameLen = height * width;
    red = new uint8_t[frameLen];
    green = new uint8_t[frameLen];
    blue = new uint8_t[frameLen];
    frameSeen = new unsigned int[frameLen];
  }
}

void 
FrameBufferBayer::getPixel( void * ptr, RawPixel * pixel ) const
{
  uint8_t *p = static_cast<uint8_t *>( ptr );
  unsigned long offset = p - buffer;
  unsigned int line = offset/width;
  unsigned int position = offset % width;
  unsigned int lineParity = line & 1;
  unsigned int positionParity = position & 1;
 
  if(initialized && frameSeen[offset] == absFrameNo)
  {
    pixel->red = red[offset];
    pixel->green = green[offset];
    pixel->blue = blue[offset];
  }
  else
  {
    //frame buffer will be bordered by black pixels.
    if(position == 0 || position == width -1 || line == 0 || line == height-1)
    {
      pixel->red = 0;
      pixel->green = 0;
      pixel->blue = 0;
    }
    else if(lineParity == positionParity)
    {
      pixel->green = (*(p+1) + *(p-1) + *(p+width) + *(p-width))/4;
      if(lineParity)
      {
        //blue pixel (odd line, odd position)
        pixel->blue = *p;
        pixel->red = (*(p + width + 1) + *(p + width -1) + *(p - width + 1) + *(p - width -1))/4;
      }
      else
      {
        //red pixel (even line, even position)
        pixel->red = *p;
        pixel->blue = (*(p + width + 1) + *(p + width -1) + *(p - width + 1) + *(p - width -1))/4;
      }
    }
    else
    {
      //green pixel (other)
      pixel->green = *p;
      if(lineParity)
      {
        pixel->blue = ( *(p+1) + *(p-1))/2;
        pixel->red = (*(p+width)+ *(p-width))/2;
      }
      else
      {
        pixel->red = ( *(p+1) + *(p-1))/2;
        pixel->blue = (*(p+width)+ *(p-width))/2;
      }
    }
    red[offset] = pixel->red;
    green[offset] = pixel->green;
    blue[offset] = pixel->blue;
    frameSeen[offset] = absFrameNo;
  }
}

void 
FrameBufferBayer::setPixel( void * ptr, RawPixel const pixel )
{
  uint8_t *p = static_cast<uint8_t *>( ptr );
  long offset = p - buffer;
  //int line = offset/width;
  //int position = offset % width;
  //int lineParity = line & 1;
  //int positionParity = position & 1;

    red[offset] = pixel.red;
    green[offset] = pixel.green;
    blue[offset] = pixel.blue;
    frameSeen[offset] = absFrameNo;
}
