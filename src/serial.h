/*
 * Serial Port Handling Routines
 *
 * Jacky Baltes <jacky@cs.umanitoba.ca> Sat May  6 14:37:03 CDT 2006
 */

#ifndef _SERIAL_H_
#define _SERIAL_H_

using namespace std;
#include <string>
#include <termios.h>

class Serial 
{
 public: 
  Serial( string devname, speed_t baudrate );
  ~Serial();
  bool isValid();

  bool write( string cmd );
  bool writeSlowly( string cmd );
  bool write( char buffer[], unsigned int len );
  bool writeSlowly( __uint8_t buffer[], unsigned int len );
  bool writeChar( char c );

  int readChar( char * p );
  unsigned int receiveBuffer (__uint8_t * buffer, unsigned int msgLength);

  static speed_t convertBaudrate( string s );
  static string convertBaudrateToString( speed_t s );
  static string bufferToHexString( unsigned char buffer[], unsigned int len );
  void rxFlush();
  void txFlush();

  inline string getDeviceName( void ) { return devname; }
  inline speed_t getBaudrate( void ) { return baudrate; }

 private:
  struct termios oldtio;
  int fd;
  string devname;
  speed_t baudrate;
};

#endif
