/**
 * Jacky Baltes <jacky@cs.umanitoba.ca> Tue Jan  8 22:55:42 CST 2013
 */

#ifndef __HTTPD_H__
#define __HTTPD_H__

#include <sys/types.h>

#define IO_BUFFER 256
#define BUFFER_SIZE 1024

/* the boundary is used for the M-JPEG stream, it separates the multipart stream of pictures */
#define BOUNDARY "boundarydonotcross"

/*
 * this defines the buffer size for a JPG-frame
 * selecting to large values will allocate much wasted RAM for each buffer
 * selecting to small values will lead to crashes due to to small buffers
 */
#define MAX_FRAME_SIZE (256*1024)
#define TEN_K (10*1024)


#define ABS(a) (((a) < 0) ? -(a) : (a))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define LENGTH_OF(x) (sizeof(x)/sizeof(x[0]))

#ifdef DEBUG
#define DBG(...) fprintf(stdout, " DBG(%s, %s(), %d): ", __FILE__, __FUNCTION__, __LINE__); fprintf(stdout, __VA_ARGS__)
#else
#define DBG(...)
#endif

#define LOG(...) { char _bf[1024] = {0}; snprintf(_bf, sizeof(_bf)-1, __VA_ARGS__); fprintf(stdout, "%s", _bf); syslog(LOG_INFO, "%s", _bf); }

#define OUTPUT_PLUGIN_PREFIX " o: "
#define OPRINT(...) { char _bf[1024] = {0}; snprintf(_bf, sizeof(_bf)-1, __VA_ARGS__); fprintf(stdout, "%s", OUTPUT_PLUGIN_PREFIX); fprintf(stdout, "%s", _bf); syslog(LOG_INFO, "%s", _bf); }

class VideoStream;
class Serial;
class HTTPD;
class HTTPDThread;

enum CommandReturn
  {
    COMMAND_ERR_OK = 1,
    COMMAND_ERR_CONTINUE = 0,
    COMMAND_ERR_COMMAND = -1,
    COMMAND_ERR_PARAMETER = -2
  };

struct Command 
{
  int (* command)( VideoStream * video, char const * parameter, char * response, unsigned int maxResponseLength );
};

#if 0
/* context of each server thread */
typedef struct {
  int sd;
  pthread_t threadID;

  HTTPD * server;
} context;
#endif

/* the webserver determines between these values for an answer */
typedef enum { A_UNKNOWN, A_SNAPSHOT, A_STREAM, A_COMMAND, A_FILE, P_FILE } answer_t;

/*
 * the client sends information with each request
 * this structure is used to store the important parts
 */
typedef struct {
  answer_t type;
  char *parameter;
  char *client;
  char *credentials;
  size_t content_length;
} request;

#if 0
/*
 * this struct is just defined to allow passing all necessary details to a worker thread
 * "cfd" is for connected/accepted filedescriptor
 */
typedef struct 
{
  HTTPDThread * thread;
  int fd;
} cfd;
#endif

/*
 * Standard header to be send along with other header information like mimetype.
 *
 * The parameters should ensure the browser does not cache our answer.
 * A browser should connect for each file and not serve files from his cache.
 * Using cached pictures would lead to showing old/outdated pictures
 * Many browser seem to ignore, or at least not always obey those headers
 * since i observed caching of files from time to time.
 */
#define STD_HEADER "Connection: close\r\n" \
                   "Server: VisionModule/1.0\r\n" \
                   "Cache-Control: no-store, no-cache, must-revalidate, pre-check=0, post-check=0, max-age=0\r\n" \
                   "Pragma: no-cache\r\n" \
                   "Expires: Mon, 1 Jan 2000 12:00:00 GMT\r\n"

/*
 * Only the following fileypes are supported.
 *
 * Other filetypes are simply ignored!
 * This table is a 1:1 mapping of files extension to a certain mimetype.
 */
static const struct {
  const char * dot_extension;
  const char * mimetype;
} mimetypes[] = {
  { ".html", "text/html" },
  { ".htm",  "text/html" },
  { ".css",  "text/css" },
  { ".js",   "text/javascript" },
  { ".txt",  "text/plain" },
  { ".cfg",  "text/plain" },
  { ".jpg",  "image/jpeg" },
  { ".jpeg", "image/jpeg" },
  { ".png",  "image/png"},
  { ".gif",  "image/gif" },
  { ".ico",  "image/x-icon" },
  { ".swf",  "application/x-shockwave-flash" },
  { ".cab",  "application/x-shockwave-flash" },
  { ".jar",  "application/java-archive" }
};

class HTTPD
{
 public:
  HTTPD( unsigned int http_port,
	 char const * http_addr,
	 char const * credentials,
	 char const * docroot,
	 char const * index,
	 struct Command const commands[]);


 private:
    unsigned int http_port;

 public:
    inline unsigned int GetHTTPPort( void ) { return http_port; };

 private:
    char const * http_addr;

 public:
    inline char const * GetHTTPAddr( void ) { return http_addr; };

 private:
    char const * credentials;

 public:
    inline char const * GetCredentials( void ) { return credentials; };

 private:
    char const * docroot;

 public:
    inline char const * GetDocRoot( void ) { return docroot; };

 private:
    char const * index;

 public:
    inline char const * GetIndex( void ) { return index; };

 private:
    struct Command const * commands;   

 public:
    inline struct Command const * GetCommands( void ) { return commands; };

 public:
    static void *server_thread( HTTPD * server, void *arg );

};


#endif /* HTTPD_H_ */




