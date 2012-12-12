// $Id: framebufferrgb565.h,v 1.1.1.1.2.1 2004/10/26 04:53:49 cvs Exp $
// Jacky Baltes <jacky@cs.umanitoba.ca>
// Sun May 16 12:10:25 CDT 2004
//

#ifndef _FRAME_BUFFER_RGB565_H_
#define _FRAME_BUFFER_RGB565_H_

#include "framebuffer.h"

class FrameBufferRGB565 : public FrameBuffer
{
 public:
  FrameBufferRGB565( );
  void getPixel( void * ptr, RawPixel * pixel ) const;
  void getBlurred3x3Pixel( void * ptr, RawPixel * pixel ) const;
  void setPixel( void * ptr, RawPixel const pixel );
};



#endif
