#ifndef __PROTOCOL_CC__
#define __PROTOCOL_CC__

#include "middleware.h"
#include "protocol.h"

int32_t mna::dhcp::OnDiscover::receive(void* parent, const uint8_t* inPtr, uint32_t inLen)
{
  std::cout << "6.OnDiscover::receive ---> " << inPtr << "inLen " << inLen << std::endl;
  dhcpEntry *dEnt = reinterpret_cast<dhcpEntry*>(parent);

  mna::dhcp::element_def_UMap_t::const_iterator it;
  it = dEnt->m_elemDefUMap.find(mna::dhcp::MESSAGE_TYPE);

  if(it != dEnt->m_elemDefUMap.end()) {

    mna::dhcp::element_def_t elm = it->second;

    dEnt->buildAndSendResponse(inPtr, inLen);
    switch(std::get<0>(elm.get_val())) {

      case mna::dhcp::DISCOVER:
        /** move to Next State. */
        dEnt->setState<OnRequest>(OnRequest::instance());
        break;

      case mna::dhcp::REQUEST:
        /** move to Next State. */
        dEnt->setState<OnRelease>(OnRelease::instance());
        break;

      case mna::dhcp::RELEASE:
        /** move to Next State. */
        dEnt->setState<OnDiscover>(OnDiscover::instance());
        break;

      case mna::dhcp::INFORM:
        /** move to Next State. */
        dEnt->setState<OnInform>(OnInform::instance());
        break;

      default:
        break;
    }
  }

  return(0);
}

void mna::dhcp::OnDiscover::onEntry(void* parent)
{
  dhcpEntry *dEnt = reinterpret_cast<dhcpEntry*>(parent);
  std::cout << "5.OnDiscover::onEntry is invoked " << std::endl;
}

void mna::dhcp::OnDiscover::onExit(void* parent)
{
  dhcpEntry *dEnt = reinterpret_cast<dhcpEntry*>(parent);
  std::cout << "5.OnDiscover::onExit is invoked " << std::endl;
}

void mna::dhcp::OnRequest::onEntry(void* parent)
{
  dhcpEntry *dEnt = reinterpret_cast<dhcpEntry*>(parent);
  std::cout << "5.OnRequest::onEntry is invoked " << std::endl;
  //mna::middleware::instance()->start_timer(5, this);
}

void mna::dhcp::OnRequest::onExit(void* parent)
{
  dhcpEntry *dEnt = reinterpret_cast<dhcpEntry*>(parent);
  std::cout << "5.OnRequest::onExit is invoked " << std::endl;
}

int32_t mna::dhcp::OnRequest::receive(void* parent, const uint8_t* inPtr, uint32_t inLen)
{
  std::cout << "OnRequest::receive ---> " << inPtr << "inLen " << inLen << std::endl;
  dhcpEntry *dEnt = reinterpret_cast<dhcpEntry*>(parent);

  mna::dhcp::element_def_UMap_t::const_iterator it;
  it = dEnt->m_elemDefUMap.find(mna::dhcp::MESSAGE_TYPE);

  if(it != dEnt->m_elemDefUMap.end()) {

    mna::dhcp::element_def_t elm = it->second;
    dEnt->buildAndSendResponse(inPtr, inLen);

    switch(std::get<0>(elm.get_val())) {

      case mna::dhcp::DISCOVER:
        /** move to Next State. */
        dEnt->setState<OnRequest>(OnRequest::instance());
        break;

      case mna::dhcp::REQUEST:
        /** move to Next State. */
        dEnt->setState<OnRelease>(OnRelease::instance());
        break;

      case mna::dhcp::RELEASE:
        /** move to Next State. */
        dEnt->setState<OnDiscover>(OnDiscover::instance());
        break;

      case mna::dhcp::INFORM:
        /** move to Next State. */
        dEnt->setState<OnInform>(OnInform::instance());
        break;

      default:
        break;
    }
  }

  return(0);
}

void mna::dhcp::OnRelease::onEntry(void* parent)
{
  dhcpEntry *dEnt = reinterpret_cast<dhcpEntry*>(parent);
  std::cout << "5.OnRelease::onEntry is invoked " << std::endl;
}

void mna::dhcp::OnRelease::onExit(void* parent)
{
  dhcpEntry *dEnt = reinterpret_cast<dhcpEntry*>(parent);
  std::cout << "5.OnRelease::onExit is invoked " << std::endl;
}

int32_t mna::dhcp::OnRelease::receive(void* parent, const uint8_t* inPtr, uint32_t inLen)
{
  std::cout << "Onrelease::receive ---> " << inPtr << "inLen " << inLen << std::endl;
  dhcpEntry *dEnt = reinterpret_cast<dhcpEntry*>(parent);

  mna::dhcp::element_def_UMap_t::const_iterator it;
  it = dEnt->m_elemDefUMap.find(mna::dhcp::MESSAGE_TYPE);

  if(it != dEnt->m_elemDefUMap.end()) {

    mna::dhcp::element_def_t elm = it->second;
    dEnt->buildAndSendResponse(inPtr, inLen);

    switch(std::get<0>(elm.get_val())) {

      case mna::dhcp::DISCOVER:
        /** move to Next State. */
        dEnt->setState<OnRequest>(OnRequest::instance());
        break;

      case mna::dhcp::REQUEST:
        /** move to Next State. */
        dEnt->setState<OnRelease>(OnRelease::instance());
        break;

      case mna::dhcp::RELEASE:
        /** move to Next State. */
        dEnt->setState<OnDiscover>(OnDiscover::instance());
        break;

      case mna::dhcp::INFORM:
        /** move to Next State. */
        dEnt->setState<OnInform>(OnInform::instance());
        break;

      default:
        break;
    }
  }

  return(0);
}

void mna::dhcp::OnInform::onEntry(void* parent)
{
  dhcpEntry *dEnt = reinterpret_cast<dhcpEntry*>(parent);
  std::cout << "5.OnInform::onEntry is invoked " << std::endl;
}

void mna::dhcp::OnInform::onExit(void* parent)
{
  dhcpEntry *dEnt = reinterpret_cast<dhcpEntry*>(parent);
  std::cout << "5.OnInform::onExit is invoked " << std::endl;
}

int32_t mna::dhcp::OnInform::receive(void* parent, const uint8_t* inPtr, uint32_t inLen)
{
  std::cout << "OnInform::receive ---> " << inPtr << "inLen " << inLen << std::endl;
  dhcpEntry *dEnt = reinterpret_cast<dhcpEntry*>(parent);

  mna::dhcp::element_def_UMap_t::const_iterator it;
  it = dEnt->m_elemDefUMap.find(mna::dhcp::MESSAGE_TYPE);

  if(it != dEnt->m_elemDefUMap.end()) {

    mna::dhcp::element_def_t elm = it->second;
    dEnt->buildAndSendResponse(inPtr, inLen);

    switch(std::get<0>(elm.get_val())) {

      case mna::dhcp::DISCOVER:
        /** move to Next State. */
        dEnt->setState<OnRequest>(OnRequest::instance());
        break;

      case mna::dhcp::REQUEST:
        /** move to Next State. */
        dEnt->setState<OnRelease>(OnRelease::instance());
        break;

      case mna::dhcp::RELEASE:
        /** move to Next State. */
        dEnt->setState<OnDiscover>(OnDiscover::instance());
        break;

      case mna::dhcp::INFORM:
        /** move to Next State. */
        dEnt->setState<OnInform>(OnInform::instance());
        break;

      default:
        break;
    }
  }

  return(0);
}

long mna::dhcp::dhcpEntry::startTimer(uint32_t delay, const void* txn)
{
  return(0);
}

void mna::dhcp::dhcpEntry::stopTimer(long tid)
{
}

int32_t mna::dhcp::dhcpEntry::tx(uint8_t* out, uint32_t outLen)
{
  return(0);
}

/**
 * @brief This method is used to build the DHCP response (OFFER/ACK) and invokes tx method
 *        to send the response to lower layer.
 * @param pointer to input/request buffer(DISCOVER/REQUEST).
 * @param length of input buffer/request length.
 * @return upon success 0 else < 0.
 * */
int32_t mna::dhcp::dhcpEntry::buildAndSendResponse(const uint8_t* in, uint32_t inLen)
{
  (void)inLen;
  uint32_t offset = 0;
  uint8_t rsp[1024];

  mna::dhcp::element_def_t elm;
  uint8_t cookie[] = {0x63, 0x82, 0x53, 0x63};
  mna::dhcp::dhcp_t *out = (mna::dhcp::dhcp_t* )rsp;
  mna::dhcp::dhcp_t *req = (mna::dhcp::dhcp_t* )in;

  /** Populating dhcp Header. */
  out->op = 2; /** Boot Reply. */
  out->htype = req->htype;
  out->hlen = req->hlen;
  out->hops = req->hops;
  out->xid = req->xid;
  out->secs = req->secs;
  out->flags = req->flags;
  out->ciaddr = req->ciaddr;
  out->yiaddr = htonl(m_clientIP);
  out->siaddr = req->siaddr;
  out->giaddr = req->giaddr;

  std::memcpy((void *)out->chaddr, req->chaddr, req->hlen);
  std::memset(out->sname, 0, sizeof(out->sname));
  std::memset(out->file, 0, sizeof(out->file));

  /*Filling up the dhcp option.*/
  offset = sizeof(mna::dhcp::dhcp_t);

  std::memcpy((void *)&rsp[offset], cookie, sizeof(cookie));
  offset += sizeof(cookie);

  element_def_UMap_t::const_iterator it;
  it = m_elemDefUMap.find(mna::dhcp::MESSAGE_TYPE);

  if(it != m_elemDefUMap.end()) {
    elm = it->second;
    switch(std::get<0>(elm.get_val())) {

      case mna::dhcp::DISCOVER:
        rsp[offset++] = mna::dhcp::MESSAGE_TYPE;
        rsp[offset++] = 1;
        rsp[offset++] = mna::dhcp::OFFER;
        break;

      case mna::dhcp::REQUEST:
        rsp[offset++] = mna::dhcp::MESSAGE_TYPE;
        rsp[offset++] = 1;
        rsp[offset++] = mna::dhcp::ACK;
        break;

      case mna::dhcp::DECLINE:
        rsp[offset++] = mna::dhcp::MESSAGE_TYPE;
        rsp[offset++] = 1;
        rsp[offset++] = mna::dhcp::ACK;
        break;

      case mna::dhcp::NACK:
        break;

      case mna::dhcp::RELEASE:
        break;

      case mna::dhcp::INFORM:
        rsp[offset++] = mna::dhcp::MESSAGE_TYPE;
        rsp[offset++] = 1;
        rsp[offset++] = mna::dhcp::ACK;
        break;

      default:
        break;
    }
  }

  /*Parameter list.*/
  it = m_elemDefUMap.find(mna::dhcp::PARAMETER_REQUEST_LIST);
  if(it != m_elemDefUMap.end()) {

    uint32_t idx = 0;
    elm = it->second;
    for(idx = 0; idx < elm.get_len(); idx++) {

      switch(elm.get_val()[idx]) {

        case mna::dhcp::SUBNET_MASK:
          rsp[offset++] = mna::dhcp::SUBNET_MASK;
          rsp[offset++] = 4;
          *((uint32_t*)&rsp[offset]) = htonl(0);
          offset += 4;
          break;

        case mna::dhcp::ROUTER:
          rsp[offset++] = mna::dhcp::ROUTER;
          rsp[offset++] = 4;
          *((uint32_t*)&rsp[offset]) = htonl(0);
          offset += 4;
          break;

        case mna::dhcp::TIME_SERVER:
          rsp[offset++] = mna::dhcp::TIME_SERVER;
          rsp[offset++] = 4;
          *((uint32_t*)&rsp[offset]) = htonl(0x01020304);
          offset += 4;
          break;

        case mna::dhcp::DNS:
          rsp[offset++] = mna::dhcp::DNS;
          rsp[offset++] = 4;
          *((uint32_t*)&rsp[offset]) = htonl(m_dnsIP);
          offset += 4;
          break;

        case mna::dhcp::NAME_SERVER:
          rsp[offset++] = mna::dhcp::NAME_SERVER;
          rsp[offset++] = 4;
          *((uint32_t*)&rsp[offset]) = htonl(0);
          offset += 4;
          break;

        case mna::dhcp::HOST_NAME:
          rsp[offset++] = mna::dhcp::HOST_NAME;
          rsp[offset++] = m_hostName.length();
          /*Host Machine Name to be updated.*/;
          std::memcpy((void *)&rsp[offset], m_hostName.c_str(),
                       m_hostName.length());
          offset += m_hostName.length();
          break;

        case mna::dhcp::DOMAIN_NAME:
          rsp[offset++] = mna::dhcp::DOMAIN_NAME;
          rsp[offset++] = m_domainName.length();
          /*Host Machine Name to be updated.*/;
          std::memcpy((void *)&rsp[offset], m_domainName.c_str(),
                      m_domainName.length());
          offset += m_domainName.length();
          break;

        case mna::dhcp::MTU:
          rsp[offset++] = mna::dhcp::MTU;
          rsp[offset++] = 2;
          /*Host Machine Name to be updated.*/;
          *((uint16_t*)&rsp[offset]) = htons(m_mtu);
          offset += 2;
          break;

        case mna::dhcp::BROADCAST_ADDRESS:
          rsp[offset++] = mna::dhcp::BROADCAST_ADDRESS;
          rsp[offset++] = 4;
          /*Host Machine Name to be updated.*/;
          *((uint32_t*)&rsp[offset]) = htonl(0x00);
          offset += 4;
          break;

        case mna::dhcp::NIS_DOMAIN:
          rsp[offset++] = mna::dhcp::NIS_DOMAIN;
          rsp[offset++] = 4;
          /*Host Machine Name to be updated.*/;
          *((uint32_t*)&rsp[offset]) = htonl(0x00);
          offset += 4;
          break;

        case mna::dhcp::NIS:
          rsp[offset++] = mna::dhcp::NIS;
          rsp[offset++] = 4;
          /*Host Machine Name to be updated.*/;
          *((uint32_t*)&rsp[offset]) = htonl(0x00);
          offset += 4;
          break;

        case mna::dhcp::NTP_SERVER:
          rsp[offset++] = mna::dhcp::NTP_SERVER;
          rsp[offset++] = 4;
          /*Host Machine Name to be updated.*/;
          *((uint32_t*)&rsp[offset]) = htonl(0x00);
          offset += 4;
          break;

        case mna::dhcp::REQUESTED_IP_ADDRESS:
          rsp[offset++] = mna::dhcp::REQUESTED_IP_ADDRESS;
          rsp[offset++] = 4;
          /*Host Machine Name to be updated.*/;
          *((uint32_t*)&rsp[offset]) = htonl(0x00);
          offset += 4;
          break;

        case mna::dhcp::IP_LEASE_TIME:
          rsp[offset++] = mna::dhcp::IP_LEASE_TIME;
          rsp[offset++] = 4;
          /*Host Machine Name to be updated.*/;
          *((uint32_t*)&rsp[offset]) = htonl(m_lease);
          offset += 4;
          break;

        case mna::dhcp::OVERLOAD:
          rsp[offset++] = mna::dhcp::OVERLOAD;
          rsp[offset++] = 4;
          /*Host Machine Name to be updated.*/;
          *((uint32_t*)&rsp[offset]) = htonl(0x00);
          offset += 4;
          break;

        case mna::dhcp::SERVER_IDENTIFIER:
          rsp[offset++] = mna::dhcp::SERVER_IDENTIFIER;
          rsp[offset++] = 4;
          /*Host Machine Name to be updated.*/;
          *((uint32_t*)&rsp[offset]) = htonl(m_serverID);
          offset += 4;
          break;

        default:
          break;
      }
    }
  }

  rsp[offset++] = mna::dhcp::IP_LEASE_TIME;
  rsp[offset++] = 4;
  *((uint32_t*)&rsp[offset]) = htonl(m_lease);
  offset += 4;

  rsp[offset++] = mna::dhcp::MTU;
  rsp[offset++] = 2;
  *((uint32_t*)&rsp[offset]) = htons(m_mtu);
  offset += 2;

  rsp[offset++] = mna::dhcp::SERVER_IDENTIFIER;
  rsp[offset++] = 4;
  *((uint32_t*)&rsp[offset]) = htonl(m_serverID);
  offset += 4;

  rsp[offset++] = mna::dhcp::END;

  return(tx(rsp, offset));
}

/**
 * @brief This member function parses the DHCP OPTION followed by DHCP Header and stores
 *        them into unordered_map based on tag. tag will be the KEY and value
 *        is the instance of element_def_t.
 * @param pointer to the dhcp option
 * @param length of dhcp option data
 * @return upon sucess 0 else < 0.
 * */
int32_t mna::dhcp::dhcpEntry::parseOptions(const uint8_t* in, uint32_t inLen)
{
  uint32_t offset = 0;

  while(inLen > 0) {

    switch(in[offset]) {

      case mna::dhcp::END:
        inLen = 0;
        break;

      default:
        mna::dhcp::element_def_t elm;
        uint8_t tag = in[offset];

        element_def_UMap_t::const_iterator it;
        it = m_elemDefUMap.find(tag);

        if(it != m_elemDefUMap.end()) {

          /*Found in the Map , Update with new contents received now.*/
          elm = it->second;
          elm.set_tag(in[offset++]);
          elm.set_len(in[offset++]);
          std::memcpy(elm.m_val.data(), &in[offset], elm.get_len());
          offset += elm.get_len();

        } else {

          /*Not found in the MAP.*/
          elm.set_tag(in[offset++]);
          elm.set_len(in[offset++]);
          std::memcpy(elm.m_val.data(), &in[offset], elm.get_len());
          offset += elm.get_len();
          /*Add it into MAP now.*/
          m_elemDefUMap.insert(std::pair<uint8_t, element_def_t>(tag, elm));

        }
    }
  }
  return(0);
}

/**
 * @brief This member function gets dhcp packet and parses DHCP Option
 *        and stores them into unordered_map and then feed the request
 *        to FSM for further processing.
 * @param dhcp packet
 * @param length of dhcp packet
 * @return upon success 0 else < 0.
 * */
int32_t mna::dhcp::dhcpEntry::rx(const uint8_t* in, uint32_t inLen)
{

  mna::dhcp::dhcp_t *req = (mna::dhcp::dhcp_t* )in;
  size_t cookie_len = 4;
  uint8_t *opt = (uint8_t *)&in[sizeof(mna::dhcp::dhcp_t) + cookie_len];

  parseOptions(opt, (inLen - (sizeof(mna::dhcp::dhcp_t) + cookie_len)));

  m_xid = req->xid;
  std::memcpy(m_chaddr.data(), req->chaddr, req->hlen);

  /** Feed to FSM now to process respective request. */
  return(getState().rx(in, inLen));

}

int32_t mna::dhcp::server::rx(const uint8_t* in, uint32_t inLen)
{
  std::cout << "1.server::rx received REQ " <<std::endl;
  dhcp_entry_onMAC_t::const_iterator it;
  dhcpEntry* dEnt = nullptr;

  const uint8_t *clientMAC = ((dhcp_t *)in)->chaddr;
  uint8_t len = ((dhcp_t *)in)->hlen;

  std::string MAC = std::string((const char *)clientMAC, len);

  it = m_dhcpUmapOnMAC.find(MAC);

  if(it != m_dhcpUmapOnMAC.end()) {

    std::cout << "2.dhcpEntry Instance is found " << std::endl;
    /* DHCP Client Entry is found. */
    dEnt = it->second;

  } else {

    std::cout << "2.dhcpEntry instantiated " << std::endl;
    /* New DHCP Client Request, create an entry for it. */
    dEnt = new dhcpEntry(this, 123, m_routerIP, m_dnsIP, m_lease, m_mtu, m_serverID, m_domainName);

    bool ret = m_dhcpUmapOnMAC.insert(std::pair<std::string, dhcpEntry*>(MAC, dEnt)).second;

    if(!ret) {
      std::cout << "Insertion of dhcpEntry failed " << std::endl;
    }

  }

  /* Feed to FSM now. */
  dEnt->rx(in, inLen);
  return(0);

}

long mna::dhcp::server::timedOut(const void* txn)
{
  dhcp_entry_onMAC_t::const_iterator it;
  const uint8_t *clientMAC = reinterpret_cast<const uint8_t *>(txn);
  std::string MAC = std::string((const char *)clientMAC, 6);
  it = m_dhcpUmapOnMAC.find(MAC);

  if(it != m_dhcpUmapOnMAC.end()) {
    mna::dhcp::dhcpEntry *dEnt = it->second;
    delete dEnt;
  }

  return(0);
}










#endif /* __PROTOCOL_CC__ */
