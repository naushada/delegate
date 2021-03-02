#ifndef __application_layer_cc__
#define __application_layer_cc__

#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include <algorithm>
#include <arpa/inet.h>

#include "delegate.hpp"
#include "application_layer.h"


/*DDNS - support*/

int32_t mna::ddns::client::on_receive(uint8_t* req, ssize_t reqLen)
{
  std::string in((const char*)req, reqLen);

  std::cout << __PRETTY_FUNCTION__ <<std::endl;
  std::cout << "Response is "<< in.c_str() <<std::endl;
  return(reqLen);
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

/**! DNS Section */
int32_t mna::dns::client::buildQDSection(uint8_t &qdcount, std::array<uint8_t, 1024>& inRef)
{
  auto offset = 0;
  mna::dns::queryData_t elm;
  mna::dns::queryHdr_t qHdr;

  for(const auto& iter : m_qDataVec) {
    elm = iter;
    for(const auto& inIter : elm.m_queryHdrVec) {
      /**! populating QNAME - abc.example.net - the dot being delimeter.*/
      qHdr = inIter;
      inRef[offset++] = qHdr.len();
      std::copy_n(qHdr.value().begin(), qHdr.len(), &inRef[offset]);
      offset += qHdr.len();
    }

    /*Terminate the QNAME with 0 length.*/
    inRef[offset++] = 0;

    *((uint16_t *)&inRef[offset]) = htons(elm.qtype());
    offset += 2;
    *((uint16_t *)&inRef[offset]) = htons(elm.qclass());
    offset += 2;

    /*Number of query section*/
    qdcount++;
  }

  return(offset);
}

int32_t mna::dns::client::buildNSSection(std::string& name, std::array<uint8_t, 1024>& inRef)
{
  int idx;
  std::array<uint8_t, 128> label;
  uint8_t len = 0;
  auto offset = 0;

  label.fill(0);
  for(idx = 0; idx < name.length(); idx++) {
    if('.' == name.c_str()[idx]) {
      inRef[offset++] = len;
      std::copy_n(label.begin(), len, &inRef[offset]);
      offset += len;
      len = 0;
      label.fill(0);
    }
    else {
      label[len++] = name.c_str()[idx];
    }
  }

  /**! If there's no dot in the host name.*/
  if(len) {
    inRef[offset++] = len;
    std::copy_n(label.begin(), len, &inRef[offset]);
    offset += len;
  }

  /*Terminate the QNAME with 0 as length.*/
  inRef[offset++] = 0;

  *((uint16_t *)&inRef[offset]) = htons(mna::dns::RRTYPE_NA);
  offset += 2;
  *((uint16_t *)&inRef[offset]) = htons(mna::dns::RRCLASS_IN);
  offset += 2;

  /*TTL - 120 seconds*/
  *((uint32_t *)&inRef[offset]) = htonl(0);
  offset += 4;

  /*RD Data Length.*/
  *((uint16_t *)&inRef[offset]) = htons(0);
  offset += 2;

  return(offset);
}

int32_t mna::dns::client::buildRRSection(std::string& name, uint32_t ip, std::array<uint8_t, 1024>& inRef)
{
  int idx;
  std::array<uint8_t, 128> label;
  uint8_t len = 0;
  auto offset = 0;
  label.fill(0);

  for(idx = 0; idx < name.length(); idx++) {
    if('.' == name.c_str()[idx]) {
      inRef[offset++] = len;
      std::copy_n(label.begin(), len, &inRef[offset]);
      offset += len;
      len = 0;
    }
    else {
      label[len++] = name.c_str()[idx];
    }
  }

  if(len) {
    inRef[offset++] = len;
    std::copy_n(label.begin(), len, &inRef[offset]);
    offset += len;
  }

  /*Terminate the QNAME with 0 as length.*/
  inRef[offset++] = 0;

  *((uint16_t *)&inRef[offset]) = htons(mna::dns::RRTYPE_A);
  offset += 2;
  *((uint16_t *)&inRef[offset]) = htons(mna::dns::RRCLASS_IN);
  offset += 2;
  /*TTL - 120 seconds*/
  *((uint32_t *)&inRef[offset]) = htonl(120);
  offset += 4;
  /*RD Data Length.*/
  *((uint16_t *)&inRef[offset]) = htons(4);
  offset += 2;

  /*RD Data Value.*/
  *((uint32_t *)&inRef[offset]) = htonl(ip);
  offset += 4;
  return(offset);
}

int32_t mna::dns::client::processRequest(const std::array<uint8_t, 2048>in, uint32_t inLen)
{
  const mna::dns::DNS& dnsHdr = *reinterpret_cast<const mna::dns::DNS*>(in.data());

  if(mna::dns::QUERY == dnsHdr.opcode) {
    if(ntohs(dnsHdr.qdcount)) {
      /*This is DNS Query, Process it.*/
      processQdcount(in, inLen, ntohs(dnsHdr.qdcount));
    }
#if 0
    if(ntohs(dnsHdr->ancount)) {
      processAncount(in, inLen, ntohs(dnsHdr->ancount));
    }

    if(ntohs(dnsHdr->nscount)) {
      processNscount(in, inLen, ntohs(dnsHdr->nscount));
    }

    if(ntohs(dnsHdr->arcount)) {
      processArcount(in, inLen, ntohs(dnsHdr->arcount));
    }
#endif
  }

  return(0);
}

int32_t mna::dns::client::processQdcount(const std::array<uint8_t, 2048>& in, uint32_t inLen, uint16_t qdcount)
{
  const uint8_t* qData = &in[sizeof(mna::dns::DNS)];
  uint8_t len = 0;
  uint16_t offset = 0;
  std::array<uint8_t, 255> tmpArr;

  while(qdcount)
  {
    mna::dns::queryData_t data;
    len = qData[offset++];

    while(len)
    {
      mna::dns::queryHdr_t qHdr;
      tmpArr.fill(0);

      qHdr.len(len);
      std::copy_n(&qData[offset], len, tmpArr.data());

      qHdr.value(tmpArr);
      /*remember it into list (which is STACK) for later use.*/
      data.m_queryHdrVec.push_back(qHdr);
      offset += len;

      /*Now update the length.*/
      len = qData[offset++];
    }

    data.qtype(ntohs(*((uint16_t *)&qData[offset])));
    offset += 2;
    data.qclass(ntohs(*((uint16_t *)&qData[offset])));
    offset += 2;
    m_qDataVec.push_back(data);
    /*Process next query data.*/
    qdcount--;
  }

#if 0
  ACE_Message_Block *mb = nullptr;
  ACE_NEW_NORETURN(mb, ACE_Message_Block(CommonIF::SIZE_2MB));

  buildDnsResponse(parent, in, inLen, *mb);
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l the length is %u\n"), mb->length()));
  parent.sendResponse(macAddr(), (ACE_Byte *)mb->rd_ptr(), mb->length());

  /*clean the vector now.*/
  purgeQData();
  /*re-claim the memory now.*/
  mb->release();
#endif
  return(0);
}


int32_t mna::dns::client::buildDnsResponse(std::array<uint8_t, 2048>& outRef)
{
  std::array<uint8_t, 1024> myArr;
  uint16_t offset = 0;
  uint16_t len = sizeof(mna::dns::DNS);

  mna::dns::DNS& rspDnsHdr = *reinterpret_cast<mna::dns::DNS*>(outRef.data());

  outRef.fill(0);
  /*Prepare DNS Header.*/
  rspDnsHdr.xid = xid();
  rspDnsHdr.rd = rd();
  rspDnsHdr.tc = 0;
  rspDnsHdr.aa = 1;
  rspDnsHdr.opcode = 0;
  rspDnsHdr.qr = 1;
  rspDnsHdr.rcode = mna::dns::RCODE_NO_ERROR;
  rspDnsHdr.z = 0;
  rspDnsHdr.ra = 0;

  /*This will be updated later.*/
  rspDnsHdr.qdcount = 0;
  rspDnsHdr.ancount = htons(2);
  rspDnsHdr.nscount = htons(1);
  rspDnsHdr.arcount = 0;

  uint8_t qdcount = 0;

  /*Build Query Section.*/
  myArr.fill(0);
  offset = buildQDSection(qdcount, myArr);
  std::copy_n(myArr.begin(), offset, &outRef[len]);
  len += offset;

  /*Number of queries now.*/
  rspDnsHdr.qdcount = htons(qdcount);

  /*AN Section Now.*/
  /*AN(1)*/
  myArr.fill(0);
  std::string myDomainName = domainName();

  auto is_myDomainName = [&](mna::dns::queryData_t& arg) -> bool {

    if(2 == arg.m_queryHdrVec.size()) {

      std::string rd0(reinterpret_cast<const char *>(arg.m_queryHdrVec[0].value().data()), arg.m_queryHdrVec[0].len());
      std::string rd1(reinterpret_cast<const char *>(arg.m_queryHdrVec[1].value().data()), arg.m_queryHdrVec[1].len());
      std::string dname = rd0 + "." + rd1;
      std::cout << "domain name from query - >> " << dname.c_str() << std::endl;
      return(myDomainName == dname);
    }
    else if(arg.m_queryHdrVec.size() > 2) {

      std::string rd0(reinterpret_cast<const char *>(arg.m_queryHdrVec[1].value().data()), arg.m_queryHdrVec[1].len());
      std::string rd1(reinterpret_cast<const char *>(arg.m_queryHdrVec[2].value().data()), arg.m_queryHdrVec[2].len());
      std::string dname = rd0 + "." + rd1;
      std::cout << "domain name from query - >> " << dname.c_str() << std::endl;
      return(myDomainName == dname);
    }
    else {
      return(false);
    }
  };

  auto it = std::find_if(m_qDataVec.begin(), m_qDataVec.end(), is_myDomainName);

  if(it != std::end(m_qDataVec)) {

    struct in_addr myIP;
    int32_t len = 0;

    inet_aton(ip().c_str(), &myIP);
    offset = buildRRSection(myDomainName, myIP.s_addr, myArr);
    std::copy_n(myArr.begin(), offset, &outRef[len]);
    len += offset;
  }

#if 0
  std::vector<ACE_CString> dName;
  std::vector<ACE_CString > hName;

  getDomainNameFromQuery(dName);

  /*Is this domain name controlled by vCPGateway?*/
  if(domainName() != dName[0])
  {
    /*Domain is not ours.*/
    getHostNameFromQuery(hName);

    ACE_TCHAR fqdn[255];
    ACE_OS::snprintf(fqdn, sizeof(fqdn), "%s.%s",hName[0].c_str(), dName[0].c_str());
    ACE_CString fqdnStr(fqdn);
    buildRRSection(fqdnStr, htonl(ipAddr()), mb);
    buildRRSection(hostName(), htonl(ipAddr()), mb);
  }
  else
  {
    getHostNameFromQuery(hName);
    ACE_CString hh(hName[0].c_str());

    if(hh == hostName())
    {
      buildRRSection(domainName(), htonl(ipAddr()), mb);
      buildRRSection(hostName(), htonl(ipAddr()), mb);
    }
    else
    {
      ACE_UINT32 hIP = 0;
      /*Find in Hash Map to get the IP.*/
      ACE_Byte *IP = parent.getDhcpServerUser().getResolverIP(hh);
      if(!IP)
      {
        /*The Hostname is not control by CPGateway.*/
        hIP = ipAddr();
        /*Domain is ours. but host is different one.*/
        ACE_TCHAR fqdn[255];
        ACE_OS::snprintf(fqdn, sizeof(fqdn), "%s.%s",hName[0].c_str(), dName[0].c_str());
        ACE_CString fqdnStr(fqdn);
        buildRRSection(fqdnStr, htonl(ipAddr()), mb);
        buildRRSection(hostName(), htonl(ipAddr()), mb);
      }
      else
      {
        buildRRSection(dName[0], htonl(hIP), mb);
        hIP = atoi((const char *)IP);
        buildRRSection(hh, htonl(hIP), mb);
        delete IP;
      }

    }
  }

  ACE_TCHAR ns_fqdn[255];
  ACE_OS::snprintf(ns_fqdn, sizeof(ns_fqdn), "%s.%s",hostName().c_str(), domainName().c_str());
  ACE_CString nsFqdnStr(ns_fqdn);
  /*NS section Now.*/
  buildNSSection(nsFqdnStr, mb);
#endif

  return(len);
}





#endif /*__application_layer_cc__*/
