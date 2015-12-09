/*
 * HTTPDServerThread
 * Jacky Baltes <jacky@cs.umanitoba.ca> Tue Jan 15 20:36:01 CST 2013
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>

#include "globals.h"
#include "httpdthread.h"
#include "httpd.h"
#include "httpdserverthread.h"
#include "httpdclientthread.h"

HTTPDServerThread::HTTPDServerThread( HTTPD * server )
  : HTTPDThread(server)
{
}

/******************************************************************************
Description.: Open a TCP socket and wait for clients to connect. If clients
              connect, start a new thread for each accepted connection.
Input Value.: arg is a pointer to the globals struct
Return Value: always NULL, will only return on exit
******************************************************************************/
void * 
HTTPDServerThread::Run( void ) 
{
  struct sockaddr_in addr;
  struct sockaddr_in client_addr;
  socklen_t addr_len = sizeof(struct sockaddr_in);
  int on;

  Globals * glob = Globals::GetGlobals();

  std::cout << "Setting up signal handler on HTTPDServerThread" << std::endl;
  struct sigaction sa;
  sa.sa_handler = glob->hupSignalHandler;
  
  sigemptyset(& sa.sa_mask);
  sa.sa_flags = SA_NODEFER;

  if ( sigaction(SIGHUP, &sa, NULL  ) == -1 )
    {
      std::cerr << "Error setting up signal handler" << std::endl;
    }

  pthread_cleanup_push( CleanUpTrampoline, (void * ) this );

  /* open socket for server */
  fd = socket(PF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
  if ( fd < 0 ) 
    {
      fprintf(stderr, "socket failed\n");
      exit(EXIT_FAILURE);
    }

  /* ignore "socket already in use" errors */
  on = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
    perror("setsockopt(SO_REUSEADDR) failed");
    exit(EXIT_FAILURE);
  }

  /* perhaps we will use this keep-alive feature oneday */
  /* setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof(on)); */

  /* configure server address to listen to all local IPs */
  memset(&addr, 0, sizeof(addr));

  addr.sin_family = AF_INET;
  addr.sin_port = htons( server->GetHTTPPort() ); 
  int e = inet_pton(AF_INET, server->GetHTTPAddr(), & addr.sin_addr );

  if ( e <= 0 )
    {
      if ( e == 0 )
	{
	  std::cerr << "Unable to parse addresse " << server->GetHTTPAddr() << std::endl;
	  std::exit(2);
	}
      if ( e < 0 )
	{
	  perror("inet_pton failed:");
	  std::exit(2);
	}
    }
    
  if ( bind( fd, (struct sockaddr*)&addr, sizeof(addr)) != 0 ) 
    {
      perror("bind:");
      OPRINT("%s(): bind(%d) failed", __FUNCTION__, server->GetHTTPPort() );
      closelog();
      exit(EXIT_FAILURE);
    }

  /* start listening on socket */
  if ( listen( fd, 10) != 0 ) {
    fprintf(stderr, "listen failed\n");
    exit(EXIT_FAILURE);
  }

  /* create a child for every client that connects */
  while ( 1 /*!pglobal->stop*/ ) {
    // //int *pfd = (int *)malloc(sizeof(int));
    //    cfd *pcfd = (cfd*)malloc(sizeof(cfd));

    //    if (pcfd == NULL) {
    //      fprintf(stderr, "failed to allocate (a very small amount of) memory\n");
    //      exit(EXIT_FAILURE);
    //}
    
    DBG("waiting for clients to connect\n");
    int cfd = accept( fd, (struct sockaddr *)&client_addr, &addr_len);

    HTTPDClientThread * client = new HTTPDClientThread( GetServer(), cfd );

    /* start new thread that will handle this TCP connected client */
    DBG("create thread to handle client that just established a connection\n");
    syslog(LOG_INFO, "serving client: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    if( client->StartAndDetach() != 0 )
      {
	DBG("could not launch another client thread\n");
	::close(fd);
      }
  }

  DBG("leaving server thread, calling cleanup function now\n");
  pthread_cleanup_pop(1);

  return NULL;
}

