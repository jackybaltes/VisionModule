
#ifndef _DC1394_DEVICE_H_
#define _DC1394_DEVICE_H_

#include "videodevice.h"
#include <dc1394/control.h>

/*------------------------------------------------------------------------
 *  Definitions
 *------------------------------------------------------------------------*/
#define NUM_BUFFERS 4
#define ISO_SPEED DC1394_ISO_SPEED_400
#define DEBAYER_METHOD DC1394_BAYER_METHOD_NEAREST

/* ATTENTION! If you experience strange color inversions like
 * red to green or some strange horizontal line rippling
 * try to change the definition below to one of the other
 * commented formats:
 */
#define BAYER_FILTER DC1394_COLOR_FILTER_RGGB
//#define BAYER_FILTER DC1394_COLOR_FILTER_BGGR
//#define BAYER_FILTER DC1394_COLOR_FILTER_GBRG
//#define BAYER_FILTER DC1394_COLOR_FILTER_GRBG

#define MIN_IMG_WIDTH 640
#define MIN_IMG_HEIGHT 480

using namespace std;

// Forward declarations
class FrameBuffer;

class DC1394Device: public VideoDevice
{
public:
  ~DC1394Device( );
  DC1394Device ( std::string devname, std::string input, std::string standard, std::string bayer, int fps,unsigned int width,unsigned int height, int depth );

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

  bool bayer;
  int startCapture( void );
  int stopCapture( void );
  void nextFrame( FrameBuffer * * curFrame );
  int releaseCurrentBuffer( void );
  bool isInterlaced( void );
  int  setControl(struct dc1394_control vc);
  int queryControl(struct dc1394_queryctrl * qc);
  int getControl(struct dc1394_control * vc);
private:

  dc1394camera_t * mCamera;
  dc1394video_frame_t *mCapturedFrame; //Pointer to the frame
  //dc1394video_frame_t *mDeallocFrame; //Pointer to the frame
  dc1394video_frame_t *mDebayeredFrame; //Pointer to a RGB frame
  dc1394video_frame_t mRgbFrame; //The actual RGB frame

  //  unsigned int mWidth; //The width of the image
  //unsigned int mHeight; //The height of the image

  //functions ripped from PvLeague's Capture class
  int CameraCleanup(dc1394camera_t *pCamera);
  int GetCamera(dc1394camera_t **return_camera);


  /* Get video mode that matches the desired color coding
          */
  int GetVideoModeWithColorCoding(dc1394color_coding_t pColorCoding,
                          dc1394camera_t *pCamera,
                          dc1394video_modes_t pVideoModeList,
                          dc1394video_mode_t &pVideoMode);

  /* Get video mode that matches the desired image size
          */
  int
  GetVideoModeWithMinSize(unsigned int pMinWidth, unsigned int pMinHeight,
                  dc1394camera_t *pCamera,
                  dc1394video_modes_t pVideoModeList,
                  dc1394video_mode_t &pVideoMode);

  int StartTransmission(dc1394camera_t *pCamera);
  int StopTransmission(dc1394camera_t *pCamera);
/*
  void printCapabilities ( ostream & os , struct __dc1394_feature_info_struct features );
  void printInput ( ostream & os, struct dc1394_input inp );
  void printStandardId ( ostream & os, dc1394_std_id id );
  void printFormatDesc( ostream & os, struct dc1394_fmtdesc const fmtd );
  void printBufferType( ostream & os, enum dc1394_buf_type const type );
  void printPixelFormat4CC( ostream & os, __u32 const pixelformat );
  void printPixelFormat( ostream & os, dc1394_pix_format const pixelformat );
  void printFormat( ostream & os, struct dc1394_format const fmt);
  void printField( ostream & os, dc1394_field const f );
  void printColorSpace( ostream & os, dc1394_colorspace const c );
  void printStreamingParameters ( ostream & os, struct dc1394_streamparm const parm );
  void printCaptureParameters ( ostream & os, struct dc1394_captureparm parm );
  void printStandard ( ostream & os , dc1394_standard const std );
  void printBuffer( ostream & os, struct dc1394_buffer const buffer );
*/
  enum ErrorCode errorCode;
  int fd;
};
#endif /* _V4L2_DEVICE_H_ */
