#ifndef __UDP_VISION_SERVER_HPP__
#define __UDP_VISION_SERVER_HPP__

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/udp.hpp>
//#include <boost/asio/signal_set.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>

class VideoStream;

class UDPVisionServer
{
public:
  UDPVisionServer(boost::asio::io_service & io_service, unsigned int port, VideoStream const * vs );
  void StartServer();

private:
  int SendResponse( std::string const & msg );

private:
  boost::asio::ip::udp::socket socket;
  boost::asio::ip::udp::endpoint remoteEP;
  boost::array<char,1> receiveBuffer;

  VideoStream const * videostream;
};

#endif /* __UDP_VISION_SERVER_HPP__ */
