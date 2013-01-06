// $Id: v4l2device.cpp,v 1.2.2.8 2004/12/20 02:38:58 cvs Exp $
//

#include <cstring>
#include <cstdlib>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <libv4l2.h>

#include "framebuffer.h"
#include "framebufferrgb565.h"
#include "framebufferrgb32.h"
#include "framebufferrgb24be.h"
#include "framebufferbayer.h"
#include "linux/videodev.h"
#include "v4l2device.h"

V4L2Device::V4L2Device (string devname, string inputName, string standardName, unsigned int fps, unsigned int width, unsigned int height, unsigned int depth, unsigned int numBuffers )
  : VideoDevice( "V4L2", devname, inputName, standardName, fps, width, height, depth, numBuffers )
{
  struct v4l2_capability cap;
  v4l2_std_id std_id = V4L2_STD_NTSC;
  int err;

#if 0
  FILE * fp;

  if ( ( fp = fopen("/tmp/v4l2log.txt","w+") ) != NULL )
    {
      v4l2_log_file = fp;
    }
  else
    {
      std::cout << "Logging failed" << std::endl;
    }
#endif

  if ( ( fd = v4l2_open (devname.c_str(), O_RDWR) ) < 0 )
    {
      std::cerr << "ERROR: V4L2Device unable to open device " << devname;
      perror (":");
      errorCode = OPEN_FAILURE;
      error = true;
      return;
    }
#ifdef DEBUG
  std::cout << "V4L2Device File Descriptor: " << fd << std::endl;
#endif

  if ( ( err = v4l2_ioctl (fd, VIDIOC_QUERYCAP, & cap) ) != 0 )
    {
      std::cerr << "WARNING: V4L2Device unable to query capabilities " << devname;
      std::cerr << "\nAttempting to continue regardless.";
      perror (":");
      errorCode = QUERYCAP_FAILURE;
      //  error = true;
      //  return;
    }
  else 
    {
#ifdef DEBUG
      printCapabilities (std::cout, cap);
#endif
  
      if ( ! (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE ) )
	{
	  std::cerr << "ERROR: V4L2Device no video capture suppport " << devname;
	  perror (":");
	  errorCode = NO_CAPTURE_FAILURE;
	  error = true;
	  return;
	}
      
      if ( ! (cap.capabilities & V4L2_CAP_STREAMING ) )
	{
	  std::cerr << "ERROR: V4L2Device no streaming support " << devname;
	  perror (":");
	  errorCode = NO_STREAMING_FAILURE;
	  error = true;
	  return;
	}
      
    }  

  if ( inputName != "auto" )
    {
      int inpIndex = -1;
      for (int i = 0, errx = 0; errx == 0; i++)
	{
	  struct v4l2_input inp;
	  inp.index = i;
	  err = v4l2_ioctl (fd, VIDIOC_ENUMINPUT, &inp);
	  
	  if (!err)
	    {
#ifdef DEBUG
	      printInput ( cout, inp);
#endif
	      if (string( (char const *) & inp.name ) == inputName )
		{
		  inpIndex = inp.index;
		  break;
		}
	    }
	  else
	    {
	      errx = 1;
	    }
	}

      if (inpIndex < 0)
	{
	  std::cerr << "WARNING: V4L2Device unable to find input named " << inputName << "\n";
	  std::cerr << "Attempting to continue.\n";
	  errorCode = NO_INPUT_FAILURE;
	  // error = true;
	  // return;
	}
      
      if ( 0 != ( err = v4l2_ioctl (fd, VIDIOC_S_INPUT, &inpIndex) ) )
	{
	  cerr << "WARNING: S_INPUT returned error " << err << endl;
	  std::cerr << "Attempting to continue.\n";
	  errorCode = SET_INPUT_FAILURE;
	  //error = true;
	  //return;
	}
      
#ifdef DEBUG
      cout << "V4L2Device:: setting Input " << inputName << "=" << inpIndex << " okay\n";
#endif
    }

  if ( standardName != "auto" )
    {
      int fieldsPerSecond;
      
      if ( standardName == "PAL" )
	{
	  std_id = V4L2_STD_PAL;
	  fieldsPerSecond = 25;
	}
      else if ( standardName == "NTSC" )
	{
	  std_id = V4L2_STD_NTSC;
	  fieldsPerSecond = 30;
	}
      else if ( standardName == "SECAM" )
	{
	  std_id = V4L2_STD_SECAM;
	}
      else
	{
	  cerr << "ERROR: V4L2Device unknown standard " << standardName << endl;
	  errorCode = UNKNWON_STANDARD_FAILURE;
	  //      error = true;
	  //return;
	}
      
#ifdef DEBUG
      cout << "Setting standard to " << std_id << endl;
#endif
      
      if (  -1 == v4l2_ioctl (fd, VIDIOC_S_STD, &std_id ) ) 
	{
	  cerr << "ERROR: V4L2Device VIDIOC_S_STD";
	  perror (":");
	  errorCode = SET_STANDARD_FAILURE;
	  //error = true;
	  //return;
	}
    }

#ifdef DEBUG
  std_id = 0;
  if ( 0 != v4l2_ioctl (fd, VIDIOC_G_STD, & std_id ) ) 
    {
      cerr << "ERROR: V4L2Device VIDIOC_G_STD";
      perror (":");
      //std::exit (GET_STANDARD_FAILURE);

      cout << "V4L2Device: succeeded in setting standard" << endl;
      printStandardId( cout, std_id );
    }
#endif

      
#ifdef DEBUG
  cout << "Video capture image formats supported: ";
  for (int i = 0, err = 0; err == 0; ++i)
    {
      struct v4l2_fmtdesc fmtd;
      fmtd.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      fmtd.index = i;
      err = v4l2_ioctl( fd, VIDIOC_ENUM_FMT, &fmtd );
      if (err == 0)
	{
	  printFormatDesc( cout, fmtd );
	}
      else
	{
	  break;
	}
    }
#endif

  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if ( ( err = v4l2_ioctl (fd, VIDIOC_G_FMT, &fmt) ) != 0 )
    {
      cerr << "ERROR: V4L2Device VIDIOC_G_FMT failed with error code " << err << endl;
      perror( ":" );
      errorCode = GET_FORMAT_FAILURE;
      error = true;
      goto _exit;
      //return;
    }

  //fmt.fmt.pix.field = V4L2_FIELD_ALTERNATE;
  fmt.fmt.pix.field = V4L2_FIELD_NONE;
  //fmt.fmt.pix.field = V4L2_FIELD_TOP;

  fmt.fmt.pix.width = width;
  fmt.fmt.pix.height = height;

  fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
  err = v4l2_ioctl (fd, VIDIOC_S_FMT, &fmt);
  if (err)
    {
      cerr << "ERROR: V4L2_Device S_FMT returned error " << err;
      perror( ";" );
      errorCode = SET_FORMAT_FAILURE;
      error = true;
      goto _exit;
    }

  err = v4l2_ioctl (fd, VIDIOC_G_FMT, &fmt);
  if (err)
    {
      cerr << "ERROR: V4L2_Device G_FMT returned error " << err;
      perror( ":" );
      errorCode = GET_FORMAT_FAILURE;
      goto _exit;
      //return;
    } 

  std::cout << "========== Selected pixel and capture format ===========" << std::endl;
  printFormat( cout, fmt);

  struct v4l2_streamparm parm;
  parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  err = v4l2_ioctl (fd, VIDIOC_G_PARM, &parm);
  if (err)
    {
      fprintf (stderr, "V4L2Device: G_PARM returned error %d\n", err);
      perror("V4L2Device G_PARM:");
      errorCode = GET_PARAMETER_FAILURE;
      error = true;
      goto _exit;
      //return;
    }

  printStreamingParameters( cout, parm );
  cout << endl;

#if 0
  parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  parm.parm.capture.timeperframe.numerator = 100000; 
  parm.parm.capture.timeperframe.denominator = fieldsPerSecond; 

  err = v4l2_ioctl (fd, VIDIOC_S_PARM, &parm);
  if (err)
    {
      cerr << "ERROR: V4L2_Device S_PARM returned error " << err;
      perror( ":" );
      errorCode = SET_PARAMETER_FAILURE;
      goto _exit;
      //return;
    }
#endif

  struct v4l2_requestbuffers req;  
  req.count = numBuffers; 
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;

  err = v4l2_ioctl (fd, VIDIOC_REQBUFS, &req);
  if (err < 0 || req.count < 2)
    {
      cerr << "ERROR: V4L2Device REQBUFS returned error " << err << ", count " << req.count << endl;
      perror(":");
      errorCode = REQUEST_BUFFER_FAILURE;
      error = true;
      goto _exit;
      //return;
    }

  for (unsigned int i = 0; i < req.count; ++i)
    {
      videobuffer[i].vidbuf.index = i;
      videobuffer[i].vidbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      err = v4l2_ioctl (fd, VIDIOC_QUERYBUF, &videobuffer[i].vidbuf);
      if (err < 0)
	{
	  cerr << "ERROR: V4L2Device QUERYBUF failed for buffer " << i << endl;
	  perror(":");
	  errorCode = QUERY_BUFFER_FAILURE;
	  error = true;
	  goto _exit;
	  //return;
	}

      videobuffer[i].data = v4l2_mmap (0, videobuffer[i].vidbuf.length,
				       PROT_READ | PROT_WRITE,
				       MAP_SHARED, fd,
				       videobuffer[i].vidbuf.m.offset);
      if ( videobuffer[i].data == (void *) -1 )
	{
	  cerr << "ERROR: V4L2Device mmap failed for buffer " << videobuffer[i].data << endl;
	  perror(":");
	  errorCode = MMAP_BUFFER_FAILURE;
	  error = true;
	  goto _exit;
	  //return;
	}
#ifdef DEBUG
      else
	{
	  cout << "V4L2Device mmap() added buffer at address " << videobuffer[i].data << endl;
	}
#endif
    }

  for (unsigned int i = 0; i < req.count; ++i)
    {
      if ((err = v4l2_ioctl (fd, VIDIOC_QBUF, &videobuffer[i].vidbuf)))
	{
	  cerr << "ERROR: V4L2Device QBUF failed for buffer " << i << endl;
	  perror(":");
	  errorCode = QBUF_BUFFER_FAILURE;
	  error = true;
	  goto _exit;
	  //return;
	}
    }

  error = false;
  errorCode = OKAY;

 _exit:
  return;
}

V4L2Device::~V4L2Device( )
{
  int err;

#ifdef DEBUG
  std::cerr << __FILE__ << __LINE__ << ":" << __PRETTY_FUNCTION__ << std::endl;
#endif

  if ( isRunning() )
    {
      stopCapture( );
    }

  for (unsigned int i = 0; i < numBuffers; ++i)
    {
      if ( videobuffer[i].data != (void *) -1 )
	{
	  err = v4l2_munmap (videobuffer[i].data, videobuffer[i].vidbuf.length );
	  if ( err != 0 )
	    {
	      cerr << __PRETTY_FUNCTION__ << " " << "ERROR: V4L2Device unmap failed for buffer " << videobuffer[i].data << endl;	      
	    }
	}
    }

  if ( ( err = v4l2_close( fd ) ) != 0 )
    {
      cerr << "ERROR: V4L2Device close failed, ";
      perror( ":" );
    }
}

void
V4L2Device::printCapabilities ( ostream & os , struct v4l2_capability const cap)
{
  os << endl << ">>> Capabilities" << endl;
  os << "Device " << cap.driver << ", card " << cap.card << ", on bus " << cap.bus_info << ", version " << cap.version;
  os << ", capabilities: ";

  if ( cap.capabilities & V4L2_CAP_VIDEO_CAPTURE ) 
    {
      os << "video capture,";
    }
  else
    {
      os << "no video capture,";
    }

  if ( cap.capabilities & V4L2_CAP_VIDEO_OUTPUT ) 
    {
      os << "video output,";
    }
  else
    {
      os << "no video output,";
    }

  if ( cap.capabilities & V4L2_CAP_VIDEO_OVERLAY ) 
    {
      os << "video overlay,";
    }
  else
    {
      os << "no video overlay,";
    }

  if ( cap.capabilities & V4L2_CAP_VBI_CAPTURE ) 
    {
      os << "vbi capture,";
    }
  else
    {
      os << "no vbi capture,";
    }

  if ( cap.capabilities & V4L2_CAP_VBI_OUTPUT ) 
    {
      os << "vbi output,";
    }
  else
    {
      os << "no vbi output,";
    }

  if ( cap.capabilities & V4L2_CAP_RDS_CAPTURE ) 
    {
      os << "rds capture,";
    }
  else
    {
      os << "no rds capture,";
    }

  if ( cap.capabilities & V4L2_CAP_TUNER ) 
    {
      os << "tuner,";
    }
  else
    {
      os << "no tuner,";
    }

  if ( cap.capabilities & V4L2_CAP_READWRITE ) 
    {
      os << "read/write,";
    }
  else
    {
      os << "no read/write,";
    }

  if ( cap.capabilities & V4L2_CAP_ASYNCIO ) 
    {
      os << "async io,";
    }
  else
    {
      os << "no async io,";
    }

  if ( cap.capabilities & V4L2_CAP_STREAMING ) 
    {
      os << "streaming";
    }
  else
    {
      os << "no streaming";
    }
  os << endl << "<<< Capabilities" << endl;
}

void
V4L2Device::printInput ( ostream & os, struct v4l2_input inp)
{
  os << endl << ">>> Input" << endl;
  os << "Input " << inp.index << ", name " << inp.name;
  
  if ( inp.type == V4L2_INPUT_TYPE_TUNER )
    {
      os << ", type tuner ";
    }
  else if ( inp.type == V4L2_INPUT_TYPE_CAMERA )
    {
      os << ", type camera ";
    }
  else
    {
      os << ", type unknown ";
    }
    
  os << ", audioset " << inp.audioset;
  os << ", tuner " << inp.tuner << ' ';
  printStandardId( os, inp.std );
  os << ", status ";
  if ( inp.status & V4L2_IN_ST_NO_POWER )
    {
      os << "no power,";
    }
  else
    {
      os << "power,";
    }

  if ( inp.status & V4L2_IN_ST_NO_SIGNAL )
    {
      os << "no signal,";
    }
  else
    {
      os << "signal,";
    }
  if ( inp.status & V4L2_IN_ST_NO_COLOR )
    {
      os << "no color,";
    }
  else
    {
      os << "color,";
    }
  if ( inp.status & V4L2_IN_ST_NO_H_LOCK )
    {
      os << "no H lock,";
    }
  else
    {
      os << "H lock,";
    }
  if ( inp.status & V4L2_IN_ST_COLOR_KILL )
    {
      os << "color kill,";
    }
  else
    {
      os << "no color kill,";
    }
  if ( inp.status & V4L2_IN_ST_NO_SYNC )
    {
      os << "no sync,";
    }
  else
    {
      os << "sync,";
    }
  if ( inp.status & V4L2_IN_ST_NO_EQU )
    {
      os << "no equalizer,";
    }
  else
    {
      os << "equalizer,";
    }
  if ( inp.status & V4L2_IN_ST_NO_CARRIER )
    {
      os << "no carrier,";
    }
  else
    {
      os << "carrier,";
    }
  if ( inp.status & V4L2_IN_ST_MACROVISION )
    {
      os << "macrovision,";
    }
  else
    {
      os << "no macrovision,";
    }
  if ( inp.status & V4L2_IN_ST_NO_ACCESS )
    {
      os << "no access,";
    }
  else
    {
      os << "access,";
    }
  if ( inp.status & V4L2_IN_ST_VTR )
    {
      os << "vtr time constant,";
    }
  else
    {
      os << "no vtr time constant,";
    }
  os << endl << "<<< Input" << endl;
}

void
V4L2Device::printStandardId ( ostream & os , v4l2_std_id const id )
{
  os << endl << ">>> Standard ID" << endl;
  if ( id & V4L2_STD_PAL_B )
    {
      os << "PAL B,";
    }
  else
    {
      os << "no PAL B,";
    }
  if ( id & V4L2_STD_PAL_B1 )
    {
      os << "PAL B1,";
    }
  else
    {
      os << "no PAL B1,";
    }
  if ( id & V4L2_STD_PAL_G )
    {
      os << "PAL G,";
    }
  else
    {
      os << "no PAL G,";
    }
  if ( id & V4L2_STD_PAL_H )
    {
      os << "PAL H,";
    }
  else
    {
      os << "no PAL H,";
    }
  if ( id & V4L2_STD_PAL_I )
    {
      os << "PAL I,";
    }
  else
    {
      os << "no PAL I,";
    }
  if ( id & V4L2_STD_PAL_D )
    {
      os << "PAL D,";
    }
  else
    {
      os << "no PAL D,";
    }
  if ( id & V4L2_STD_PAL_D1 )
    {
      os << "PAL D1,";
    }
  else
    {
      os << "no PAL D1,";
    }
  if ( id & V4L2_STD_PAL_K )
    {
      os << "PAL K,";
    }
  else
    {
      os << "no PAL K,";
    }
  if ( id & V4L2_STD_PAL_M )
    {
      os << "PAL M,";
    }
  else
    {
      os << "no PAL M,";
    }
  if ( id & V4L2_STD_PAL_N )
    {
      os << "PAL N,";
    }
  else
    {
      os << "no PAL N,";
    }
  if ( id & V4L2_STD_PAL_Nc )
    {
      os << "PAL Nc,";
    }
  else
    {
      os << "no PAL Nc,";
    }
  if ( id & V4L2_STD_PAL_60 )
    {
      os << "PAL 60,";
    }
  else
    {
      os << "no PAL 60,";
    }
  if ( id & V4L2_STD_NTSC_M )
    {
      os << "NTSC M,";
    }
  else
    {
      os << "no NTSC M,";
    }
  if ( id & V4L2_STD_NTSC_M_JP )
    {
      os << "NTSC M JP,";
    }
  else
    {
      os << "no NTSC M JP,";
    }
  if ( id & V4L2_STD_SECAM_B )
    {
      os << "SECAM B,";
    }
  else
    {
      os << "no SECAM B,";
    }
  if ( id & V4L2_STD_SECAM_D )
    {
      os << "SECAM D,";
    }
  else
    {
      os << "no SECAM D,";
    }
  if ( id & V4L2_STD_SECAM_H )
    {
      os << "SECAM H,";
    }
  else
    {
      os << "no SECAM H,";
    }
  if ( id & V4L2_STD_SECAM_K )
    {
      os << "SECAM K,";
    }
  else
    {
      os << "no SECAM K,";
    }
  if ( id & V4L2_STD_SECAM_K1 )
    {
      os << "SECAM K1,";
    }
  else
    {
      os << "no SECAM K1,";
    }
  if ( id & V4L2_STD_SECAM_L )
    {
      os << "SECAM L,";
    }
  else
    {
      os << "no SECAM L,";
    }
  if ( id & V4L2_STD_ATSC_8_VSB )
    {
      os << "ATSC 8 VSB,";
    }
  else
    {
      os << "no ATSC 8 VSB,";
    }
  if ( id & V4L2_STD_ATSC_16_VSB )
    {
      os << "ATSC 16 VSB,";
    }
  else
    {
      os << "no ATSC 16 VSB,";
    }
  os << endl << "<<< Standard ID" << endl;
}

void
V4L2Device::printStandard ( ostream & os , v4l2_standard const std )
{
  os << endl << ">>> Standard" << endl;
  os << "index " << std.index;
  printStandardId( os, std.id );
  os << "name " << std.name;
  // os << "frameperiod " << ***;
  os << ", framelines " << std.framelines;
  os << endl << "<<< Standard" << endl;
}

void 
V4L2Device::printFormatDesc( ostream & os, struct v4l2_fmtdesc const fmtd)
{
  os << endl << ">>> Format Descriptor" << endl;
  os << "index " << fmtd.index;
  printBufferType( os, fmtd.type );
  if ( fmtd.flags & V4L2_FMT_FLAG_COMPRESSED )
    {
      os << "compressed,";
    }
  else
    {
      os << "uncompressed,";
    }
  printPixelFormat4CC( os, fmtd.pixelformat );
  os << "description " << fmtd.description;
  os << endl << "<<< Format Description" << endl;
}

void 
V4L2Device::printFormat( ostream & os, struct v4l2_format const fmt)
{
  os << endl << ">>> Format" << endl;
  printBufferType( os, fmt.type );
  if ( fmt.type == V4L2_BUF_TYPE_VIDEO_CAPTURE )
    {
      printPixelFormat( os, fmt.fmt.pix );
    }
  else
    {
      os << "unknown format" << endl;
    }
  os << endl << "<<< Format" << endl;
}

void 
V4L2Device::printBufferType( ostream & os, enum v4l2_buf_type const type )
{
  os << endl << ">>> Buffer Type" << endl;
  switch( type )
    {
    case V4L2_BUF_TYPE_VIDEO_CAPTURE:
      os << "video capture";
      break;
    case V4L2_BUF_TYPE_VIDEO_OUTPUT:
      os << "video output";
      break;
    case V4L2_BUF_TYPE_VIDEO_OVERLAY:
      os << "video overlay";
      break;
    case V4L2_BUF_TYPE_VBI_CAPTURE:
      os << "vbi capture";
      break;
    case V4L2_BUF_TYPE_VBI_OUTPUT:
      os << "vbi output";
      break;
    case V4L2_BUF_TYPE_PRIVATE:
      os << "private";
      break;
    default:
      os << "unknown " << type;
      break;
    }
  os << endl << "<<< Buffer Type" << endl;
}

void 
V4L2Device::printPixelFormat4CC( ostream & os, __u32 const pixelformat )
{
  os << endl << ">>> Pixel Format 4CC" << endl;
  char c1 = ( pixelformat & 0x000000ff ) >> 0 ;
  char c2 = ( pixelformat & 0x0000ff00 ) >> 8;
  char c3 = ( pixelformat & 0x00ff0000 ) >> 16;
  char c4 = ( pixelformat & 0xff000000 ) >> 24;

  os << c1 << c2 << c3 << c4;
  os << endl << "<<< Pixel Format 4CC" << endl;
}

void 
V4L2Device::printPixelFormat( ostream & os, v4l2_pix_format const pixelformat )
{
  os << endl << ">>> Pixel Format" << endl;
  os << width << "x" << height;
  printPixelFormat4CC( os, pixelformat.pixelformat );
  printField( os, pixelformat.field );
  os << "bytesperline " << pixelformat.bytesperline << ",sizeimage " << pixelformat.sizeimage;
  printColorSpace( os, pixelformat.colorspace );
  os << endl << "<<< Pixel Format" << endl;
}

void 
V4L2Device::printColorSpace( ostream & os, v4l2_colorspace const c )
{
  os << endl << ">>> Colorspace" << endl;
  switch( c )
    {
    case V4L2_COLORSPACE_SMPTE170M:
      os << "SMPTE170M";
      break;
    case V4L2_COLORSPACE_SMPTE240M:
      os << "SMPTE240M";
      break;
    case V4L2_COLORSPACE_REC709:
      os << "REC709";
      break;
    case V4L2_COLORSPACE_BT878:
      os << "BT878";
      break;
    case V4L2_COLORSPACE_470_SYSTEM_M:
      os << "470 System M";
      break;
    case V4L2_COLORSPACE_470_SYSTEM_BG:
      os << "470 System BG";
      break;
    case V4L2_COLORSPACE_JPEG:
      os << "JPEG";
      break;
    case V4L2_COLORSPACE_SRGB:
      os << "SRGB";
      break;
    default:
      os << "unknown";
      break;
    }
  os << endl << "<<< Colorspace" << endl;
}

void 
V4L2Device::printField( ostream & os, enum v4l2_field f )
{
  os << endl << ">>> Field" << endl;
  switch( f )
    {
    case V4L2_FIELD_ANY:
      os << "any field";
      break;
    case V4L2_FIELD_NONE:
      os << "no field";
      break;
    case V4L2_FIELD_TOP:
      os << "top field";
      break;
    case V4L2_FIELD_BOTTOM:
      os << "bottom field";
      break;
    case V4L2_FIELD_INTERLACED:
      os << "interlaced field";
      break;
    case V4L2_FIELD_SEQ_TB:
      os << "sequential top bottom field";
      break;
    case V4L2_FIELD_SEQ_BT:
      os << "sequential bottom top field";
      break;
    case V4L2_FIELD_ALTERNATE:
      os << "alternate fields in separate buffers";
      break;
    default:
      os << "unknown";
      break;
    }
  os << endl << "<<< Field" << endl;
}

void
V4L2Device::printStreamingParameters ( ostream & os, struct v4l2_streamparm const parm )
{
  os << endl << ">>> Streaming parameters" << endl;
  printBufferType( os, parm.type );
  switch ( parm.type ) 
    {
    case V4L2_BUF_TYPE_VIDEO_CAPTURE:
      printCaptureParameters( os, parm.parm.capture );
      break;
    case V4L2_BUF_TYPE_VIDEO_OUTPUT:
      os << "No more information for video output parameters\n";
      break;
    case V4L2_BUF_TYPE_VIDEO_OVERLAY:
      os << "No more information for video overlay parameters\n";
      break;
    case V4L2_BUF_TYPE_VBI_CAPTURE:
      os << "No more information for vbi capture parameters\n";
      break;
    case V4L2_BUF_TYPE_VBI_OUTPUT:
      os << "No more information for vbi output parameters\n";
      break;
    case V4L2_BUF_TYPE_PRIVATE:
      os << "No more information for private parameters\n";
      break;
    default:
      os << "No more information for unknown parameters\n";
      break;
    }
  os << endl << "<<< Streaming parameters" << endl;
}

void
V4L2Device::printCaptureParameters ( ostream & os, struct v4l2_captureparm parm )
{
  os << endl << "<<< Capture parameters" << endl;
  if ( parm.capability & V4L2_MODE_HIGHQUALITY ) 
    {
      os << " high quality supported";
    }
  if ( parm.capability & V4L2_CAP_TIMEPERFRAME ) 
    {
      os << " time per frame supported";
    }
  if ( parm.capturemode & V4L2_MODE_HIGHQUALITY ) 
    {
      os << " high quality mode selected";
    }
  if ( parm.capturemode & V4L2_CAP_TIMEPERFRAME ) 
    {
      os << " time per frame selected";
    }
  os << endl << "<<< Capture parameters" << endl;
}

enum V4L2Device::ErrorCode V4L2Device::getErrorCode( void ) const
{
  return errorCode;
}

int
V4L2Device::startCapture (void)
{
  int err;

  int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  err = v4l2_ioctl (fd, VIDIOC_STREAMON, &type);
  if (err)
    {
      cerr << "ERROR: V4L2Device::startCapture() VIDIOC_STREAMON failed";
      perror( ":" );
      errorCode = VIDIOC_STREAMOFF_FAILURE;
    }
  running = true;
  return err;
}

int
V4L2Device::stopCapture( void )
{
  int err;

  int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  err = v4l2_ioctl (fd, VIDIOC_STREAMOFF, &type);
  if (err)
    {
      cerr << "ERROR: V4L2Device::stopCapture() VIDIOC_STREAMOFF failed";
      perror( ":" );
      errorCode = VIDIOC_STREAMOFF_FAILURE;
    }
  running = false;
  return err;
}

void
V4L2Device::nextFrame ( FrameBuffer * * curFramePtr )
{
  tempbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  tempbuf.memory = V4L2_MEMORY_MMAP;
 
  while( v4l2_ioctl (fd, VIDIOC_DQBUF, &tempbuf) == -1)
    {
      perror ("DQBUF in nextFrame");
      std::cerr << "errno: " << errno << std::endl;
      std::cerr << "ESPIPE: " << ESPIPE << std::endl;
      if ( ( errno != EINTR ) && ( errno != ESPIPE ) && ( errno != EIO ) )
	{
	  std::exit ( EXIT_FAILURE );
	}
    }
  int index = tempbuf.index;
      
  if ( fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_RGB565 )
    {
      if ( ( * curFramePtr != 0 ) && ( (*curFramePtr)->type() != FrameBuffer::RGB565) )
	{
	  delete *curFramePtr;
	  *curFramePtr = 0;
	}
      if ( * curFramePtr == 0 )
	{
	  *curFramePtr = new FrameBufferRGB565();
	}
    }
  else if ( fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_RGB32 )
    {
      if ( ( * curFramePtr != 0 ) && ( (*curFramePtr)->type() != FrameBuffer::RGB32) )
	{
	  delete *curFramePtr;
	  *curFramePtr = 0;
	}
      if ( * curFramePtr == 0 )
	{
	  *curFramePtr = new FrameBufferRGB32();
	}
    }
  else if ( fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_RGB24 )
    {
      if ( ( * curFramePtr != 0 ) && ( (*curFramePtr)->type() != FrameBuffer::RGB24BE) )
	{
	  delete *curFramePtr;
	  *curFramePtr = 0;
	}
      if ( * curFramePtr == 0 )
	{
	  *curFramePtr = new FrameBufferRGB24BE();
	}
    }
  else if ( fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_SGBRG8 )
    {
      if ( ( * curFramePtr != 0 ) && ( (*curFramePtr)->type() != FrameBuffer::BAYER) )
	{
	  delete *curFramePtr;
	  *curFramePtr = 0;
	}
      if ( * curFramePtr == 0 )
	{
	  *curFramePtr = new FrameBufferBayer();
	}
    }
  else
    {
      cerr << __PRETTY_FUNCTION__ << "(" << __FILE__ << ":" << __LINE__ << ")" 
	   << "ERROR: unknown pixel format" 
	   << fmt.fmt.pix.pixelformat << endl;
    }
  
  (*curFramePtr)->initialize( fmt.fmt.pix.width, fmt.fmt.pix.height, (uint8_t *) (videobuffer[index].data) );

  if ( tempbuf.field == V4L2_FIELD_INTERLACED )
    {
      (*curFramePtr)->interlaced = true;
      (*curFramePtr)->fieldNo = 2;
    }
  else
    {
      (*curFramePtr)->interlaced = false;
      (*curFramePtr)->fieldNo = tempbuf.field;
    }

  (*curFramePtr)->absFrameNo = tempbuf.sequence;
  (*curFramePtr)->setTimestamp();

#if defined(XX_DEBUG)
  printf
	("FrameBuffer Information: data=%p %dx%d, bpp = %d, bpl=%d, fieldNo=%d absFrameNo=%d, interlaced=%d\n",
	 (*curFramePtr)->buffer, (*curFramePtr)->width, (*curFramePtr)->height,
	 (*curFramePtr)->bytesPerPixel, (*curFramePtr)->bytesPerLine,
	 (*curFramePtr)->fieldNo, (*curFramePtr)->absFrameNo, (*curFramePtr)->interlaced);
#endif
}


int
V4L2Device::releaseCurrentBuffer (void)
{
  int err;
  err = v4l2_ioctl (fd, VIDIOC_QBUF, &tempbuf);
  if (err)
    {
      cerr << "ERROR: V4L2Device::stopCapture() VIDIOC_QBUF failed";
      perror( ":" );
      errorCode = VIDIOC_QBUF_FAILURE;
    }
  return err;
}

void V4L2Device::printBuffer( ostream & os, struct v4l2_buffer const buffer )
{
  os << endl << ">>> Buffer" << endl;
  printBufferType( os, buffer.type );
  os << "index " << buffer.index << ", bytesused " << buffer.bytesused;
  os << "flags " << buffer.field << ", field " << buffer.field;
  os << endl << "<<< Buffer" << endl;
}

bool V4L2Device::isInterlaced( void )
{
  return ( fmt.fmt.pix.field != V4L2_FIELD_ALTERNATE );
}

int 
V4L2Device::setControl(struct v4l2_control * vc)
{
#ifdef DEBUG
  std::cout << "V4L2Device::setControl() called\n";
  std::cout << "file descriptor: " << fd << std::endl;
  std::cout << "control id: " << vc->id << std::endl;
#endif
  int ret = -1;
  struct v4l2_queryctrl queryctrl;

  memset (&queryctrl, 0, sizeof (queryctrl));
  queryctrl.id = vc->id;

  if ( -1 == v4l2_ioctl( fd,  VIDIOC_QUERYCTRL, &queryctrl)) 
    {
      if (errno != EINVAL) 
	{ 
	  ret = -1;
	  goto exit;
        } 
      else 
	{
	  std::cerr << vc->id << " video control is not supported" << std::endl;
	  ret = -1;
	  goto exit;
        }
      
    } 
  else if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) 
    {
	  std::cerr << vc->id << " video control is disabled" << std::endl;
	  ret = -1;
	  goto exit;
    } 
  else 
    {
      if ( vc->value == -1 ) 
	{
	  vc->value = queryctrl.default_value;
	}

      if ( ( ret = v4l2_ioctl (fd, VIDIOC_S_CTRL, vc) ) < 0 )
	{
	  std::cerr << "VIDIOC_S_CTRL of " << vc->id << " to value " << vc->value << " failed with error code " << ret;
	  perror ("VIDIOC_S_CTRL");
	}
    }
 exit:
  return ret;
}

int 
V4L2Device::getControl(struct v4l2_control * vc)
{
  return v4l2_ioctl (fd, VIDIOC_G_CTRL, vc);
}

int 
V4L2Device::queryControl(struct v4l2_queryctrl * qc)
{
#ifdef DEBUG
  std::cout << "V4L2Device::queryControl() called\n";
  std::cout << "file descriptor: " << fd << std::endl;
#endif
  return v4l2_ioctl (fd, VIDIOC_QUERYCTRL, qc);
}

int
V4L2Device::SetBrightness( unsigned int val )
{
  struct v4l2_control vc;
  memset( &vc, 0, sizeof(vc) );

  vc.id = V4L2_CID_BRIGHTNESS;
  if ( val > 0 )
    {
      vc.value = val;
    }
  return setControl( & vc );
}

int
V4L2Device::GetBrightness( void )
{
  struct v4l2_control vc;
  memset( &vc, 0, sizeof(vc) );

  vc.id = V4L2_CID_BRIGHTNESS;
  unsigned int ret = getControl( & vc );  
  unsigned int result = -1;
  if ( ret == 0 )
    {
      result = vc.value;
    }
  else
    {
      std::cerr << "getcontrol failed" << std::endl;
      result = -1;
    }
  return result;
}

int
V4L2Device::SetContrast( unsigned int val )
{
  struct v4l2_control vc;
  memset( &vc, 0, sizeof(vc) );

  vc.id = V4L2_CID_CONTRAST;
  if ( val > 0 )
    {
      vc.value = val;
    }
  return setControl( & vc );
}

int
V4L2Device::GetContrast( void )
{
  struct v4l2_control vc;
  memset( &vc, 0, sizeof(vc) );

  vc.id = V4L2_CID_CONTRAST;
  unsigned int ret = getControl( & vc );  
  unsigned int result = -1;
  if ( ret == 0 )
    {
      result = vc.value;
    }
  else
    {
      std::cerr << "getcontrast failed" << std::endl;
      result = -1;
    }
  return result;
}

int
V4L2Device::SetSaturation( unsigned int val )
{
  struct v4l2_control vc;
  memset( &vc, 0, sizeof(vc) );

  vc.id = V4L2_CID_SATURATION;
  if ( val > 0 )
    {
      vc.value = val;
    }
  return setControl( & vc );
}

int
V4L2Device::GetSaturation( void )
{
  struct v4l2_control vc;
  memset( &vc, 0, sizeof(vc) );

  vc.id = V4L2_CID_SATURATION;
  unsigned int ret = getControl( & vc );  
  unsigned int result = -1;
  if ( ret == 0 )
    {
      result = vc.value;
    }
  else
    {
      std::cerr << "getsaturation failed" << std::endl;
      result = -1;
    }
  return result;
}

int
V4L2Device::SetSharpness( unsigned int val )
{
  struct v4l2_control vc;
  memset( &vc, 0, sizeof(vc) );

  vc.id = V4L2_CID_SHARPNESS;
  if ( val > 0 )
    {
      vc.value = val;
    }
  return setControl( & vc );
}

int
V4L2Device::GetSharpness( void )
{
  struct v4l2_control vc;
  memset( &vc, 0, sizeof(vc) );

  vc.id = V4L2_CID_SHARPNESS;
  unsigned int ret = getControl( & vc );  
  unsigned int result = -1;
  if ( ret == 0 )
    {
      result = vc.value;
    }
  else
    {
      std::cerr << "getsharpness failed" << std::endl;
      result = -1;
    }
  return result;
}



