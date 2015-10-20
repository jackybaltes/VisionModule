/*
** filedevice.h
** 
** Made by Jacky (Hansjorg) Baltes
** Login   <jacky@scalare.cs.umanitoba.ca>
** 
** Started on  Sat Sep 19 15:02:37 2009 Jacky (Hansjorg) Baltes
** Last update Sat Sep 19 15:02:37 2009 Jacky (Hansjorg) Baltes
*/

#ifndef   	FILEDEVICE_H_
# define   	FILEDEVICE_H_

extern "C" {
#include <libavformat/avformat.h>
}

#include "videodevice.h"
#include <inttypes.h>

using namespace std;

// Forward declarations
class FrameBuffer;

struct AVFormatContext;
struct AVCodecContext;
struct AVCodec;
struct AVFrame;
struct AVFrame;
struct AVPacket;
struct SwsContext;

class FileDevice: public VideoDevice
{
public:
  ~FileDevice( );
  FileDevice ( std::string devname, int fps, int width, int height, int depth );

  enum ErrorCode
  {
    OKAY = 0,
    OPEN_FAILURE
    
  };

  enum FileDevice::ErrorCode getErrorCode( void ) const;

  int startCapture( void );
  int stopCapture( void );
  void nextFrame( FrameBuffer * * curFrame );
  int releaseCurrentBuffer( void );
  bool isInterlaced( void );

  int  setControl(struct v4l2_control vc);
  int queryControl(struct v4l2_queryctrl * qc);
  int getControl(struct v4l2_control * vc);

private:
  ErrorCode errorCode;

  int fd;

  PixelFormat format;
  int videoStream;

  AVFormatContext * pFormatCtx;
  AVCodecContext  * pCodecCtx;
  AVCodec         * pCodec;
  AVFrame         * pFrame; 
  AVFrame         * pFrameRGB;
  SwsContext      * sws_ctx;
  uint8_t         * buffer;
};

#endif 	    /* !FILEDEVICE_H_ */
