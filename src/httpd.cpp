/**
 * Jacky Baltes <jacky@cs.umanitoba.ca> Tue Jan  8 22:55:42 CST 2013
 */


#include <ostream>
#include <fstream>
#include <iostream>
#include <string>
#include <cstdlib>
#include <sstream>

#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <inttypes.h>

#include "httpd.h"
#include "httpdthread.h"
#include "videostream.h"
#include "configuration.h"
#include "globals.h"

HTTPD::HTTPD( unsigned int http_port,
	      char const * http_addr,
	      char const * credentials,
	      char const * docroot,
	      char const * index,
	      struct Command const commands[])
{
  this->http_port = http_port;
  this->http_addr = http_addr;
  this->credentials = credentials;
  this->docroot = docroot;
  this->index = index;
  this->commands = commands;

  //  HTTPDThread * thread = new HTTPDThread( this );

  // Start the server thread here?
  //thread->StartAndDetach();
}


