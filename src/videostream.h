/*
 * videostream.h
 *
 */

#ifndef __VIDEOSTREAM_H__
#define __VIDEOSTREAM_H__

#include <string>
#include <vector>
#include <boost/program_options.hpp>

#include <pthread.h>

#include "../libvideo/colourdefinition.h"

using namespace std;
namespace po = boost::program_options;

class FrameBuffer;
class VideoDevice;
class ColourDefinition;
class RawPixel;
class Configuration;

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
    Sharpness,
    Gain
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
	      unsigned int subsample,
	      int brightness,
	      int contrast,
	      int saturation,
	      int sharpness,
	      int gain
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
  bool getDone( void ) const;
  
  //  pthread_t               cam;
  pthread_mutex_t         controls_mutex;
  
  void ProcessFrame( enum ProcessType ptype, 
		     FrameBuffer * frame, 
		     FrameBuffer * outFrame, 
		     unsigned int subsample, 
		     std::vector<ColourDefinition> colours, 
		     RawPixel mark );


  static int CommandProcessingMode( VideoStream * video, char const * command, char * response, unsigned int respLength );
  static int CommandUpdateColour( VideoStream * video, char const * command, char * response, unsigned int respLength );
  static int CommandQueryColour( VideoStream * video, char const * command, char * response, unsigned int respLength );
  static int CommandVideoControl( VideoStream * video, char const * command, char * response, unsigned int respLength );
  static int CommandQueryColourList( VideoStream * video, char const * command, char * response, unsigned int respLength );
  static int CommandAddColour( VideoStream * video, char const * command, char * response, unsigned int respLength );
  static int CommandDeleteColour( VideoStream * video, char const * command, char * response, unsigned int respLength );
  static int CommandSelectColour( VideoStream * video, char const * command, char * response, unsigned int respLength );
  static int CommandShutdown( VideoStream * video, char const * command, char * response, unsigned int respLength );

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
  void SetColours( std::vector<ColourDefinition> colours );

 private:
  std::vector<ColourDefinition> colours;

 public:
  std::vector<ColourDefinition> * nextColours;

 private:
  unsigned int subsample;

 public:
  unsigned int GetSubsample( void ) const;
  void SetSubsample( unsigned int subsample );

 public:
  std::string ReadRunningConfiguration( void );

 public:
  void UpdateRunningConfiguration( std::string configStr );

 private:
  std::string GetColourList( void );

 private:
  std::string selectedColour;

 private:
  po::options_description configOptions;
};

#endif /* __VIDEOSTREAM_H__ */
