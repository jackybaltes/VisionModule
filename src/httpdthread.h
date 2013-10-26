/*
 * Class to encapsulate a server thread
 * Jacky Baltes <jacky@cs.umanitoba.ca> Tue Jan 15 02:39:52 CST 2013
 */

#ifndef __HTTPDTHREAD_H__
#define __HTTPDTHREAD_H__

#include <pthread.h>

class HTTPD;

class HTTPDThread
{
 public:
  HTTPDThread( HTTPD * server );

 protected:
  int fd;

 public:
  virtual inline int GetSD( void ) { return fd; };

 protected:
  pthread_t id;

 public:
  virtual inline pthread_t GetID( void ) { return id; };

 public:
  virtual inline void SetID( pthread_t id ) { this->id = id; };

 protected:
  HTTPD * server;

 public:
  virtual inline HTTPD * GetServer( void ) { return server; };  

 public:
  static void * RunTrampoline( void * arg );

 public:
  virtual void * Run( void ) = 0;

 public:
  int StartAndDetach( void );

 public:
  static void CleanUpTrampoline( void  * arg );

 public:
  virtual void CleanUp( void );
};


#endif /* __HTTPDTHREAD_H__ */
