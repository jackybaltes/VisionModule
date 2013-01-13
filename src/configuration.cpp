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
}

//Configuration::Configuration( std::string configStr )
//{
//  
//}

Configuration::Configuration( po::variables_map const & vm )
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

