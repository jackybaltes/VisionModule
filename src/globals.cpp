/* 
 * Class that encapsulates global data
 * Jacky Baltes <jacky@cs.umanitoba.ca> Tue Jan 15 00:09:43 CST 2013
 */

#include <cstdio>
#include <cstdlib>
#include <unistd.h>

#include "globals.h"

Globals * Globals::globals = 0;

/* This routine is not threadsafe, so call in main as early as possible */
Globals * 
Globals::GetGlobals( void )
{
  if ( globals == 0 )
    {
      globals = new Globals();
    }
  return globals;
}

Globals::Globals( void )
  : buf(0),
    size(0),
    video(0),
    serial(0),
    server(0)
{
    if(pthread_mutex_init(& db, NULL) != 0)
    {
      perror("pthread_mutex_init:");
      exit(EXIT_FAILURE);
    }
      
  if(pthread_cond_init(& db_update, NULL) != 0)
    {
      perror("pthread_cond_init:");
      exit(EXIT_FAILURE);
    }
      
  if(pthread_mutex_init(& controls_mutex, NULL) != 0)
    {
      perror("pthread_mutex_init:");
      exit(EXIT_FAILURE);
    }
}

int
Globals::LockBuffer( void )
{
  return pthread_mutex_lock( & db );
}

int
Globals::UnlockBuffer( void )
{
  return pthread_mutex_unlock( & db );
}

int
Globals::BroadcastBuffer( void )
{
  return pthread_cond_broadcast( & db_update );
}

int
Globals::CondWaitForBuffer( void )
{
  return pthread_cond_wait( & db_update, & db );
}

void
Globals::UpdateRunningConfiguration( Configuration const * cfg )
{
  Globals * glob = Globals::GetGlobals();

  
}
