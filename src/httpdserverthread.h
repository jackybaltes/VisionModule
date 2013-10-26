/*
 * HTTPDServerThread
 * Jacky Baltes <jacky@cs.umanitoba.ca> Tue Jan 15 20:36:01 CST 2013
 */

#ifndef __HTTPDSERVERTHREAD_H__
#define __HTTPDSERVERTHREAD_H__

#include "httpdthread.h"

class HTTPDServerThread : public HTTPDThread
{
 public:
  HTTPDServerThread( HTTPD * server );

 public:
  virtual void * Run( void );

};

#endif
