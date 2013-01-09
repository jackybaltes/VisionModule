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
#include "configuration.h"

using namespace std;

namespace po = boost::program_options;

globals VideoStream::global;

int ApplyConfiguration( Configuration & configuration );

int 
main( int argc, char ** argv )
{
  string config_file;

  int bayer = 0;

  Configuration configuration;

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
      unsigned int subsample;
      po::options_description generalOptions("General Options");
      generalOptions.add_options()
	("subsample", po::value<unsigned int>( & configuration.subsample )->default_value(1),"sub sample")
	;

      po::options_description cameraOptions("Camera Options");
      cameraOptions.add_options()
	("video_device,d", po::value<string>(& configuration.device_video)->default_value("/dev/video0"), "video device name")
	("width,w", po::value<unsigned int>( & configuration.width )->default_value(320),"width")
	("height,h", po::value<unsigned int>( & configuration.height )->default_value(240),"height")
	("depth", po::value<unsigned int>( & configuration.depth )->default_value(24),"depth")
	("brightness", po::value<int> ( & configuration.brightness )->default_value(-1),"brightness")
	("contrast", po::value<int> ( & configuration.contrast )->default_value(-1),"contrast")
	("saturation", po::value<int> ( & configuration.saturation )->default_value(-1),"saturation")
	("sharpness", po::value<int> ( & configuration.sharpness )->default_value(-1),"sharpness")
	("gain", po::value<int> ( & configuration.gain )->default_value(-1),"gain")
	;

      po::options_description httpOptions("Http Server Options");
      httpOptions.add_options()
	("http-port", po::value<unsigned int>( & configuration.http_port )->default_value(8080)->required(),"http port number")
	("http-addr", po::value<string>(& configuration.http_addr)->default_value("0.0.0.0")->required(),"http address")
	("docroot", po::value<string>(& configuration.docroot)->default_value("www/")->required(),"http document root")
	("index", po::value<string>(& configuration.index)->default_value("index.html"),"index.html file name")
	;
        
      po::options_description colourOptions("General Options");
      colourOptions.add_options()
	("colour", po::value<vector<string> >( & configuration.colours),"colour definition")
      ;
      
      po::options_description serialOptions("Serial Port Options");
      serialOptions.add_options()
	("serial_device", po::value<string>( & configuration.device_serial )->default_value(""),"serial device name or empty for no serial port output")
	("baudrate", po::value<string>(& configuration.baudrate)->default_value("B115200"),"baudrate");

      po::options_description commandLineOptions;
      commandLineOptions.add(commandLineOnlyOptions).add(generalOptions).add(cameraOptions).add(httpOptions).add(colourOptions).add(serialOptions)
	;

      po::variables_map vm;
      po::store(po::parse_command_line(argc,argv,commandLineOptions),vm);
      po::notify(vm);

      po::options_description configFileOptions;
      configFileOptions.add(generalOptions).add(cameraOptions).add(httpOptions).add(colourOptions).add(serialOptions);

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

      ApplyConfiguration( configuration );
    }
  catch( exception & e )
    {
      cerr << e.what() << endl;
      return 1;
    }
}

int
ApplyConfiguration( Configuration & configuration )
{
  string driver;
  
  if ( configuration.device_video.substr(0, 10) ==  "/dev/video" )
    {
      driver = "V4L2";
    }
  else if ( configuration.device_video.substr(0, 5) ==  "file:" )
    {
      driver = "File";
      configuration.device_video = configuration.device_video.substr( 5, configuration.device_video.length() );
    }
  else if ( configuration.device_video.substr(0, 11) ==  "/dev/dc1394" )
    {
      driver = "DC1394";
    }
  else
    {
      cerr << "Unable to determine video capture device driver " << configuration.device_video << endl;
      return 1;
    }
  
#if defined(DEBUG)
  cout << "device " << configuration.device_video << ", driver " << driver << ", width " << configuration.width << ", height " << configuration.height << endl;
#endif

  string input("default");
  string standard("default");
  unsigned int numBuffers = 3;
  unsigned int fps = 30;

  VideoStream * video = new VideoStream( driver, 
					 configuration.device_video, 
					 input, 
					 standard, 
					 fps, 
					 configuration.width, 
					 configuration.height, 
					 configuration.depth, 
					 numBuffers, 
					 configuration.subsample, 
					 configuration.brightness, 
					 configuration.contrast, 
					 configuration.saturation, 
					 configuration.sharpness, 
					 configuration.gain );

  // TODO Auto-generated constructor stub
  VideoStream::global.buf = ( uint8_t *)malloc( configuration.width * configuration.height * configuration.depth/8 );

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
  VideoStream::global.size = configuration.width * configuration.height * configuration.depth/8;
  
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

  Serial * serial = 0;
  if ( configuration.device_serial != "" )
    {
      serial = new Serial(configuration.device_serial, 
			  Serial::ConvertStringToBaudrate( configuration.baudrate ) );
      if ( ( serial == 0 ) || ( ! serial->isValid() ) )
	{
	  serial = 0;
	}
    }


  video->server.pglobal = & VideoStream::global;
  video->server.conf.http_port = configuration.http_port;
  video->server.conf.http_addr = configuration.http_addr.c_str();
  video->server.conf.credentials = NULL;
  video->server.conf.docroot = configuration.docroot.c_str();
  video->server.conf.index = configuration.index.c_str();
  video->server.conf.nocommands = 0;
  video->server.conf.commands = video->Commands;
  video->server.conf.video = video;
  video->server.conf.serial = serial;

  std::vector<ColourDefinition> colourDefs;

  for(std::vector<string>::iterator it = configuration.colours.begin(); it != configuration.colours.end(); ++it) 
    {
      ColourDefinition cd;
      std::stringstream is(*it);
      is.clear();
      is >> cd;
      if ( ! is.fail() ) 
	{
	  colourDefs.push_back( cd );
	}
      else
	{
	  std::cerr << "Reading of colour " << (*it) << " failed" << std::endl;
	}
	  
#if defined(DEBUG)
      cout << "adding colour" << endl;
#endif	  
    }
  video->nextColours = 0;
  video->SetColours( colourDefs );
      
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
      sleep(1000);
    }
}
