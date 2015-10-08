#include "httpd.h"
#include "httpdthread.h"
#include "httpdserverthread.h"
#include "videostream.h"
#include "serial.h"
#include "configuration.h"
#include "globals.h"
#include "udpvisionserver.h"

#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <boost/program_options.hpp>
#include <arpa/inet.h>


using namespace std;

namespace po = boost::program_options;

int ApplyConfiguration( Configuration & configuration );

static char progname[256];

void
CatchHUPSignal( int num )
{
  signal(SIGHUP, CatchHUPSignal );
  int err;

  if ( ( err = execl( progname, "-c", "./www/__config__.cfg", NULL) ) < 0 )
    {
      perror("execl failed:");
      exit(10);
    }
}

int 
main( int argc, char ** argv )
{
  string config_file;
  Configuration configuration;
  //  Globals * glob = Globals::GetGlobals();

  strncpy(progname, argv[0], 255 );
  progname[255] = '\0';

  try 
    {
      // Declare a group of options that will be 
      // allowed only on command line
      po::options_description commandLineOnlyOptions("Command Line Options");
      commandLineOnlyOptions.add_options()
	("version,v", "print version string")
	("help", "produce help message")    
	("config,c", po::value<string>( & config_file )->default_value(""), "config file name")
	;

      po::options_description commandLineOptions;
      commandLineOptions.add(commandLineOnlyOptions).add(configuration.options)
	;
      po::variables_map vm;
      po::store(po::parse_command_line(argc,argv, commandLineOptions),vm);
      po::notify(vm);

      if ( vm.count("help") )
	{
	  std::cout << commandLineOptions << "\n";
	  return 1;
	}

      if (vm.count("version") )
	{
	  cout << "Version 0.0.1 Sun Jan 13 02:21:18 CST 2013" << endl;
	  return 1;
	}

      if ( config_file != "" ) 
	{
	  ifstream ifs( config_file.c_str() );
	  
	  po::store(po::parse_config_file(ifs, configuration.options), vm );
	  po::notify(vm);
	}

      configuration.UpdateConfiguration( vm );
      std::cout << configuration;
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
  Globals * glob = Globals::GetGlobals();
  
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

  glob->SetBuffer( ( uint8_t *)malloc( configuration.width * configuration.height * configuration.depth/8 ), configuration.width * configuration.height * configuration.depth/8 );

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


  glob->SetVideo( video );

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

  glob->SetSerial( serial );

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
      
  HTTPD * server = new HTTPD( configuration.http_port, 
			      configuration.http_addr.c_str(), 
			      NULL, 
			      configuration.docroot.c_str(), 
			      configuration.index.c_str(), 
			      video->Commands );
  glob->SetHTTPDServer( server );

  HTTPDServerThread * thread = new HTTPDServerThread( server );

  glob->SetHTTPDServerThread( thread );
  thread->StartAndDetach();
      
  cout << "Starting video processing thread" << endl;
      
  pthread_create(&(video->threadID), NULL, (void * (*) ( void *)) video->run_trampoline, static_cast<void *>( video ) );
  pthread_detach(video->threadID);
      
  signal(SIGHUP, CatchHUPSignal );

  if ( configuration.udp_port > 0 )
    {
      boost::asio::io_service io_service;
      
      UDPVisionServer * vs = new UDPVisionServer( io_service, configuration.udp_port, video );
      vs->StartServer();   // Does not return right now
    }
  for(;;)
    {
      sleep(1000);
    }
}
