// $Id$
//

#include <cstdlib>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "framebuffer.h"
#include "framebufferrgb565.h"
#include "framebufferrgb32.h"
#include "framebufferrgb24.h"
#include "linux/videodev2.h"
#include "filedevice.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

FileDevice::FileDevice (string devname, int fps, int width, int height, int depth )
  : VideoDevice( "File", devname, "", "", fps, width, height, depth, 1 ),
    format(PIX_FMT_BGR32),
    videoStream(-1),
    pFormatCtx( nullptr ),
    pCodecCtx( nullptr ),
    pCodec( nullptr ),
    pFrame( nullptr ),
    pFrameRGB( nullptr ),
    sws_ctx( nullptr ),
    buffer( nullptr )
{

#ifdef DEBUG
  std::cout << "Creating file device for file stream " << devname << std::endl;
#endif  

  if ( depth == 32 )
    {
      format = PIX_FMT_BGR32;
    }
  else if ( depth == 24 )
    {
      format = PIX_FMT_BGR24;
    }
  else if ( depth == 16 )
    {
      format = PIX_FMT_RGB565;
    }
  else 
    {
      std::cerr << "Unknown depth " << depth << std::endl;
    }
  
  av_register_all();

  pFormatCtx = avformat_alloc_context();
  
  if ( avformat_open_input( & pFormatCtx, devname.c_str(), 0, nullptr ) != 0 )
    {
      std::cerr << "ERROR: unable to open video file " << devname << std::endl;
      perror(":");
      std::exit(EXIT_FAILURE);
    }

  if( avformat_find_stream_info( pFormatCtx, nullptr ) < 0 )
    {
      std::cerr << "ERROR: unable to find stream information" << std::endl;
      perror(":");
      std::exit(EXIT_FAILURE);
    }

#ifdef DEBUG
  // Dump information about file onto standard error
  av_dump_format(pFormatCtx, 0, devname.c_str(), 0);
#endif

  // Find the first video stream
  videoStream=-1;
  for( unsigned int i=0; i<pFormatCtx->nb_streams; i++ )
    {
      if( pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) 
	{
	  videoStream=i;
	  break;
	}
    }

  if( videoStream == -1 )
    {
      std::cerr << "ERROR: unable to find video stream" << std::endl;
      perror(":");
      std::exit(EXIT_FAILURE);
    }
  
  // Get a pointer to the codec context for the video stream
  pCodecCtx=pFormatCtx->streams[videoStream]->codec;
  
  // Find the decoder for the video stream
  pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
  if ( pCodec == nullptr )
    {
      std::cerr << "ERROR: unsupported codec" << std::endl;
      std::exit(EXIT_FAILURE);
    }

  // Open codec

  if( avcodec_open2( pCodecCtx, pCodec, nullptr ) < 0 )
    {
      std::cerr << "ERROR: unable to open codec" << std::endl;
      std::exit(EXIT_FAILURE);
    }

  errorCode = OKAY;
  error = false;
}

FileDevice::~FileDevice( )
{
  if ( pCodecCtx != 0 )
    {
      // Close the codec
      avcodec_close(pCodecCtx);
      pCodecCtx = nullptr;
    }

  if ( pFormatCtx != 0 )
    {
      // Close the video file
      avformat_close_input( & pFormatCtx);
    }
}

enum FileDevice::ErrorCode FileDevice::getErrorCode( void ) const
{
  return errorCode;
}

int
FileDevice::startCapture (void)
{
  int err = 0;
  int numBytes;

#ifdef DEBUG
  std::cout << __PRETTY_FUNCTION__ << " at (" << __FILE__ << ":" << __LINE__ << ")" << std::endl;
#endif  

  pFrame=av_frame_alloc();
  if( pFrame == nullptr )
    {
      std::cerr << "ERROR: unable to open codec" << std::endl;
      std::exit(EXIT_FAILURE);
    }

  // Allocate an AVFrame structure
  pFrameRGB=av_frame_alloc();
  if( pFrameRGB == nullptr )
    {
      std::cerr << "ERROR: unable to open codec" << std::endl;
      std::exit(EXIT_FAILURE);
    }
  
  // Determine required buffer size and allocate buffer
  numBytes=avpicture_get_size( format, width, height);

  buffer = (uint8_t *) malloc(numBytes);

  // Assign appropriate parts of buffer to image planes in pFrameRGB
  avpicture_fill((AVPicture *)pFrameRGB, buffer, format, width, height);

  // Create the image converter context to format
  sws_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, 
			   pCodecCtx->pix_fmt, 
			   width, height, format, SWS_BICUBIC, 
			   nullptr, nullptr, nullptr);
  if( sws_ctx == nullptr ) 
    {
      fprintf(stderr, "Cannot initialize the conversion context!\n");
      exit(1);
    }

  running = true;
  return err;
}

int
FileDevice::stopCapture( void )
{
  int err = 0;
#ifdef DEBUG
  std::cout << __PRETTY_FUNCTION__ << " at (" << __FILE__ << ":" << __LINE__ << ")" << std::endl;
#endif  

  // Free the RGB image
  free(buffer);
  av_free(pFrameRGB);
  
  // Free the YUV frame
  av_free(pFrame);

  running = false;
  return err;
}

void
FileDevice::nextFrame ( FrameBuffer * * curFramePtr )
{
  int read;
  int frameFinished;
  AVPacket packet;
  static int count = 0;

#ifdef DEBUG
  //  std::cout << __PRETTY_FUNCTION__ << " at (" << __FILE__ << ":" << __LINE__ << ")" << std::endl;
#endif  

  if ( depth == 32 ) 
    { if ( ( * curFramePtr != 0 ) && ( (*curFramePtr)->type() != FrameBuffer::RGB32 ) )
	{
	  delete * curFramePtr;
	  * curFramePtr = 0;
	}
      if ( * curFramePtr == 0 )
	{
	  *curFramePtr = new FrameBufferRGB32();
	}
    }
  else if ( depth == 24 ) 
    {
      if ( ( * curFramePtr != 0 ) && ( (*curFramePtr)->type() != FrameBuffer::RGB24 ) )
	{
	  delete * curFramePtr;
	  * curFramePtr = 0;
	}
      if ( * curFramePtr == 0 )
	{
	  *curFramePtr = new FrameBufferRGB24();
	}
    }
  else if ( depth == 16 )
    {
      if ( ( * curFramePtr != 0 ) && ( (*curFramePtr)->type() != FrameBuffer::RGB565 ) )
	{
	  delete * curFramePtr;
	  * curFramePtr = 0;
	}
      if ( * curFramePtr == 0 )
	{
	  *curFramePtr = new FrameBufferRGB565();
	}
    }
  else
    {
      std::cerr << "Unable to handle depth " << depth << " framebuffer" << std::endl;
    }

  frameFinished = 0;
  while ( ! frameFinished  )
    {
      if ( ( read = av_read_frame( pFormatCtx, & packet ) ) < 0 )
	{
	  // Rewind the stream here
	  if ( av_seek_frame( pFormatCtx, videoStream, 0, AVSEEK_FLAG_BACKWARD ) )
	    {
	      std::cerr << __FILE__ ":" << __LINE__ << " " << __PRETTY_FUNCTION__;
	      std::cerr << " av_seek_frame failed for file " << pFormatCtx->filename << std::endl;
	    }
	  avcodec_flush_buffers( pCodecCtx );
	}
      else
	{
	  // Is this a packet from the video stream?
	  if( packet.stream_index == videoStream ) 
	    {
	      // Decode video frame
	      avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, 
				    & packet);
	      
	      // Did we get a video frame?
	    }
	}
    }

  if ( frameFinished )
    {
      sws_scale(sws_ctx, pFrame->data, 
		pFrame->linesize, 0, 
		pCodecCtx->height, 
		pFrameRGB->data, pFrameRGB->linesize);
 
      (*curFramePtr)->initialize( width, height, (uint8_t *) (pFrameRGB->data[0]) );
      (*curFramePtr)->interlaced = true;
      (*curFramePtr)->absFrameNo = count++;
      (*curFramePtr)->setTimestamp();
#ifdef DEBUG
      //      printf
      //	("FrameBuffer Information: data=%p %dx%d, bpp = %d, bpl=%d, fieldNo=%d absFrameNo=%d, interlaced=%d\n",
      //	 (*curFramePtr)->buffer, (*curFramePtr)->width, (*curFramePtr)->height,
      //	 (*curFramePtr)->bytesPerPixel, (*curFramePtr)->bytesPerLine,
      //	 (*curFramePtr)->fieldNo, (*curFramePtr)->absFrameNo, (*curFramePtr)->interlaced);
#endif
    }
  else
    {
      std::cerr << __FILE__ ":" << __LINE__ << " " << __PRETTY_FUNCTION__;
      std::cerr << " unable to find finished frame" << std::endl;
    }
}

int
FileDevice::releaseCurrentBuffer (void)
{
  int err = 0;

  return err;
}

bool FileDevice::isInterlaced( void )
{
  return 0;
}

int FileDevice::setControl(struct v4l2_control vc)
{
  return ioctl (fd, VIDIOC_S_CTRL, &vc);
}

int FileDevice::getControl(struct v4l2_control * vc)
{
  return ioctl (fd, VIDIOC_G_CTRL, vc);
}

int FileDevice::queryControl(struct v4l2_queryctrl * qc)
{
  return ioctl (fd, VIDIOC_QUERYCTRL, qc);
}



