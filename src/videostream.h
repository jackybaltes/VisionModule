/*
 * videostream.h
 *
 */

#ifndef __VIDEOSTREAM_H__
#define __VIDEOSTREAM_H__

#include <string>
#include <vector>

#include <pthread.h>

#include "httpd.h"

#include "../libvideo/colourdefinition.h"

using namespace std;

class FrameBuffer;
class VideoDevice;
class ColourDefinition;
class RawPixel;


class VideoStream
{
public:

  enum ProcessType {
    Raw,
    ShowColours,
    SegmentColours,
    Scanlines,
    Segmentation
  };

  enum VideoControl 
  {
    IllegalControl,
    Brightness,
    Hue,
    Saturation,
    Contrast,
    Sharpness
  };

  VideoStream(string driver,
	      string name,
	      string input,
	      string standard,
	      unsigned int fps,
	      unsigned int width,
	      unsigned int height,
	      unsigned int depth,
	      unsigned int numBuffers,
	      unsigned int subSample
	      );

  virtual ~VideoStream();
  
  static void* server_thread(void * arg);
  
  void run( );
  static void run_trampoline( VideoStream * vs );
  
  int input_init();
  //int input_cmd(in_cmd_type cmd, int value);
  int sendImage(FrameBuffer * img);
  
  //int output_init();
  //int output_run();
  
  void setDone( bool done );
  
  bool getDone( void );
  
  static globals          global;
  
  //  pthread_t               cam;
  pthread_mutex_t         controls_mutex;
  
  context                 server;
  
  void ProcessFrame( enum ProcessType ptype, 
		     FrameBuffer * frame, 
		     FrameBuffer * outFrame, 
		     unsigned int subSample, 
		     std::vector<ColourDefinition> colours, 
		     RawPixel marks[] );

  static int CommandProcessingMode( VideoStream * video, char const * command, char * response, unsigned int respLength );
  static int CommandUpdateColour( VideoStream * video, char const * command, char * response, unsigned int respLength );
  static int CommandQueryColour( VideoStream * video, char const * command, char * response, unsigned int respLength );
  static int CommandVideoControl( VideoStream * video, char const * command, char * response, unsigned int respLength );

 private:
  VideoDevice * device;
  struct timeval prev;
  bool done;

 public:
  pthread_t threadID;

 public:

  static struct Command const Commands[];

 private:
  volatile enum ProcessType mode;

 private:
  void UpdateColour(ColourDefinition const colour );
  ColourDefinition * GetColour( std::string const & name );

 public:
  enum ProcessType GetMode( void ) const;
  void SetMode( enum ProcessType mode );

 public:
  std::vector<ColourDefinition> colours;

 public:
  unsigned int subSample;
};

#endif /* __VIDEOSTREAM_H__ */
