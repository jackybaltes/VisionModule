/*
 * Serial Port Handling Routines
 *
 * Jacky Baltes <jacky@cs.umanitoba.ca> Sat May  6 14:37:03 CDT 2006
 */

using namespace std;

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>


#include "serial.h"

Serial::Serial( string devname, speed_t baudrate )
{
  struct termios newtio;

#ifdef DEBUG
  cout << "Serial::Serial opening device " << devname << '\n';
#endif

  /* Taken from the Serial Programming HOWTO */
  /*
     Open modem device for reading and writing and not as controlling tty
     because we don't want to get killed if linenoise sends CTRL-C.
  */
  fd = ::open( devname.c_str(), O_RDWR | O_NOCTTY );
  if (fd <0)
    {
      perror( "Serial::Serial " );
    }

  tcgetattr( fd, &oldtio ); /* save current serial port settings */
  bzero(&newtio, sizeof(newtio)); /* clear struct for new port settings */

  /*
     BAUDRATE: Set bps rate. You could also use cfsetispeed and cfsetospeed.
     CRTSCTS : output hardware flow control (only used if the cable has all necessary lines. See sect. 7 of Serial-HOWTO)
     CS8     : 8n1 (8bit,no parity,1 stopbit)
     CLOCAL  : local connection, no modem contol
     CREAD   : enable receiving characters
  */
  newtio.c_cflag = baudrate | CS8 | CLOCAL | CREAD;

  /*
    IGNPAR  : ignore bytes with parity errors
    ICRNL   : map CR to NL (otherwise a CR input on the other computer
    will not terminate input)
    otherwise make device raw (no other input processing)
  */
  newtio.c_iflag = IGNPAR; //| ICRNL;

  /*
    Raw output.
  */
  newtio.c_oflag = 0;
  
  /*
    now clean the modem line and activate the settings for the port
  */
  tcflush(fd, TCIFLUSH);
  tcsetattr(fd,TCSANOW,&newtio);

  this->devname = devname;
  this->baudrate = baudrate;
}

Serial::~Serial()
{
  tcflush( fd, TCIFLUSH );
  tcsetattr( fd, TCSANOW, &oldtio );
  close( fd );
}

bool Serial::isValid()
{
  return ( fd > 0 );
}

bool 
Serial::writeSlowly( string cmd )
{
  bool rc = false;

#ifdef DEBUG
  cout << "Serial::writeSlowly trying to write " << cmd;
  cout << "Serial::write isValid is " << isValid() << '\n';
#endif
  if ( isValid() )
    {
      for( unsigned int i = 0; i < cmd.length(); i++ )
	{
	  if ( ! writeChar( cmd.at(i) ) ) 
	    {
	      cerr << "Serial::writeByte failed for character" << cmd.at(i) << '\n';
	    }
	  usleep( 10000 );
	}
      rc = true;
    }
  return rc;
}

bool 
Serial::writeSlowly( __uint8_t buffer[], unsigned int len )
{
  bool rc = false;

#ifdef DEBUG
  //cout << "Serial::writeSlowly trying to write " << cmd;
  cout << "Serial::write isValid is " << isValid() << '\n';
#endif
  if ( isValid() )
    {
      for( unsigned int i = 0; i < len; i++ )
	{
	  if ( ! writeChar( buffer[ i ]) ) 
	    {
	      cerr << "Serial::writeByte failed for character" << i << '\n';
	    }
	  usleep( 10000 );
	}
      rc = true;
    }
  return rc;
}

bool Serial::write( string cmd )
{
  bool rc = false;

#ifndef QT_NO_DEBUG 
  cout << "Serial::write trying to write " << cmd;
  cout << "Serial::write isValid is " << isValid() << '\n';
#endif
  if ( isValid() )
    {
      int len = cmd.length();
      int written = 0;

      if ( ( written = ::write( fd, cmd.c_str(), len ) ) != len )
	{
	  cerr << "Serial::write failed for " << cmd << " wrote " << written << " Bytes\n";
	}
      rc = true;
    }
  return rc;
}

bool Serial::write( char buffer[], unsigned int len ) {

  	bool rc = false;

	#ifdef DEBUG
  		cout << "Serial::write bytes trying to write " << len << " bytes\n";
  		cout << "Serial::write bytes isValid is " << isValid() << '\n';
	#endif
  
	if ( isValid() ) {
      
		ssize_t written = 0;

		if ( ( written = ::write( fd, buffer, static_cast<ssize_t>( len ) ) ) != static_cast<ssize_t>(len) )
			cerr << "Serial::write failed for buffer wrote " << written << " Bytes\n";
      
		rc = true;
    
	}

	#ifdef DEBUG
		cout << "Serial::write bytes returns " << rc << std::endl;
	#endif

	return rc;

}

bool Serial::writeChar( char c )
{
  bool rc = false;

#ifdef DEBUG
  cout << "Serial::writeChar trying to write " << c << '\n';
#endif
  
  if ( c == '\n' )
    {
      c = '\r';
    }
  if ( isValid() )
    {
      if ( ::write( fd, &c, 1 ) == 1 )
	{
	  rc = true;
	}
    }
  return rc;
}

int Serial::readChar( char * p )
{
  char data;
  int ret_val;

  
  if ( 1 == ( ret_val = ::read( fd, &data, 1 ) ) )
    {
      *p = data;
    }
  else
    {
      if ( 0 > ret_val )
	{
	  perror( "Serial::readChar:" );
	}
    }
  return ret_val;
}

unsigned int
Serial::receiveBuffer (__uint8_t * buffer, unsigned int msgLength)
{
  unsigned int len;
  ssize_t res;
  fd_set readfs;		/* file descriptor set */
  int maxfd;
  struct timeval timeOut;
  int nSources;

  FD_ZERO( & readfs );
  FD_SET( fd, &readfs );	/* set testing */
  maxfd = fd + 1;

  timeOut.tv_usec = 5000000;
  timeOut.tv_sec = 5;

  len = 0;

  while (len < msgLength)
    {
      nSources = ::select (maxfd, &readfs, NULL, NULL, &timeOut);
      if (nSources > 0)
	{
	  if (FD_ISSET (fd, &readfs))
	    {
	      res = ::read (fd, &buffer[len], msgLength - len);
	      if (-1 == res)
		{
		  perror ("serial: read from serial port failed:");
		  break;
		}
#ifdef DEBUG
	      std::cout << __FUNCTION__ << " receiveBuffer res = " << res << std::endl;
#endif
	      len = len + res;
	    }
	  else
	    {
	      std::cout << __FUNCTION__ << " race condition? nothing to read" << std::endl;
	    }
	}
      else
	{
	  break;
	}
    }
#ifdef DEBUG
  std::cout << __FUNCTION__ << " returns " << len << std::endl;
#endif
  return len;
}

speed_t 
Serial::ConvertStringToBaudrate( std::string s )
{
  speed_t b;

  if ( s == "B0" )
    {
      b = B0;
    }
  else if ( s == "B50" )
    {
      b = B50;
    }
  else if ( s == "B75" )
    {
      b = B75;
    }
  else if ( s == "B110" )
    {
      b = B110;
    }
  else if ( s == "B134" )
    {
      b = B134;
    }
  else if ( s == "B150" )
    {
      b = B150;
    }
  else if ( s == "B200" )
    {
      b = B200;
    }
  else if ( s == "B300" )
    {
      b = B300;
    }
  else if ( s == "B600" )
    {
      b = B600;
    }
  else if ( s == "B1200" )
    {
      b = B1200;
    }
  else if ( s == "B1800" )
    {
      b = B1800;
    }
  else if ( s == "B2400" )
    {
      b = B2400;
    }
  else if ( s == "B4800" )
    {
      b = B4800;
    }
  else if ( s == "B9600" )
    {
      b = B9600;
    }
  else if ( s == "B19200" )
    {
      b = B19200;
    }
  else if ( s == "B38400" )
    {
      b = B38400;
    }
  else if ( s == "B57600" )
    {
      b = B57600;
    }
  else if ( s == "B115200" )
    {
      b = B115200;
    }
  else if ( s == "B230400" )
    {
      b = B230400;
    }
  else if ( s == "B1000000" )
    {
      b = B1000000;
    }
  else
    {
      b = B0;
    }
  return b;
}

string
Serial::ConvertBaudrateToString( speed_t b )
{
  string s;

  if ( b == B0 )
    {
      s = "B0";
    }
  else if ( b == B50 )
    {
      s = "B50";
    }
  else if ( b == B75 )
    {
      s = "B75";
    }
  else if ( b == B110 )
    {
      s = "B110";
    }
  else if ( b == B134 )
    {
      s = "B134";
    }
  else if ( b == B150 )
    {
      s = "B150";
    }
  else if ( b == B200 )
    {
      s = "B200";
    }
  else if ( b == B300 )
    {
      s = "B300";
    }
  else if ( b == B600 )
    {
      s = "B600";
    }
  else if ( b == B1200 )
    {
      s = "B1200";
    }
  else if ( b == B1800 )
    {
      s = "B1800";
    }
  else if ( b == B2400 )
    {
      s = "B2400";
    }
  else if ( b == B4800 )
    {
      s = "B4800";
    }
  else if ( b == B9600 )
    {
      s = "B9600";
    }
  else if ( b == B19200 )
    {
      s = "B19200";
    }
  else if ( b == B38400 )
    {
      s = "B38400";
    }
  else if ( b == B57600 )
    {
      s = "B57600";
    }
  else if ( b == B115200 )
    {
      s = "B115200";
    }
  else if ( b == B230400 )
    {
      s = "B230400";
    }
  else if ( b == B1000000 )
    {
      s = "B1000000";
    }
  else
    {
      s = "B0";
    }
  return s;
}

string 
Serial::bufferToHexString( __uint8_t buffer[], unsigned int len )
{
  ostringstream s;

  s << "[";
  for( unsigned int i = 0; i < len; i++ )
    {
      s << hex << setw(2) << setfill('0') << static_cast<unsigned int>(buffer[ i ]);
    }
  s << "]";
  return s.str();
}

void
Serial::rxFlush()
{
  tcflush(fd, TCIFLUSH);
}

void
Serial::txFlush()
{
  tcflush(fd, TCOFLUSH);
}
