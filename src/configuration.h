/* 
 * A class that encapsulates all configuration information
 * Jacky Baltes <jacky@cs.umanitoba.ca> Tue Jan  8 23:20:07 CST 2013
 *
 */

#ifndef __CONFIGURATION_H__
#define __CONFIGURATION_H__

#include <string>
#include <vector>
#include <ostream>
#include <boost/program_options.hpp>

using namespace std;
namespace po = boost::program_options;

class Configuration 
{
 public:
  Configuration( );

 public:
  void UpdateConfiguration( po::variables_map const & vm );

 public:
  void UpdateConfiguration( std::string configStr );

 public:
  void UpdateConfiguration( std::istream & iconfig );

 public:
  // General options
  unsigned int subsample;

  // Camera options
  string device_video;
  unsigned int width;
  unsigned int height;
  unsigned int depth;
  int brightness;
  int contrast;
  int saturation;
  int sharpness;
  int gain;

  // HTTPD options
  unsigned int http_port;
  string http_addr;
  string docroot;
  string index;

  // Colour options
  vector<string> colours;
  
  // Serial options
  string device_serial;
  string baudrate;

  friend std::ostream & operator<<(std::ostream & os, Configuration const & config );

 public:
  po::options_description options;

};

#endif /* __CONFIGURATION_H__ */
