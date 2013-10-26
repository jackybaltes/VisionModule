/*
 * HTTPDClientThread
 * Jacky Baltes <jacky@cs.umanitoba.ca> Tue Jan 15 22:29:39 CST 2013
 */

#ifndef __HTTPDCLIENTTHREAD_H__
#define __HTTPDCLIENTTHREAD_H__

#include "httpdthread.h"

/* the iobuffer structure is used to read from the HTTP-client */
typedef struct {
  int level;              /* how full is the buffer */
  char buffer[IO_BUFFER]; /* the data */
} iobuffer;

class HTTPDClientThread : public HTTPDThread
{
 public:
  HTTPDClientThread( HTTPD * server, int fd );

 public:
  void * Run( void );

 public:
    void init_iobuffer(iobuffer *iobuf);

 public:
    void init_request(request *req);

 public:
    void free_request(request *req);

 public:
    int _read( iobuffer *iobuf, void *buffer, size_t len, int timeout);

 public:
    int _readline( iobuffer *iobuf, void *buffer, size_t len, int timeout);

 public:
    void decodeBase64(char *data);

 public:
    void send_snapshot( void );

 public:
    void send_stream( void );

 public:
    void SendFile( char const * parameter);

 public:
    void ReceiveFile( iobuffer * iobuf, char const * parameter, size_t length );

 public:
    void ParseCommand( char const * parameter );

 public:
    void send_error( int which, char const * message );
};

#endif /* __HTTPDCLIENTTHREAD_H__ */
