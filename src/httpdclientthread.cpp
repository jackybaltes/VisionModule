/*
 * HTTPDClientThread
 * Jacky Baltes <jacky@cs.umanitoba.ca> Tue Jan 15 22:29:39 CST 2013
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>

#include <inttypes.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "httpd.h"
#include "httpdclientthread.h"
#include "globals.h"
#include "videostream.h"
#include "configuration.h"

HTTPDClientThread::HTTPDClientThread( HTTPD * server, int fd )
  : HTTPDThread(server)
{
  this->fd = fd;
}

/******************************************************************************
Description.: Serve a connected TCP-client. This thread function is called
              for each connect of a HTTP client like a webbrowser. It determines
              if it is a valid HTTP request and dispatches between the different
              response options.
Input Value.: arg is the filedescriptor and server-context of the connected TCP
              socket. It must have been allocated so it is freeable by this
              thread function.
Return Value: always NULL
******************************************************************************/
void * 
HTTPDClientThread::Run( void ) 
{
  int cnt;
  char buffer[BUFFER_SIZE]={0}, *pb=buffer;
  iobuffer iobuf;
  request req;
  //  cfd lcfd; /* local-connected-file-descriptor */
  char const * s;
  Globals * glob = Globals::GetGlobals();

  /* initializes the structures */
  init_iobuffer(&iobuf);
  init_request(&req);
  
  /* What does the client want to receive? Read the request. */
  memset(buffer, 0, sizeof(buffer));
  if ( (cnt = _readline( &iobuf, buffer, sizeof(buffer)-1, 5)) == -1 ) 
    {
      close(fd);
      return NULL;
    }
  
  /* determine what to deliver */
  if ( strstr(buffer, "GET /?action=snapshot") != NULL ) 
    {
      req.type = A_SNAPSHOT;
    }
  else if ( strstr(buffer, "GET /?action=stream") != NULL ) 
    {
      req.type = A_STREAM;
    }
  else if ( strstr(buffer, "GET /?action=command") != NULL ) 
    {
      int len;
      req.type = A_COMMAND;
      
      /* advance by the length of known string */
      if ( (pb = strstr(buffer, "GET /?action=command")) == NULL ) 
	{
	  DBG("HTTP request seems to be malformed\n");
	  send_error( 400, "Malformed HTTP request");
	  close(fd);
	  return NULL;
	}
      pb += strlen("GET /?action=command");
      
      /* only accept certain characters */
      len = MIN(MAX(strspn(pb, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_-=&1234567890.{}"), 0), 100);
      req.parameter = (char*)malloc(len+1);
      if ( req.parameter == NULL ) {
	exit(EXIT_FAILURE);
      }
      memset(req.parameter, 0, len+1);
      strncpy(req.parameter, pb, len);
      
      DBG("command parameter (len: %d): \"%s\"\n", len, req.parameter);
    }
  else if (strstr(buffer, "POST " ) != NULL ) 
    {
      DBG("receiving file\n");
      req.type = P_FILE;
      if ( ( pb = strstr(buffer, "POST /") ) == NULL ) 
	{
	  DBG("HTTP request seems to be malformed\n");
	  send_error( 400, "Malformed HTTP request");
	  close(fd);
	  return NULL;
	}
      pb += strlen("POST /");
      size_t len = MIN(MAX(strspn(pb, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ._-1234567890"), 0), 100);
      req.parameter = (char*)malloc(len+1);
      if ( req.parameter == NULL ) 
	{
	  exit(EXIT_FAILURE);
	}
    
      memset(req.parameter, 0, len+1);
      strncpy(req.parameter, pb, len);
    }
  else if (strstr(buffer, "GET " ) != NULL )  
    {
      DBG("try to serve a file\n");
      req.type = A_FILE;
      
      if ( (pb = strstr(buffer, "GET /")) == NULL ) 
	{
	  DBG("HTTP request seems to be malformed\n");
	  send_error( 400, "Malformed HTTP request");
	  close(fd);
	  return NULL;
	}
      
      pb += strlen("GET /");
      size_t len = MIN(MAX(strspn(pb, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ._-1234567890"), 0), 100);
      req.parameter = (char*)malloc(len+1);
      if ( req.parameter == NULL ) 
	{
	  exit(EXIT_FAILURE);
	}
    
      memset(req.parameter, 0, len+1);
      strncpy(req.parameter, pb, len);
    }
  DBG("parameter (len: %d): \"%s\"\n", len, req.parameter);
  
  /*
   * parse the rest of the HTTP-request
   * the end of the request-header is marked by a single, empty line with "\r\n"
   */
  do {
    memset(buffer, 0, sizeof(buffer));
    
    if ( (cnt = _readline( &iobuf, buffer, sizeof(buffer)-1, 5)) == -1 ) 
      {
	free_request(&req);
	close(fd);
	return NULL;
      }
    
    if ( strstr(buffer, "User-Agent: ") != NULL ) 
      {
	req.client = strdup(buffer+strlen("User-Agent: "));
      }
    else if ( strstr(buffer, "Authorization: Basic ") != NULL ) 
      {
	req.credentials = strdup(buffer+strlen("Authorization: Basic "));
	decodeBase64(req.credentials);
	DBG("username:password: %s\n", req.credentials);
      } 
    else if ( ( s = strstr( buffer, "Content-Length: " ) ) != NULL ) 
      {
	s = s + strlen("Content-Length: ");
	req.content_length = atoi(s);
      }
    
  } while( cnt > 2 && !(buffer[0] == '\r' && buffer[1] == '\n') );
  
  /* check for username and password if parameter -c was given */
  if ( server->GetCredentials() != NULL ) {
    if ( req.credentials == NULL || strcmp( GetServer()->GetCredentials(), req.credentials) != 0 ) {
      DBG("access denied\n");
      send_error( 401, "username and password do not match to configuration");
      close(fd);
      if ( req.parameter != NULL ) free(req.parameter);
      if ( req.client != NULL ) free(req.client);
      if ( req.credentials != NULL ) free(req.credentials);
      return NULL;
    }
    DBG("access granted\n");
  }
  
  glob->SetClientRequest( true );
  
  /* now it's time to answer */
  switch ( req.type ) {
  case A_SNAPSHOT:
      DBG("Request for snapshot\n");
      send_snapshot();
      break;
  case A_STREAM:
    DBG("Request for stream\n");
    send_stream();	  
    break;
  case A_COMMAND:
    if ( GetServer()->GetCommands() == 0 ) {
      send_error( 501, "this server is configured to not accept commands");
      break;
    }
    ParseCommand( req.parameter);
    break;
  case A_FILE:
    if ( GetServer()->GetDocRoot() == NULL )
      {
	send_error( 501, "no www-folder configured");
      }
    else
      {
	SendFile( req.parameter);
      }
    break;
  case P_FILE:
    if ( GetServer()->GetDocRoot() == NULL )
      {
	send_error( 501, "no www-folder configured");
      }
    else
      {
	ReceiveFile( & iobuf, req.parameter, req.content_length);
      }
    break;    
  default:
    DBG("unknown request\n");
  }
  
  close(fd);
  free_request(&req);
  
  DBG("leaving HTTP client thread\n");
  return NULL;
}

/******************************************************************************
Description.: initializes the iobuffer structure properly
Input Value.: pointer to already allocated iobuffer
Return Value: iobuf
******************************************************************************/
void 
HTTPDClientThread::init_iobuffer( iobuffer * iobuf) 
{
  memset(iobuf->buffer, 0, sizeof(iobuf->buffer));
  iobuf->level = 0;
}

/******************************************************************************
Description.: initializes the request structure properly
Input Value.: pointer to already allocated req
Return Value: req
******************************************************************************/
void HTTPDClientThread::init_request(request *req) {
  req->type        = A_UNKNOWN;
  req->parameter   = NULL;
  req->client      = NULL;
  req->credentials = NULL;
}

/******************************************************************************
Description.: If strings were assigned to the different members free them
              This will fail if strings are static, so always use strdup().
Input Value.: req: pointer to request structure
Return Value: -
******************************************************************************/
void HTTPDClientThread::free_request(request *req) {
  if ( req->parameter != NULL ) free(req->parameter);
  if ( req->client != NULL ) free(req->client);
  if ( req->credentials != NULL ) free(req->credentials);
}

/******************************************************************************
Description.: read with timeout, implemented without using signals
              tries to read len bytes and returns if enough bytes were read
              or the timeout was triggered. In case of timeout the return
              value may differ from the requested bytes "len".
Input Value.: * fd.....: fildescriptor to read from
              * iobuf..: iobuffer that allows to use this functions from multiple
                         threads because the complete context is the iobuffer.
              * buffer.: The buffer to store values at, will be set to zero
                         before storing values.
              * len....: the length of buffer
              * timeout: seconds to wait for an answer
Return Value: * buffer.: will become filled with bytes read
              * iobuf..: May get altered to save the context for future calls.
              * func().: bytes copied to buffer or -1 in case of error
******************************************************************************/
int 
HTTPDClientThread::_read( iobuffer *iobuf, void *buffer, size_t len, int timeout) 
{
  int copied=0, rc, i;
  fd_set fds;
  struct timeval tv;

  memset(buffer, 0, len);

  while ( (copied < (int)len) ) 
    {
      i = MIN(iobuf->level, (int)len-copied);
      memcpy(static_cast<void *>( static_cast<uint8_t *>(buffer)+copied), iobuf->buffer+IO_BUFFER-iobuf->level, i);
      
      iobuf->level -= i;
      copied += i;
      if ( copied >= len )
	{
	  return copied;
	}

      /* select will return in case of timeout or new data arrived */
      tv.tv_sec = timeout;
      tv.tv_usec = 0;
      FD_ZERO(&fds);
      FD_SET(fd, &fds);
      if ( (rc = select(fd+1, &fds, NULL, NULL, &tv)) <= 0 ) 
	{
	  if ( rc < 0)
	    {
	      exit(EXIT_FAILURE);
	    }

	  /* this must be a timeout */
	  return copied;
	}

      init_iobuffer(iobuf);
      
      /*
       * there should be at least one byte, because select signalled it.
       * But: It may happen (very seldomly), that the socket gets closed remotly between
       * the select() and the following read. That is the reason for not relying
       * on reading at least one byte.
       */
      if ( (iobuf->level = read(fd, &iobuf->buffer, IO_BUFFER)) <= 0 ) 
	{
	  /* an error occured */
	  return -1;
	}

      /* align data to the end of the buffer if less than IO_BUFFER bytes were read */
      memmove(iobuf->buffer+(IO_BUFFER-iobuf->level), iobuf->buffer, iobuf->level);
    }
  return 0;
}

/******************************************************************************
Description.: Read a single line from the provided fildescriptor.
              This funtion will return under two conditions:
              * line end was reached
              * timeout occured
Input Value.: * fd.....: fildescriptor to read from
              * iobuf..: iobuffer that allows to use this functions from multiple
                         threads because the complete context is the iobuffer.
              * buffer.: The buffer to store values at, will be set to zero
                         before storing values.
              * len....: the length of buffer
              * timeout: seconds to wait for an answer
Return Value: * buffer.: will become filled with bytes read
              * iobuf..: May get altered to save the context for future calls.
              * func().: bytes copied to buffer or -1 in case of error
******************************************************************************/
/* read just a single line or timeout */
int 
HTTPDClientThread::_readline( iobuffer *iobuf, void *buffer, size_t len, int timeout) {
  char c='\0', *out=(char*)buffer;
  int i;

  memset(buffer, 0, len);

  for ( i=0; ( i<len ) && ( c != '\n'); i++ ) 
    {
      if ( _read( iobuf, &c, 1, timeout) <= 0 ) 
	{
	  /* timeout or error occured */
	  return -1;
	}
      *out++ = c;
    }

  return i;
}

/******************************************************************************
Description.: Decodes the data and stores the result to the same buffer.
              The buffer will be large enough, because base64 requires more
              space then plain text.
Hints.......: taken from busybox, but it is GPL code
Input Value.: base64 encoded data
Return Value: plain decoded data
******************************************************************************/
void HTTPDClientThread::decodeBase64(char *data) {
  const unsigned char *in = (const unsigned char *)data;
  /* The decoded size will be at most 3/4 the size of the encoded */
  unsigned ch = 0;
  int i = 0;

  while (*in) {
    int t = *in++;

    if (t >= '0' && t <= '9')
      t = t - '0' + 52;
    else if (t >= 'A' && t <= 'Z')
      t = t - 'A';
    else if (t >= 'a' && t <= 'z')
      t = t - 'a' + 26;
    else if (t == '+')
      t = 62;
    else if (t == '/')
      t = 63;
    else if (t == '=')
      t = 0;
    else
      continue;

    ch = (ch << 6) | t;
    i++;
    if (i == 4) {
      *data++ = (char) (ch >> 16);
      *data++ = (char) (ch >> 8);
      *data++ = (char) ch;
      i = 0;
    }
  }
  *data = '\0';
}

/******************************************************************************
Description.: Send a complete HTTP response and a single JPG-frame.
Input Value.: fildescriptor fd to send the answer to
Return Value: -
******************************************************************************/
void 
HTTPDClientThread::send_snapshot( void ) 
{
  unsigned char *frame=NULL;
  int frame_size=0;
  char buffer[BUFFER_SIZE] = {0};
  Globals * glob = Globals::GetGlobals();

  /* wait for a fresh frame */
  glob->CondWaitForBuffer();

  /* read buffer */
  frame_size = glob->GetSize( );

  /* allocate a buffer for this single frame */
  if ( (frame = (uint8_t *)malloc(frame_size+1)) == NULL ) 
    {
      glob->UnlockBuffer();
      send_error( 500, "not enough memory");
      return;
    }

  memcpy(frame, (void const *) glob->GetBuffer(), frame_size);
  DBG("got frame (size: %d kB)\n", frame_size/1024);

  glob->UnlockBuffer();

  /* write the response */
  sprintf(buffer, "HTTP/1.0 200 OK\r\n" \
                  STD_HEADER \
                  "Content-type: image/jpeg\r\n" \
                  "\r\n");

  /* send header and image now */
  if( write(fd, buffer, strlen(buffer)) < 0 ) {
    free(frame);
    return;
  }
  write(fd, frame, frame_size);

  free(frame);
}

/******************************************************************************
Description.: Send a complete HTTP response and a stream of JPG-frames.
Input Value.: fildescriptor fd to send the answer to
Return Value: -
******************************************************************************/
void HTTPDClientThread::send_stream( void ) {
  unsigned char *frame=NULL, *tmp=NULL;
  int frame_size=0, max_frame_size=0;
  char buffer[BUFFER_SIZE] = {0};
  Globals * glob = Globals::GetGlobals();

  DBG("preparing header\n");

  sprintf(buffer, "HTTP/1.0 200 OK\r\n" \
                  STD_HEADER \
                  "Content-Type: multipart/x-mixed-replace;boundary=" BOUNDARY "\r\n" \
                  "\r\n" \
                  "--" BOUNDARY "\r\n");

  if ( write(fd, buffer, strlen(buffer)) < 0 ) {
    free(frame);
    return;
  }

  DBG("Headers send, sending stream now\n");

  while ( 1 /*!pglobal->stop*/ ) {

    /* wait for fresh frames */
    glob->CondWaitForBuffer();

    /* read buffer */
    frame_size = glob->GetSize();

    /* check if framebuffer is large enough, increase it if necessary */
    if ( frame_size > max_frame_size ) 
      {
	DBG("increasing buffer size to %d\n", frame_size);
	
	max_frame_size = frame_size+TEN_K;
	if ( (tmp = (unsigned char*)realloc(frame, max_frame_size)) == NULL ) 
	  {
	    free(frame);
	    glob->UnlockBuffer();
	    send_error( 500, "not enough memory");
	    return;
	  }
	
	frame = tmp;
      }

    memcpy(frame, (void const *)glob->GetBuffer(), frame_size);
    DBG("got frame (size: %d kB)\n", frame_size/1024);

    glob->UnlockBuffer();

    /*
     * print the individual mimetype and the length
     * sending the content-length fixes random stream disruption observed
     * with firefox
     */
    sprintf(buffer, "Content-Type: image/jpeg\r\n" \
                    "Content-Length: %d\r\n" \
                    "\r\n", frame_size);
    DBG("sending intemdiate header\n");
    if ( write(fd, buffer, strlen(buffer)) < 0 ) break;

    DBG("sending frame\n");
    if( write(fd, frame, frame_size) < 0 ) break;

    DBG("sending boundary\n");
    sprintf(buffer, "\r\n--" BOUNDARY "\r\n");
    if ( write(fd, buffer, strlen(buffer)) < 0 ) break;
  }

  free(frame);
}

/******************************************************************************
Description.: Send error messages and headers.
Input Value.: * fd.....: is the filedescriptor to send the message to
              * which..: HTTP error code, most popular is 404
              * message: append this string to the displayed response
Return Value: -
******************************************************************************/
void HTTPDClientThread::send_error( int which, char const * message) {
  char buffer[BUFFER_SIZE] = {0};

  if ( which == 401 ) {
    sprintf(buffer, "HTTP/1.0 401 Unauthorized\r\n" \
                    "Content-type: text/plain\r\n" \
                    STD_HEADER \
                    "WWW-Authenticate: Basic realm=\"MJPG-Streamer\"\r\n" \
                    "\r\n" \
                    "401: Not Authenticated!\r\n" \
                    "%s", message);
  } else if ( which == 404 ) {
    sprintf(buffer, "HTTP/1.0 404 Not Found\r\n" \
                    "Content-type: text/plain\r\n" \
                    STD_HEADER \
                    "\r\n" \
                    "404: Not Found!\r\n" \
                    "%s", message);
  } else if ( which == 500 ) {
    sprintf(buffer, "HTTP/1.0 500 Internal Server Error\r\n" \
                    "Content-type: text/plain\r\n" \
                    STD_HEADER \
                    "\r\n" \
                    "500: Internal Server Error!\r\n" \
                    "%s", message);
  } else if ( which == 400 ) {
    sprintf(buffer, "HTTP/1.0 400 Bad Request\r\n" \
                    "Content-type: text/plain\r\n" \
                    STD_HEADER \
                    "\r\n" \
                    "400: Not Found!\r\n" \
                    "%s", message);
  } else {
    sprintf(buffer, "HTTP/1.0 501 Not Implemented\r\n" \
                    "Content-type: text/plain\r\n" \
                    STD_HEADER \
                    "\r\n" \
                    "501: Not Implemented!\r\n" \
                    "%s", message);
  }

  write(fd, buffer, strlen(buffer));
}

/******************************************************************************
Description.: Send HTTP header and copy the content of a file. To keep things
              simple, just a single folder gets searched for the file. Just
              files with known extension and supported mimetype get served.
              If no parameter was given, the file "index.html" will be copied.
Input Value.: * fd.......: filedescriptor to send data to
              * parameter: string that consists of the filename
              * id.......: specifies which server-context is the right one
Return Value: -
******************************************************************************/
void 
HTTPDClientThread::SendFile( char const * parameter) 
{
  char buffer[BUFFER_SIZE] = {0};
  char const * extension, *mimetype=NULL;
  int i, lfd;
  Globals * glob = Globals::GetGlobals();

  /* in case no parameter was given */
  if ( parameter == NULL || strlen(parameter) == 0 )
    {
      parameter = GetServer()->GetIndex();
    }

  /* find file-extension */
  if ( (extension = strstr(parameter, ".")) == NULL ) {
    send_error( 400, "No file extension found");
    return;
  }

  /* determine mime-type */
  for ( i=0; i < LENGTH_OF(mimetypes); i++ ) {
    if ( strcmp(mimetypes[i].dot_extension, extension) == 0 ) {
      mimetype = (char *)mimetypes[i].mimetype;
      break;
    }
  }

  /* in case of unknown mimetype or extension leave */
  if ( mimetype == NULL ) {
    send_error( 404, "MIME-TYPE not known");
    return;
  }

  /* now filename, mimetype and extension are known */
  DBG("trying to serve file \"%s\", extension: \"%s\" mime: \"%s\"\n", parameter, extension, mimetype);

  /* build the absolute path to the file */
  strncat(buffer, GetServer()->GetDocRoot(), sizeof(buffer)-1);
  strncat(buffer, parameter, sizeof(buffer)-strlen(buffer)-1);
  
  if ( !strcmp( parameter, "__config__.cfg" ) )
    {
      std::string configString = glob->GetVideo()->ReadRunningConfiguration( );
      if ( configString != "" )
	{
	  stringstream os;

	  os << "HTTP/1.0 200 OK\r\n"	
	     << "Content-type: %s\r\n"
	     << "Content-Disposition: attachment\r\n"
	     << STD_HEADER << "\r\n" 
	     << configString
	     << "\r\n";

	  if ( write(fd, os.str().c_str(), strlen( os.str().c_str() ) ) < 0 )
	    {
	      return;
	    }
	}
    }
  else
    {
      /* try to open that file */
      if ( (lfd = open(buffer, O_RDONLY)) < 0 ) 
	{
	  DBG("file %s not accessible\n", buffer);
	  send_error( 404, "Could not open file");
	  return;
	}

      DBG("opened file: %s\n", buffer);
      
      char const * disposition;
      
      if ( ! strcmp( extension, ".cfg" ) ) 
	{
	  disposition = "Content-Disposition: attachment\r\n";
	}
      else
	{
	  disposition = "";
	}
      
      /* prepare HTTP header */
      sprintf(buffer, "HTTP/1.0 200 OK\r\n"	\
	      "Content-type: %s\r\n"		\
	      "%s"				\
	      STD_HEADER			\
	      "\r\n", mimetype, disposition);
      i = strlen(buffer);
      /* first transmit HTTP-header, afterwards transmit content of file */
      
  
      do {
	if ( write(fd, buffer, i) < 0 ) {
	  close(lfd);	  
	  return;
	}
      } while ( (i=read(lfd, buffer, sizeof(buffer))) > 0 );
      
      /* close file, job done */
      close(lfd);
    }
}

/******************************************************************************
Description.: Perform a command specified by parameter. Send response to fd.
Input Value.: * fd.......: filedescriptor to send HTTP response to.
              * parameter: contains the command and value as string.
              * id.......: specifies which server-context to choose.
Return Value: -
******************************************************************************/
void 
HTTPDClientThread::ParseCommand( char const * parameter) 
{
  Globals * glob = Globals::GetGlobals();
  char tmpCommand[256];
  char tmp[256];
  char const * pcommand;
  char response[1024];

  char buffer[BUFFER_SIZE] = {0};

  /* sanity check of parameter-string */
  if ( parameter == NULL || strlen(parameter) >= 256 || strlen(parameter) == 0 ) {
    DBG("parameter string looks bad\n");
    send_error( 400, "Parameter-string of command does not look valid.");
    return;
  }
  struct Command const * icom = GetServer()->GetCommands();

  int done = -1;

  while( ( icom != NULL ) && ( icom->command != NULL ) )
    {
      int ret = icom->command( glob->GetVideo(), parameter, response, sizeof(response) );
      if ( ret > 0 )
	{
	  done = ret;
	  break;
	}
      else if ( ret == 0 )
	{
	  done = ret;
	}
      icom++;
    }

  if ( done >= 0 )
    {
      /* Send HTTP-response */
      sprintf(buffer, "HTTP/1.0 200 OK\r\n"		\
	      "Content-type: text/plain\r\n"		\
	      STD_HEADER				\
	      "\r\n"					\
	      "%s", response);
      
      write(fd, buffer, strlen(buffer));
    }
  else
    {
      DBG("Unknown command %s", tmpCommand );
    }
  //  if (ccommand != NULL) free(command);
}

void 
HTTPDClientThread::ReceiveFile( iobuffer * iobuf, char const * parameter, size_t length ) 
{
  char content[256];
  char mark[80];
  std::ostream * os;
  std::ostringstream oss;
  std::ofstream ofile;
  
  DBG("Receiving file %s of size %d", parameter, length );

  if ( !strcmp( parameter, "__config__.cfg" ) )
    {
      os = & oss;
    }
  else
    {
      /* build the absolute path to the file */
      char buffer[BUFFER_SIZE] = {0};
      strncat(buffer, GetServer()->GetDocRoot(), sizeof(buffer)-1);
      strncat(buffer, parameter, sizeof(buffer)-strlen(buffer)-1);
      
      ofile.open( buffer, ios::trunc );
      if ( ! ofile.is_open() ) 
	{
      	  DBG("file %s not writeable\n", buffer);
	  send_error( 404, "Could not open file");
	  return;
	}
      os = & ofile;
    }
  
  int total = 0;
  int in;
  unsigned int row = 0;
  mark[0] = '\0';
  int doWrite;
  int header = 1;

  while( total < length )
    {
      int rem = MIN( sizeof(content) - 1, length - total );
      doWrite = 1;
      if( ( in = _readline( iobuf, content, rem, 5) ) > 0 )
	{	  
	  if ( ( row == 0 ) && ( !strncmp("-----------------------------", content, strlen("-----------------------------") ) ) ) 
	    {
	      strncpy( mark, content, in );
	      mark[in-2] = '-';
	      mark[in-1] = '-';
	      mark[in] = '\r';
	      mark[in+1] = '\n';

	      doWrite = 0;
	    }
	  
	  if ( ( header ) && ( ! strcmp(content,"\r\n") ) )
	    {
	      header = 0;
	    }
	  total = total + in;

	  if ( ( total == length ) && ( !strncmp( content, mark, strlen( mark ) ) ) )
	    {
	      doWrite = 0;
	    }

	  if ( ( doWrite ) && ( ! header ) )
	    {
	      (* os ).write( content, strlen( content ) );
	    }
	  row++;
	}
    }
  DBG("Read and saved %d bytes\n", total );

  std::ofstream * of;
  std::ostringstream * ossp;
  if ( ( of = dynamic_cast<std::ofstream *>(os) ) != 0 ) 
    {
      (*of).close();
    }
  else if ( ( ossp = dynamic_cast<std::ostringstream *>(os) ) != 0 )
    {
      std::string configStr = (*ossp).str();
      Configuration config;

      config.UpdateConfiguration( configStr );
      std::cout << config;
      Globals * glob = Globals::GetGlobals();
      glob->UpdateRunningConfiguration( & config );
    }
}



