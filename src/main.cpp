#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <boost/program_options.hpp>
#include <arpa/inet.h>

#include "httpd.h"
#include "videostream.h"
#include "serial.h"

using namespace std;

namespace po = boost::program_options;

globals VideoStream::global;


int 
main( int argc, char ** argv )
{
  string config_file;
  string device_video;
  string driver;
  string index;

  string input("default");
  string standard("default");

  int bayer = 0;

  unsigned int width = 0;
  unsigned int height = 0;
  unsigned int depth = 32;
  unsigned int numBuffers = 2;
  unsigned int fps = 10;

  unsigned int http_port;
  string http_addr;
  string docroot;

  string device_serial;
  string baudrate;
  Serial * serial;

  unsigned int subSample;

  try 
    {
      // Declare a group of options that will be 
      // allowed only on command line
      po::options_description commandLineOnlyOptions("Command Line Options");
      commandLineOnlyOptions.add_options()
	("version,v", "print version string")
	("help", "produce help message")    
	("config,c", po::value<string>( & config_file )->default_value("vision_module.cfg"), "config file name")
	;
      
      po::options_description generalOptions("General Options");
      generalOptions.add_options()
	("driver,u", po::value<string>(& driver)->default_value("v4l2"), "driver type [v4l2,v4l,bayer,file]")
	("video_device,d", po::value<string>(& device_video)->default_value("/dev/video0"), "video device name")
	("width,w", po::value<unsigned int>( & width )->default_value(320),"width")
	("height,h", po::value<unsigned int>( & height )->default_value(240),"height")
	("depth", po::value<unsigned int>( & depth )->default_value(24),"depth")
	("subsample", po::value<unsigned int>( & subSample )->default_value(1),"sub sample")
	;

      po::options_description httpOptions("Http Server Options");
      httpOptions.add_options()
	("http-port", po::value<unsigned int>( & http_port )->default_value(8080)->required(),"http port number")
	("http-addr", po::value<string>(& http_addr)->default_value("0.0.0.0")->required(),"http address")
	("docroot", po::value<string>(& docroot)->default_value("www/")->required(),"http document root")
	("index", po::value<string>(& index)->default_value("index.html"),"index.html file name");
        
      po::options_description serialOptions("Serial Port Options");
      serialOptions.add_options()
	("serial_device", po::value<string>( & device_serial )->default_value(""),"serial device name or empty for no serial port output")
	("baudrate", po::value<string>(& baudrate)->default_value("115200"),"baudrate");

      po::options_description commandLineOptions;
      commandLineOptions.add(commandLineOnlyOptions).add(generalOptions).add(httpOptions).add(serialOptions);

      po::variables_map vm;
      po::store(po::parse_command_line(argc,argv,commandLineOptions),vm);
      po::notify(vm);

      po::options_description configFileOptions;
      configFileOptions.add(generalOptions).add(httpOptions).add(serialOptions);

      ifstream ifs( config_file.c_str() );
      po::store(po::parse_config_file(ifs, configFileOptions), vm );
      po::notify(vm);

      if ( vm.count("help") )
	{
	  std::cout << commandLineOptions << "\n";
	  return 1;
	}

      if (vm.count("version") )
	{
	  cout << "Version 0.0.0 Thu Dec  6 02:25:47 CST 2012" << endl;
	  return 1;
	}

      if ( device_video.substr(0, 10) ==  "/dev/video" )
	{
	  driver = "V4L2";
	}
      else if ( device_video.substr(0, 5) ==  "file:" )
	{
	  driver = "File";
	  device_video = device_video.substr( 5, device_video.length() );
	}
      else if ( device_video.substr(0, 11) ==  "/dev/dc1394" )
	{
	  driver = "DC1394";
	}
      else
	{
	 cerr << "Unable to determine video capture device driver " << device_video << endl;
	  return 1;
	}

#if defined(DEBUG)
      cout << "device " << device_video << ", driver " << driver << ", width " << width << ", height " << height << endl;
#endif

      VideoStream * video = new VideoStream( driver, device_video, input, standard, fps, width, height, depth, numBuffers, subSample );

      // TODO Auto-generated constructor stub
      VideoStream::global.buf = ( uint8_t *)malloc( width * height * depth/8 );

#if defined(DEBUG)
      unsigned int bpp = depth/8;
      unsigned int bpl = width * bpp;
      uint8_t * p;
      for( unsigned int i = 0; i < height; i++ )
	{
	  p = static_cast<uint8_t *>(VideoStream::global.buf + i * bpl);
	  for ( unsigned int j = 0; j < width; j++ )
	    {
	      *p++ = i & 0xff;
	      *p++ = j & 0xff;
	      *p++ = 0x80;
	    }
	}
#endif
      VideoStream::global.size = width * height * depth;
  
      if(pthread_mutex_init(& VideoStream::global.db, NULL) != 0)
	{
	  perror("pthread_mutex_init:");
	  exit(EXIT_FAILURE);
	}
      
      if(pthread_cond_init(& VideoStream::global.db_update, NULL) != 0)
	{
	  perror("pthread_cond_init:");
	  exit(EXIT_FAILURE);
	}
      
      if(pthread_mutex_init(& (video->controls_mutex), NULL) != 0)
	{
	  perror("pthread_mutex_init:");
	  exit(EXIT_FAILURE);
	}

      serial = 0;
      if ( device_serial != "" )
	{
	  serial = new Serial(device_serial, Serial::convertBaudrate( baudrate ) );
	  if ( ( serial == 0 ) || ( ! serial->isValid() ) )
	    {
	      serial = 0;
	    }
	}
      video->server.pglobal = & VideoStream::global;
      video->server.conf.http_port = htons( http_port );
      video->server.conf.http_addr = http_addr.c_str();
      video->server.conf.credentials = NULL;
      video->server.conf.docroot = docroot.c_str();
      video->server.conf.index = index.c_str();
      video->server.conf.nocommands = 0;
      video->server.conf.commands = video->Commands;
      video->server.conf.video = video;
      video->server.conf.serial = serial;

#if defined(DEBUG)
      cout << "Starting video thread" << endl;
#endif

      pthread_create(&(video->server.threadID), NULL, HTTPD::server_thread, &(video->server));
      pthread_detach(video->server.threadID);

      cout << "Starting video processing thread" << endl;

      pthread_create(&(video->threadID), NULL, (void * (*) ( void *)) video->run_trampoline, static_cast<void *>( video ) );
      pthread_detach(video->threadID);

      for(;;)
	{
	  sleep(100);
	}
    }
  catch( exception & e )
    {
      cerr << e.what() << endl;
      return 1;
    }
}

