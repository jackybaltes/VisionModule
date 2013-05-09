/* 
 * A class that encapsulates all configuration information
 * Jacky Baltes <jacky@cs.umanitoba.ca> Tue Jan  8 23:20:07 CST 2013
 *
 */

#include "configuration.h"
#include "../libvideo/colourdefinition.h"
#include <boost/program_options.hpp>

namespace po = boost::program_options;

Configuration::Configuration( )
{
  po::options_description generalOptions("General Options");
  generalOptions.add_options()
    ("subsample", po::value<unsigned int>( )->default_value(1),"sub sample")
    ;
  
  po::options_description cameraOptions("Camera Options");
  cameraOptions.add_options()
    ("video_device,d", po::value<string>( )->default_value("/dev/video0"), "video device name")
    ("width,w", po::value<unsigned int>( )->default_value(320),"width")
    ("height,h", po::value<unsigned int>( )->default_value(240),"height")
    ("depth", po::value<unsigned int>( )->default_value(24),"depth")
    ("brightness", po::value<int> ( )->default_value(-1),"brightness")
    ("contrast", po::value<int> ( )->default_value(-1),"contrast")
    ("saturation", po::value<int> ( )->default_value(-1),"saturation")
    ("sharpness", po::value<int> ( )->default_value(-1),"sharpness")
    ("gain", po::value<int> ( )->default_value(-1),"gain")
    ;
  
  po::options_description httpOptions("Http Server Options");
  httpOptions.add_options()
    ("http_port", po::value<unsigned int>( )->default_value(8080)->required(),"http port number")
    ("http_addr", po::value<string>( )->default_value("0.0.0.0")->required(),"http address")
    ("docroot", po::value<string>( )->default_value("www/")->required(),"http document root")
    ("index", po::value<string>( )->default_value("index.html"),"index.html file name")
    ;
  
  po::options_description colourOptions("General Options");
  colourOptions.add_options()
    ("colour", po::value<vector<string> >( ),"colour definition")
    ;
  
  po::options_description serialOptions("Serial Port Options");
  serialOptions.add_options()
    ("serial_device", po::value<string>( )->default_value(""),"serial device name or empty for no serial port output")
    ("baudrate", po::value<string>( )->default_value("B115200"),"baudrate");

  options.add(generalOptions).add(cameraOptions).add(httpOptions).add(colourOptions).add(serialOptions);
}

void
Configuration::UpdateConfiguration( std::string configStr )
{
  std::istringstream is( configStr );

  std::cout << ">>>>>>>>>> UpdateConfiguration" << std::endl;
  std::cout << configStr << std::endl;
  std::cout << "<<<<<<<<<< UpdateConfiguration" << std::endl;

  UpdateConfiguration( is );
}

void
Configuration::UpdateConfiguration( std::istream & iconfig )
{
  po::variables_map vm;

  po::store(po::parse_config_file(iconfig, options), vm );
  po::notify(vm);

  UpdateConfiguration( vm );
}

void
Configuration::UpdateConfiguration( po::variables_map const & vm )
{
  // General options
  subsample = vm["subsample"].as<unsigned int>();

  // Camera options
  device_video = vm["video_device"].as<std::string>();
  width = vm["width"].as<unsigned int>();
  height = vm["height"].as<unsigned int>();
  depth = vm["depth"].as<unsigned int>();

  brightness = vm["brightness"].as<int>();
  contrast = vm["contrast"].as<int>();
  saturation = vm["saturation"].as<int>();
  sharpness = vm["sharpness"].as<int>();
  gain = vm["gain"].as<int>();

  // HTTPD options
  http_port = vm["http_port"].as<unsigned int>();
  http_addr = vm["http_addr"].as<std::string>();
  docroot = vm["docroot"].as<std::string>();
  index = vm["index"].as<std::string>();

  // Colour options
  if ( vm.count("colour") > 0 ) 
    {
      colours = vm["colour"].as<std::vector<string> >();
    }

  // Serial options
  device_serial = vm["serial_device"].as<std::string>();
  baudrate = vm["baudrate"].as<std::string>();
}
      

std::ostream & 
operator<<(std::ostream & os, Configuration const & config )
{
  // General options
  os << "subsample" << " = " << config.subsample << "\r\n";
  os << "\r\n";

  // Camera options
  os << "video_device" << " = " << config.device_video << "\r\n"
     << "width" << " = " << config.width << "\r\n"
     << "height" << " = " << config.height << "\r\n"
     << "depth" << " = " << config.depth << "\r\n"
     << "brightness" << " = " << config.brightness << "\r\n"
     << "contrast" << " = " << config.contrast << "\r\n"
     << "saturation" << " = " << config.saturation << "\r\n"
     << "sharpness" << " = " << config.sharpness << "\r\n"
     << "gain" << " = " << config.gain << "\r\n";
  os << "\r\n";

  // HTTPD options
  os << "http_port" << " = " << config.http_port << "\r\n"
     << "http_addr" << " = " << config.http_addr << "\r\n"
     << "docroot" << " = " << config.docroot << "\r\n"
     << "index" << " = " << config.index << "\r\n";
  os << "\r\n";

  // Colours
  for( vector<string>::const_iterator i = config.colours.begin();
       i != config.colours.end();
       ++i)
    {
      string const t = (*i);
      os << "colour" << "=" << t << "\r\n";
    }
  // Serial options
  if ( config.device_serial != "" )
    {
      os << "serial_device" << " = " << config.device_serial << std::endl
	 << "baudrate" << " = " << config.baudrate << std::endl;
    }
  ;
  return os;
}

