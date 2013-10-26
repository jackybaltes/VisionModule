/* 
 * Class that encapsulates global data
 * Jacky Baltes <jacky@cs.umanitoba.ca> Tue Jan 15 00:09:43 CST 2013
 */

#include <inttypes.h>
#include <pthread.h>

class FrameBuffer;
class VideoStream;
class Serial;
class HTTPD;
class HTTPDThread;
class HTTPDServerThread;
class Configuration;

class Globals
{
 public:
  static Globals * GetGlobals( void );

 private:
  Globals( void );

 private:
  static Globals * globals;
  
 protected:
  pthread_mutex_t db;

 protected:
  pthread_mutex_t controls_mutex;
  
 public:
  pthread_cond_t db_update;

 private:
  uint8_t * buf;

 public:
  inline uint8_t * GetBuffer( void ) { return buf; };

 public:
  inline void SetBuffer( uint8_t * buf, unsigned int size ) { this->buf = buf; this->size = size; };

 private:
  volatile unsigned int size;

 public:
  inline unsigned int GetSize( void ) { return size; };

 public:
  inline void SetSize( unsigned int size ) { this->size = size; };

 public:
  int LockBuffer( void );
  
 public:
  int UnlockBuffer( void );

 public:
  int BroadcastBuffer( void );

 public:
  int CondWaitForBuffer( void );

 public:
  int CompressImageToJpeg( FrameBuffer * img );

 private:
  VideoStream * video;

 public:
  inline VideoStream * GetVideo( void ) { return video; };

 public:
  inline void SetVideo( VideoStream * video ) { this->video = video; };

 private:
  Serial * serial;

 public:
  inline Serial * GetSerial( void ) { return serial; };

 public:
  inline void SetSerial( Serial * serial ) { this->serial = serial; };

 private:
  HTTPD * server;

 public:
  inline void SetHTTPDServer( HTTPD * server ) { this->server = server; };

 public:
  inline HTTPD * GetHTTPDServer( void ) { return server; };

 private:
  HTTPDServerThread * thread;

 public:
  inline HTTPDServerThread * GetHTTPDServerThread( void ) { return thread; };

 public:
  inline void SetHTTPDServerThread( HTTPDServerThread * thread ) { this->thread = thread; };

 private:
  volatile bool clientRequest;

 public:
  inline bool GetClientRequest( void ) { return static_cast<bool>( clientRequest ); };
  
 public:
  inline void SetClientRequest( bool clientRequest ) { this->clientRequest = clientRequest; };

 public:
  void UpdateRunningConfiguration( Configuration const * cfg );
};
