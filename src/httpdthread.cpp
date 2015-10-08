/*
 * Class to encapsulate a server thread
 * Jacky Baltes <jacky@cs.umanitoba.ca> Tue Jan 15 02:39:52 CST 2013
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <unistd.h>

#include "httpdthread.h"
#include "httpd.h"

HTTPDThread::HTTPDThread( HTTPD * server )
  : fd(-1),
    server(server)
{
}


void *
HTTPDThread::RunTrampoline( void * arg )
{
  HTTPDThread * t = (HTTPDThread *) arg;
  return t->Run();
}

int
HTTPDThread::StartAndDetach( void )
{
  int err;

  err = pthread_create(& id, NULL, HTTPDThread::RunTrampoline, this );
  if ( err != 0 )
    {
      perror("pthread_create:");
      std::exit(EXIT_FAILURE);
    }
  
  err = pthread_detach( id );
  if ( err != 0 )
    {
      perror("pthread_create:");
      std::exit(EXIT_FAILURE);
    }
  return 0;
}

void
HTTPDThread::CleanUpTrampoline( void * arg )
{
  HTTPDThread * thread = (HTTPDThread *) arg;

  thread->CleanUp();
}

/******************************************************************************
Description.: This function cleans up ressources allocated by the server_thread
Input Value.: arg is not used
Return Value: -
******************************************************************************/
void 
HTTPDThread::CleanUp( ) 
{
  OPRINT("cleaning up ressources allocated by server thread #%02ld\n", id );
  
  if ( fd >= 0 )
    {
      ::close( fd );
    }
}
