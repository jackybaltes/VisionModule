/*
 * videostream.h
 *
 */

#ifndef __VIDEOSTREAM_H__
#define __VIDEOSTREAM_H__

#include <string>

#include <pthread.h>

#include "httpd.h"

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

  VideoStream(string driver,
	      string name,
	      string input,
	      string standard,
	      unsigned int fps,
	      unsigned int width,
	      unsigned int height,
	      unsigned int depth,
	      unsigned int numBuffers
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
		     ColourDefinition colours[], 
		     unsigned int numColours, 
		     RawPixel marks[] );
 private:
  VideoDevice * device;
  struct timeval prev;
  bool done;
 public:
  pthread_t threadID;
};


#endif /* __VIDEOSTREAM_H__ */
