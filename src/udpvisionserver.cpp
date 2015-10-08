#include "udpvisionserver.h"
#include "videostream.h"
#include "globals.h"

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/udp.hpp>
//#include <boost/asio/signal_set.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>

#include <ctime>
#include <iostream>
#include <syslog.h>
#include <unistd.h>
#include <string>

using boost::asio::ip::udp;

UDPVisionServer::UDPVisionServer( boost::asio::io_service & io_service, unsigned int port, VideoStream const * vs )
  : socket( io_service, udp::endpoint( udp::v4(), port ) ),
    videostream( vs )
{
}

void 
UDPVisionServer::StartServer()
{
  char data[4096];
  size_t maxLength = sizeof(data);

  for(;;)
    {
      size_t len = socket.receive_from( boost::asio::buffer(data,maxLength), remoteEP );
#ifndef NDEBUG
      std::cout << "Received message of size " << len << std::endl;
#endif
      std::string command(data,len);
#ifndef NDEBUG
      std::cout << "Received command " << command << std::endl;
#endif

      Globals * glob = Globals::GetGlobals();
      glob->LockBuffer();
      std::string results = videostream->resultString;
      //      std::string results("Ok");
      glob->UnlockBuffer();

#ifndef NDEBUG
      std::cout << "Sending response " << results << std::endl;
#endif

      SendResponse( results );
    }
}

int
UDPVisionServer::SendResponse( std::string const & msg )
{
  return socket.send_to( boost::asio::buffer( msg ), remoteEP );
}

