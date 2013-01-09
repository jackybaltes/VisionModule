/* 
 * A class that encapsulates all configuration information
 * Jacky Baltes <jacky@cs.umanitoba.ca> Tue Jan  8 23:20:07 CST 2013
 *
 */

#include "configuration.h"

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

  // Serial options
  if ( config.device_serial != "" )
    {
      os << "serial_device" << " = " << config.device_serial << std::endl
	 << "baudrate" << " = " << config.baudrate << std::endl;
    }
  ;
  return os;
}

