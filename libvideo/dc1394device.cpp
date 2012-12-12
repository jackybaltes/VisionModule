
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <cstdlib>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "dc1394device.h"
#include "framebuffer.h"
#include "framebufferrgb24be.h"
#include "framebufferbayer.h"

#include <dc1394/dc1394.h>

DC1394Device::DC1394Device (string devname, string inputName, string standardName, string bayer, int fps,unsigned int w,unsigned int h, int depth )
  : VideoDevice( "DC1394", devname, inputName, standardName, fps, w, h, depth )
{



  if (GetCamera(&mCamera) < 0){
    fprintf(stderr,"No cameras found");
    return;
  }//if

  int32_t i;
  dc1394video_modes_t video_modes;
  dc1394video_mode_t video_mode;
  //dc1394video_mode_t minsize_video_mode;
  dc1394framerate_t frame_rate;
  dc1394framerates_t frame_rates;
  dc1394color_coding_t color_coding;
  //dc1394featureset_t features;

  /* Get supported video modes */
  if (dc1394_video_get_supported_modes(mCamera,&video_modes)!=DC1394_SUCCESS)
  {
    CameraCleanup(mCamera);
    fprintf(stderr,"Could not get supported video modes");
    return;
  }

  /* Output supported modes */
  printf("Supported color codings and resolutions:\n");
  for ( i = video_modes.num-1; i >= 0; i-- )
  {
    if ( ! dc1394_is_video_mode_scalable(video_modes.modes[i] ) )
    {
      unsigned int widthScalable = 0, heightScalable = 0;
      dc1394_get_color_coding_from_video_mode(mCamera,
                                              video_modes.modes[i],&color_coding);
      dc1394_get_image_size_from_video_mode(mCamera,
                                            video_modes.modes[i], &widthScalable, &heightScalable);
      dc1394_video_get_supported_framerates(
                                            mCamera,video_modes.modes[i],
                                            &frame_rates);
      printf("(mode: %d color id %d, resolution %dx%d, framerate id %d)\n",
             video_modes.modes[i], color_coding, widthScalable, heightScalable,
             frame_rates.framerates[frame_rates.num-1]);
    }
  }
  printf("\n");
	
  /* Initialize to make compiler happy */
  video_mode=video_modes.modes[0];

  /* Try to select a RGB 24bpp video mode */
  if (GetVideoModeWithColorCoding( DC1394_COLOR_CODING_RGB8, mCamera, video_modes, video_mode ) < 0)
  {
    /* Try to select a MONO8 video mode
    * (it is actually Bayer, for the
    *  PointGrey Scorpion)
                */
    if (GetVideoModeWithColorCoding(DC1394_COLOR_CODING_MONO8,
        mCamera, video_modes, video_mode) < 0)
    {
      if (dc1394_is_video_mode_scalable(video_mode))
      {
        fprintf(stderr,
                "No support for scalable video modes yet\n");
        return;
      } else {
        /* Try to use the video mode found */
      }
    }
  }
  //get the video width and height
  if(dc1394_get_image_size_from_video_mode(mCamera, video_mode, static_cast<uint32_t * >( & width) , static_cast<uint32_t *>( & height ) ) != DC1394_SUCCESS)
  {
    std::cout << __FILE__ << ":" << __LINE__ << " " << __PRETTY_FUNCTION__
              << "Problem getting image size" << endl;
    width =w;
    height=h;
  }
  else
  {
    if(width < w || height < h)
    {
      std::cout << __FILE__ << ":" << __LINE__ << " " << __PRETTY_FUNCTION__
                << "Desired video mode not available at desired resolution";
      std::exit(0);
    }
  }

/*
  if (GetVideoModeWithMinSize(
      w, h,
      mCamera, video_modes,
      minsize_video_mode) < 0)
  {
    fprintf(stderr, "Supported size seems too small.\n");
  } else {
    //video_mode = minsize_video_mode;
    dc1394_get_image_size_from_video_mode(mCamera, video_mode, static_cast<uint32_t * >( & width) , static_cast<uint32_t *>( & height ) );
  }
  std::cout << __FILE__ << ":" << __LINE__ << " " << __PRETTY_FUNCTION__ 
	    << "Video mode " << video_mode << " image size set to " << width << "x" << height << " pixels." << std::endl; 
  cout << video_mode << endl;
  video_mode = DC1394_VIDEO_MODE_640x480_RGB8;
*/
  
  /* Find highest supported framerate for that mode */
  if (dc1394_video_get_supported_framerates(
      mCamera,video_mode,&frame_rates)!=DC1394_SUCCESS)
  {
    fprintf(stderr,	"Could not get supported frame rates, so trying %dfps.\n", fps);
    frame_rate = ( dc1394framerate_t ) fps;
  } else {
    bool found = false;
    for(unsigned int i = 0; !found && i < frame_rates.num; i++){
      std::cout << __FILE__ << ":" << __LINE__ << " " << __PRETTY_FUNCTION__ 
		<< " frame rate " << frame_rates.framerates[i] << std::endl;
      if(static_cast< dc1394framerate_t >( fps ) == frame_rates.framerates[i]){
        found = true;
        frame_rate = ( dc1394framerate_t ) fps;
      }
    }
		
    if( !found ){
      fprintf(stderr,	"%dfps not supported, trying anyways...\n", fps);
      frame_rate = ( dc1394framerate_t ) fps;
    }//if
  }

  /* Setup the bus speed */
  if (dc1394_video_set_iso_speed(mCamera, ISO_SPEED)!=DC1394_SUCCESS)
  {
    fprintf(stderr,"Could not set ISO speed");
    CameraCleanup(mCamera);
    return;
  }

  /* Setup the frame resolution and color space */
  if (dc1394_video_set_mode(mCamera, video_mode)!=DC1394_SUCCESS)
  {
    fprintf(stderr, "Could not set video mode");
    CameraCleanup(mCamera);
    return;
  }

  /* Setup the frame rate */
  if (dc1394_video_set_framerate(mCamera,frame_rate)!=DC1394_SUCCESS)
  {
    fprintf(stderr,"Could not set framerate");
    CameraCleanup(mCamera);
    return;
  }

  /* Setup capture with VIDEO1394 */
  if (dc1394_capture_setup(mCamera,NUM_BUFFERS,DC1394_CAPTURE_FLAGS_DEFAULT)!=DC1394_SUCCESS)
  {
    fprintf(stderr,"Problem setting capture");
    CameraCleanup(mCamera);
    return;
  }


  /* Report camera's features */
  /*if (dc1394_get_camera_feature_set(mCamera,&features)!=DC1394_SUCCESS)
  {
    fprintf(stderr,"Could not get the set of supported features");
    CameraCleanup(mCamera);
    return;
  } else {
    dc1394_print_feature_set(&features);
  }*/

  this->bayer = (bayer == "y");
  
  error = OKAY;
  /* If we got here, then we succeeded */
  //mDeallocFrame=0;
}//constructor

DC1394Device::~DC1394Device( )
{
  /* Frees allocated memory for the RGB frame */
  //  free(mRgbFrame.image);

  /* Stop cameras, de-allocate memory, close devices */
  CameraCleanup(mCamera);

  /* Reset the bus */
  dc1394_reset_bus(mCamera);
}




int DC1394Device::CameraCleanup(dc1394camera_t *pCamera)
{
  /* Give some time */
  usleep(500000);

  /* Stop transmission */
  if (dc1394_video_set_transmission(pCamera, DC1394_OFF)!=DC1394_SUCCESS)
  {
    cerr << "Problem stopping transmission, ";
    perror( ":" );
    return -1;
  }

  /* Stop capture */
  if (dc1394_capture_stop(pCamera)!=DC1394_SUCCESS)
  {
    cerr << "Could not stop capture, ";
    perror( ":" );
    return -1;
  }

  /* If here, then succeeded */
  return 0;
}





int DC1394Device::GetCamera(dc1394camera_t **return_camera)
{
  dc1394camera_list_t * list;
  dc1394_t * d;
  //uint32_t totalCameras; //Total number of connected cameras
  //dc1394camera_t **cameras=NULL; //Vector pointing to the connected cameras
  //uint32_t i; //index for searching cameras
  dc1394error_t err;
  /* Find the connected cameras and allocates memory */
  //if (dc1394_find_cameras(&cameras, &totalCameras)!=DC1394_SUCCESS)
  d = dc1394_new ();
  err=dc1394_camera_enumerate (d, &list);
  DC1394_ERR_RTN(err,"Failed to enumerate cameras"
            " - Verify if the camera is connected\n"
            " - On Linux verify if the kernel modules ieee1394, raw1394, ohci1394 are loaded\n"
            " - Verify if you have read/write access to /dev/raw1394\n\n");

  /*{
    fprintf(stderr,"Problem finding/allocating camera\n"
        " - Verify if the camera is connected\n"
            " - On Linux verify if the kernel modules ieee1394, raw1394, ohci1394 are loaded\n"
            " - Verify if you have read/write access to /dev/raw1394\n\n");
    return -1;
  }*/

  /* Check how many cameras were found */
  /*if (totalCameras < 1)
  {
    fprintf (stderr, "No cameras were found\n");
    std::exit(1);
  }*/
  if (list->num == 0) {
        dc1394_log_error("No cameras found");
        return 1;
    }


  /* Will use the first camera in the bus */
  *return_camera = dc1394_camera_new (d, list->ids[0].guid);
  if (!*return_camera) {
        dc1394_log_error("Failed to initialize camera with guid %llx", list->ids[0].guid);
        return 1;
    }
    dc1394_camera_free_list (list);

  //*return_camera = cameras[0];

  /* Free the other cameras */
  /*for (i=1;i<totalCameras;i++) {
    dc1394_free_camera(cameras[i]);
  }*/
  //free(cameras);

  return 0;
}


/* Get video mode that matches the desired color coding
 */
int DC1394Device::GetVideoModeWithColorCoding(dc1394color_coding_t pColorCoding,
					      dc1394camera_t *pCamera,
					      dc1394video_modes_t pVideoModeList,
					      dc1394video_mode_t &pVideoMode)
{
  int result = -1;

  /* for storing the color coding */
  dc1394color_coding_t color_coding;
  
  /* Select highest resolution of the given color coding*/
  for ( int i = pVideoModeList.num-1; i >= 0; i-- ) 
    {
      if ( !dc1394_is_video_mode_scalable(pVideoModeList.modes[i] ) ) 
	{
	  dc1394_get_color_coding_from_video_mode( pCamera, pVideoModeList.modes[i], &color_coding );
	  if (color_coding==pColorCoding)
	    {
	      pVideoMode=pVideoModeList.modes[i];
	      result = 0;
	      break;
	    }
	}
    }
  return result;
}

/* Get video mode that matches the desired image size
 */
int DC1394Device::GetVideoModeWithMinSize(unsigned int pMinWidth, unsigned int pMinHeight,
                                     dc1394camera_t *pCamera,
                                     dc1394video_modes_t pVideoModeList,
                                     dc1394video_mode_t &pVideoMode)
{
  /* Select highest resolution of the given color coding*/
  for (int i=pVideoModeList.num-1;i>=0;i--) {
    if (!dc1394_is_video_mode_scalable(pVideoModeList.modes[i])) {
      if (dc1394_get_image_size_from_video_mode(pCamera,
          pVideoModeList.modes[i],
          &width, &height)==DC1394_SUCCESS)
      {
	if ( width == pMinWidth && height == pMinHeight )
	{
 	  pVideoMode = pVideoModeList.modes[i];
	  break;
	}	
        else if (width > pMinWidth && height > pMinHeight)
        {
          pVideoMode=pVideoModeList.modes[i];
        }
      }
    }
  }
	
  if ((dc1394_is_video_mode_scalable(pVideoMode)) ||
       (width < pMinWidth) || (height < pMinHeight))
  {
    return -1;
  } else {
    return 0;
  }
}



/*------------------------------------------------------------------------
 * Start transmission
 *------------------------------------------------------------------------*/
int DC1394Device::StartTransmission(dc1394camera_t *pCamera)
{
	dc1394switch_t status = DC1394_OFF;
	uint32_t i = 0;
	if (dc1394_video_set_transmission(pCamera, DC1394_ON) != DC1394_SUCCESS)
	{
		CameraCleanup(pCamera);
		fprintf(stderr,"Problem starting transmission");
		return -1;
	}
	while(status == DC1394_OFF && i++ < 5) {
		usleep(50000);
		if (dc1394_video_get_transmission(pCamera,&status) != DC1394_SUCCESS)
		{
			CameraCleanup(pCamera);
			fprintf(stderr,"Unable to get transmission status\n");
			return -1;
		}
	}
	if (i==5) {
		fprintf(stderr, "Camera is taking too long to turn on\n");
		CameraCleanup(pCamera);
		return -1;
	}

	/* If we got here then we have succeeded */
	return 0;
}


/*------------------------------------------------------------------------
 * Stop transmission
 *------------------------------------------------------------------------*/
int DC1394Device::StopTransmission(dc1394camera_t *pCamera)
{
	if (dc1394_video_set_transmission(pCamera, DC1394_OFF) != DC1394_SUCCESS)
	{
		CameraCleanup(pCamera);
		fprintf(stderr,"Problem stopping transmission");
		return -1;
	}

	/* If we got here then we have succeeded */
	return 0;
}

enum DC1394Device::ErrorCode DC1394Device::getErrorCode( void ) const
{
  return errorCode;
}

int DC1394Device::startCapture (void)
{
  int err;


  /* Allocates memory for the RGB frame */
  mRgbFrame.image = (unsigned char *)malloc( getWidth() * getHeight() * 3 );
  if (mRgbFrame.image==NULL) {
    fprintf(stderr,"Could not allocate memory for the RGB image\n");
    CameraCleanup(mCamera);
    return -1;
  }
  
  /* Initialize frame structure */
  mRgbFrame.allocated_image_bytes = getWidth() *getHeight() * 3;
  mRgbFrame.color_coding = DC1394_COLOR_CODING_RGB8;
  
  /* Init pointer to that frame */
  mDebayeredFrame = &mRgbFrame;

  
  err = StartTransmission(mCamera);
  if (err)
  {
    cerr << "ERROR: DC1394Device::StartTransmission() failed";
    perror( ":" );
    errorCode = VIDIOC_STREAMOFF_FAILURE;
  }

  return err;
}

int DC1394Device::stopCapture( void )
{
  int err;
  err = StopTransmission(mCamera);
  if (err)
  {
    cerr << "ERROR: DC1394Device::StopTransmission() failed";
    perror( ":" );
    errorCode = VIDIOC_STREAMOFF_FAILURE;
  }

  /* Frees allocated memory for the RGB frame */
    free(mRgbFrame.image);
    mRgbFrame.image = 0;
    mRgbFrame.allocated_image_bytes = 0;
    
    CameraCleanup(mCamera);
  return err;
}

void
DC1394Device::nextFrame ( FrameBuffer * * curFramePtr )
{
  if(bayer)
  {
    if ( ( * curFramePtr != 0 ) && ( (*curFramePtr)->type() != FrameBuffer::BAYER) )
      {
        delete *curFramePtr;
        *curFramePtr = 0;
      }

    /* Capture one frame */
    if (dc1394_capture_dequeue(mCamera, DC1394_CAPTURE_POLICY_WAIT, &mCapturedFrame) != DC1394_SUCCESS)
    {
      CameraCleanup(mCamera);
      fprintf(stderr,"Problem capturing the first image frame");
      return;
    }
    if ( * curFramePtr == 0 )
    {
      *curFramePtr = new FrameBufferBayer();
      (*curFramePtr)->initialize( getHeight(), getWidth() );
    }
  }
  else
  {
    if ( ( * curFramePtr != 0 ) && ( (*curFramePtr)->type() != FrameBuffer::RGB24BE) )
    {
      delete *curFramePtr;
      *curFramePtr = 0;
    }

        /* Capture one frame */
    if (dc1394_capture_dequeue(mCamera, DC1394_CAPTURE_POLICY_WAIT, &mCapturedFrame) != DC1394_SUCCESS)
    {
      CameraCleanup(mCamera);
      fprintf(stderr,"Problem capturing the first image frame");
      return;
    }
    if ( * curFramePtr == 0 )
    {
      *curFramePtr = new FrameBufferRGB24BE();
    }
  }

   


  switch (mCapturedFrame->color_coding)
  {
    case DC1394_COLOR_CODING_MONO8:
      /* Fix color coding info for enabling conversion */
      mCapturedFrame->color_coding = DC1394_COLOR_CODING_RAW8;
    case DC1394_COLOR_CODING_RAW8:
      mCapturedFrame->color_filter = BAYER_FILTER;
      /* Transform from bayer format to rgb 24 bits per pixel (r+g+b) */
      /*std::cerr << "(" << __FILE__ << ":" << __LINE__ << ") "
		<< __PRETTY_FUNCTION__
		<< "WARNING: no debayered frame method available anymore" << std::endl;*/
        //dc1394_debayer_frames(mCapturedFrame, mDebayeredFrame, DEBAYER_METHOD);
        //(*curFramePtr)->buffer = (unsigned char *) (mDebayeredFrame->image);
        (*curFramePtr)->buffer = (unsigned char *) (mCapturedFrame->image);
      break;
    case DC1394_COLOR_CODING_RGB8:
            (*curFramePtr)->buffer = (unsigned char *) (mCapturedFrame->image);
            //memcpy(mDebayeredFrame->image, mCapturedFrame->image, getWidth() * getHeight() * 3);
      break;
    default:
      /* Transform from arbitrary format to rgb 24 bpp */
      //      dc1394_convert_frames(mCapturedFrame, mDebayeredFrame);
      std::cerr << "(" << __FILE__ << ":" << __LINE__ << ") "
		<< __PRETTY_FUNCTION__
		<< "WARNING: no debayered frame method available anymore" << std::endl;
      break;
  }
		
  /* Save pointer to img in FrameBuffer */

  //(*curFramePtr)->buffer = (unsigned char *) (mCapturedFrame->image);

  //*(mCapturedFrame->image) = 1;

  (*curFramePtr)->width = getWidth();
  (*curFramePtr)->height = getHeight();

  //Not sure what the next two vals should be... 
  //  (*curFramePtr)->bytesPerPixel = 3; //mCapturedFrame -> bit_depth; `

  //can get mCapturedFrame -> dc1394color_coding_t -> color_coding, but types look incompatible (from control.h)
  //  (*curFramePtr)->type = FrameBuffer::RGB24;

  //  (*curFramePtr)->bytesPerLine = mCapturedFrame->stride * (*curFramePtr)->bytesPerPixel;
  (*curFramePtr)->bytesPerLine = (*curFramePtr)->width * (*curFramePtr)->bytesPerPixel;
  (*curFramePtr)->frameSize = (*curFramePtr)->bytesPerLine * (*curFramePtr)->height;

  (*curFramePtr)->interlaced = false;
  (*curFramePtr)->fieldNo = 1;  //This is just a guess...

  (*curFramePtr)->fieldsPerSecond = fps;
  (*curFramePtr)->absFrameNo++;// = mCapturedFrame -> id;    //Another guess...
  (*curFramePtr)->setTimestamp();

}


int DC1394Device::releaseCurrentBuffer (void)
{

  /* Release the buffer */
  if (dc1394_capture_enqueue(mCamera, mCapturedFrame) != DC1394_SUCCESS)
  {
    CameraCleanup(mCamera);
    std::cerr << "(" << __FILE__ << ":" << __LINE__ << ") "
	      << __PRETTY_FUNCTION__ 
	      << "Could not release buffer" << std::endl;
  }
  //mDeallocFrame = mCapturedFrame;
  return 0;
}

bool DC1394Device::isInterlaced( void )
{
  //always false, since progressive scan
  return false;
}







