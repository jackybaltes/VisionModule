// $Id: framebufferrgb32.h,v 1.1.1.1 2004/09/16 23:12:05 cvs Exp $
// Jacky Baltes <jacky@cs.umanitoba.ca>
// Sun May 16 12:10:25 CDT 2004
//

#ifndef _FRAME_BUFFER_RGB32_H_
#define _FRAME_BUFFER_RGB32_H_

#include "framebuffer.h"

class FrameBufferRGB32 : public FrameBuffer
{
 public:
  FrameBufferRGB32();
  void getPixel( void * p, RawPixel * pixel ) const;
  void setPixel( void * p, RawPixel const pixel );
};

#endif
