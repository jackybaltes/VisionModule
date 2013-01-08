// $Id: v4l2device.h,v 1.1.1.1.2.2 2004/10/15 03:34:29 cvs Exp $
//

#ifndef _V4L2_DEVICE_H_
#define _V4L2_DEVICE_H_

#include "videodevice.h"
#include <linux/videodev.h>

using namespace std;

// Forward declarations
class FrameBuffer;

class V4L2Device: public VideoDevice
{
public:
  ~V4L2Device( );
  V4L2Device ( std::string devname, std::string input, std::string standard, unsigned int fps, unsigned int width, unsigned int height, unsigned int depth, unsigned int numBuffers );

  enum ErrorCode
    {
      OKAY = 0,
      OPEN_FAILURE,
      QUERYCAP_FAILURE,
      NO_CAPTURE_FAILURE,
      NO_STREAMING_FAILURE,
      NO_INPUT_FAILURE,
      SET_INPUT_FAILURE,
      GET_INPUT_FAILURE,
      UNKNWON_STANDARD_FAILURE,
      SET_STANDARD_FAILURE,
      GET_STANDARD_FAILURE,
      GET_FORMAT_FAILURE,
      SET_FORMAT_FAILURE,
      GET_PARAMETER_FAILURE,
      SET_PARAMETER_FAILURE,
      REQUEST_BUFFER_FAILURE,
      QUERY_BUFFER_FAILURE,
      MMAP_BUFFER_FAILURE,
      QBUF_BUFFER_FAILURE,
      VIDIOC_STREAMON_FAILURE,
      VIDIOC_STREAMOFF_FAILURE,
      VIDIOC_QBUF_FAILURE,
      LAST_FAILURE
    };

  enum ErrorCode getErrorCode( void ) const;

  int startCapture( void );
  int stopCapture( void );
  void nextFrame( FrameBuffer * * curFrame );
  int releaseCurrentBuffer( void );
  bool isInterlaced( void );
  int  setControl(struct v4l2_control * vc);
  int queryControl(struct v4l2_queryctrl * qc);
  int getControl(struct v4l2_control * vc);

 public:
  virtual int GetBrightness( void );
  virtual int SetBrightness( unsigned int val );
  virtual int GetContrast( void );
  virtual int SetContrast( unsigned int val );
  virtual int GetSaturation( void );
  virtual int SetSaturation( unsigned int val );
  virtual int GetSharpness( void );
  virtual int SetSharpness( unsigned int val );
  virtual int GetGain( void );
  virtual int SetGain( unsigned int val );

  static unsigned int const XNUM_BUFFERS;

private:

  struct VideoBuffer
  {
    struct v4l2_buffer vidbuf;
    void *data;
  };
  
  struct VideoBuffer videobuffer[3];
  struct v4l2_buffer tempbuf;
  struct v4l2_format fmt;

  void printCapabilities ( ostream & os , struct v4l2_capability cap );
  void printInput ( ostream & os, struct v4l2_input inp );
  void printStandardId ( ostream & os, v4l2_std_id id );
  void printFormatDesc( ostream & os, struct v4l2_fmtdesc const fmtd );
  void printBufferType( ostream & os, enum v4l2_buf_type const type );
  void printPixelFormat4CC( ostream & os, __u32 const pixelformat );
  void printPixelFormat( ostream & os, v4l2_pix_format const pixelformat );
  void printFormat( ostream & os, struct v4l2_format const fmt);
  void printField( ostream & os, v4l2_field const f );
  void printColorSpace( ostream & os, v4l2_colorspace const c );
  void printStreamingParameters ( ostream & os, struct v4l2_streamparm const parm );
  void printCaptureParameters ( ostream & os, struct v4l2_captureparm parm );
  void printStandard ( ostream & os , v4l2_standard const std );
  void printBuffer( ostream & os, struct v4l2_buffer const buffer );

  enum ErrorCode errorCode;
  int fd;
};
#endif /* _V4L2_DEVICE_H_ */
