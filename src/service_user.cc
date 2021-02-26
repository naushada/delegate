#ifndef __service_user_cc__
#define __service_user_cc__
#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include <algorithm>

#include "delegate.hpp"
#include "service_user.h"


/*DDNS - support*/

int32_t mna::ddns::client::on_receive(uint8_t* req, ssize_t reqLen)
{
  std::string in((const char*)req, reqLen);

  std::cout << __PRETTY_FUNCTION__ <<std::endl;
  std::cout << "Response is "<< in.c_str() <<std::endl;
  return(0);
}

/**
 * @brief 
 * @param req
 * @return 
 * */
int32_t mna::ddns::client::buildWanIPRequest(std::string& req)
{
  req.clear();
  req += "GET / HTTP/1.1\r\n";
  req += "Host: ip1.dynupdate.no-ip.com\r\n";
  req += "Connection: keep-alive\r\n";
  req += "Content-Length: 0\r\n";
  req += "User-Agent: curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3\r\n";
  req += "Accept-Encoding: *\r\n";
  req += "Accept-Language: en, mi\r\n";
  req += "\r\n\r\n";

  return(0);
}

/**
 * @brief
 * @param req
 * @param peer
 * @return
 * */
int32_t mna::ddns::client::buildWanIPUpdateRequest(std::string& req, vddnsPeer& peer, std::string b64AuthStr)
{
  req.clear();
  std::string hosts;

  for_each(peer.hostName().begin(), peer.hostName().end(), [&](std::string arg) {hosts += arg; hosts += ",";});

  if(!hosts.empty()) {
    /*! getridof of last , from a host string.*/
    hosts.erase(hosts.size() - 1);
  }

  req += "GET /nic/update?hostname=";
  req += hosts;
  //req += "&myip";
  //req += wanIP().c_str();
  req += " HTTP/1.1\r\n";
  req += "Host: ";
  req += peer.domainName();
  req += " \r\n";
  req += "Authorization: ";
  req += b64AuthStr.c_str();
  req += "\r\n";
  req += "Connection: keep-alive\r\n";
  req += "Content-Length: 0\r\n";
  req += "User-Agent: curl/7.16.3\r\n";
  req += "Accept-Encoding: *\r\n";
  req += "Accept-Language: en, mi\r\n";
  req += "\r\n\r\n";

  return(0);
}















#endif /*__service_user_cc__*/
