#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <ctime>
#include <iostream>
#include <syslog.h>
#include <unistd.h>

#include <cmath>
#include <iostream>
#include <sstream>


#define DEFAULT_PORT "2134"
#define DEFAULT_SERVER "127.0.0.1"

using boost::asio::ip::udp;

std::string
SendCommand(  udp::socket & socket, udp::resolver::iterator & iterator, std::string const & command )
{
  char replyBuffer[4096];
  size_t replyBufferMaxLength = sizeof( replyBuffer );

  socket.send_to( boost::asio::buffer( command ), * iterator );
  
  udp::endpoint sender_endpoint;
  size_t replyLength = socket.receive_from(boost::asio::buffer( replyBuffer, replyBufferMaxLength), sender_endpoint );
  
  std::string reply(replyBuffer,replyLength);
#ifndef NDEBUG
  std::cout << "Response " << reply << std::endl;
#endif					   		      

  return reply;
}

int 
main(int argc, char *argv[] )
{
#ifndef NDEBUG
  std::cout << "Debugging build" << std::endl;
#endif
  
  std::string port;
  std::string server;

  if ( argc == 3 )
    {
      server = argv[1];
      port = atoi(argv[2]);
    }
  else if ( argc == 2 ) 
    {
      port = argv[1];
    }
  else
    {
      server = DEFAULT_SERVER;
      port = DEFAULT_PORT;
    }

  boost::asio::io_service io_service;
  udp::socket socket( io_service, udp::endpoint( udp::v4(), 0 ) );

  udp::resolver resolver( io_service );
  udp::resolver::query query( udp::v4(), server, port );
  udp::resolver::iterator iterator = resolver.resolve( query );

  for(;;)
    {
      std::string com("Results");
      std::string reply;
      reply = SendCommand( socket, iterator, com );
      std::cout << "Results found " << reply << std::endl;
    }
  return 0;
}

