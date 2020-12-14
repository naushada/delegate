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
        std::cout << "DISCOVER Received " << std::endl;
        /** move to Next State. */
        dEnt->setState(OnRequest::instance());
        break;

      case mna::dhcp::REQUEST:
        std::cout << "REQUEST Received " << std::endl;
        /** move to Next State. */
        dEnt->setState(OnRelease::instance());
        break;

      case mna::dhcp::RELEASE:
        std::cout << "RELEASE Received " << std::endl;
        /** move to Next State. */
        dEnt->setState(OnDiscover::instance());
        break;

      case mna::dhcp::INFORM:
        /** move to Next State. */
        dEnt->setState(OnInform::instance());
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
  std::cout << "5.OnRequest::onEntry is invoked & Timer is started" << std::endl;

  dEnt->set_tid(dEnt->startTimer(/*dEnt->get_lease()*/1, (const void* )dEnt));
}

void mna::dhcp::OnRequest::onExit(void* parent)
{
  dhcpEntry *dEnt = reinterpret_cast<dhcpEntry*>(parent);
  std::cout << "5.OnRequest::onExit is invoked " << std::endl;
  dEnt->stopTimer(dEnt->get_tid());
}

int32_t mna::dhcp::OnRequest::receive(void* parent, const uint8_t* inPtr, uint32_t inLen)
{
  std::cout << "OnRequest::receive ---> " << "inLen " << inLen << std::endl;
  dhcpEntry *dEnt = reinterpret_cast<dhcpEntry*>(parent);

  mna::dhcp::element_def_UMap_t::const_iterator it;
  it = dEnt->m_elemDefUMap.find(mna::dhcp::MESSAGE_TYPE);

  if(it != dEnt->m_elemDefUMap.end()) {

    mna::dhcp::element_def_t elm = it->second;
    dEnt->buildAndSendResponse(inPtr, inLen);

    switch(std::get<0>(elm.get_val())) {

      case mna::dhcp::DISCOVER:
        std::cout << "OnRequest::DISCOVER " <<std::endl;
        /** move to Next State. */
        dEnt->setState(OnRequest::instance());
        break;

      case mna::dhcp::REQUEST:
        std::cout << "OnRequest::REQUEST " <<std::endl;
        /** move to Next State. */
        dEnt->setState(OnRelease::instance());
        break;

      case mna::dhcp::RELEASE:
        /** move to Next State. */
        dEnt->setState(OnDiscover::instance());
        break;

      case mna::dhcp::INFORM:
        /** move to Next State. */
        dEnt->setState(OnInform::instance());
        break;

      default:
        std::cout << "OnRequest::Default " <<std::endl;
        break;
    }
  }

  return(0);
}

void mna::dhcp::OnRelease::onEntry(void* parent)
{
  dhcpEntry *dEnt = reinterpret_cast<dhcpEntry*>(parent);
  std::cout << "5.OnRelease::onEntry is invoked & timer is started" << std::endl;
  std::string data((const char* )dEnt->get_chaddr().data(), 6);
  //dEnt->set_tid(dEnt->startTimer(/*dEnt->get_lease()*/1, (const void* )dEnt->get_chaddr().data()));
  //dEnt->set_tid(dEnt->startTimer(/*dEnt->get_lease()*/1, (const void* )data.c_str()));
  //dEnt->set_tid(dEnt->startTimer(dEnt->get_lease(), (const void* )dEnt->get_chaddr().data()));
}

void mna::dhcp::OnRelease::onExit(void* parent)
{
  dhcpEntry *dEnt = reinterpret_cast<dhcpEntry*>(parent);
  std::cout << "5.OnRelease::onExit is invoked & timer is stopped" << std::endl;
  dEnt->stopTimer(dEnt->get_tid());
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
        dEnt->setState(OnRequest::instance());
        break;

      case mna::dhcp::REQUEST:
        /** move to Next State. */
        dEnt->setState(OnRelease::instance());
        break;

      case mna::dhcp::RELEASE:
        /** move to Next State. */
        dEnt->setState(OnDiscover::instance());
        break;

      case mna::dhcp::INFORM:
        /** move to Next State. */
        dEnt->setState(OnInform::instance());
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
        dEnt->setState(OnRequest::instance());
        break;

      case mna::dhcp::REQUEST:
        /** move to Next State. */
        dEnt->setState(OnRelease::instance());
        break;

      case mna::dhcp::RELEASE:
        /** move to Next State. */
        dEnt->setState(OnDiscover::instance());
        break;

      case mna::dhcp::INFORM:
        /** move to Next State. */
        dEnt->setState(OnInform::instance());
        break;

      default:
        break;
    }
  }

  return(0);
}

long mna::dhcp::dhcpEntry::startTimer(uint32_t delay, const void* txn)
{
  std::cout << "Inside " << __PRETTY_FUNCTION__ << std::endl;
  long tid = get_start_timer()(delay, txn, false);
  return(tid);
}

void mna::dhcp::dhcpEntry::stopTimer(long tid)
{
  std::cout << "Inside " << __PRETTY_FUNCTION__ << std::endl;
  get_stop_timer()(tid);
}

int32_t mna::dhcp::dhcpEntry::tx(uint8_t* out, size_t outLen)
{
  std::cout << "Inside " << __PRETTY_FUNCTION__ << std::endl;
  return(get_downstream()(out, outLen));
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
      case mna::dhcp::INFORM:
      case mna::dhcp::RELEASE:
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

  std::cout << "DHCP PAYLOAD Length " << offset << std::endl;
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

          std::cout << "Option's entry found in the map " << std::endl;
          /*Found in the Map , Update with new contents received now.*/
          elm = it->second;
          elm.set_tag(in[offset++]);
          elm.set_len(in[offset++]);
          std::memcpy(elm.m_val.data(), &in[offset], elm.get_len());
          offset += elm.get_len();

          /*Update the MAP now.*/
          m_elemDefUMap[tag] = elm;

        } else {

          std::cout << "Option's entry not found in the map " << std::endl;
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

  const uint8_t *clientMAC = ((dhcp_t *)in)->chaddr;
  uint8_t len = ((dhcp_t *)in)->hlen;

  std::cout << "CHADDR "
            << std::hex << clientMAC[0] << " "
            << std::hex << clientMAC[1] << " "
            << std::hex << clientMAC[2] << " "
            << std::hex << clientMAC[3] << " "
            << std::hex << clientMAC[4] << " "
            << std::hex << clientMAC[5] << " " << std::endl;

  std::string MAC = std::string((const char *)clientMAC, len);

  it = m_dhcpUmapOnMAC.find(MAC);

  if(it != m_dhcpUmapOnMAC.end()) {

    std::cout << "2.dhcpEntry Instance is found " << std::endl;
    /* DHCP Client Entry is found. */
    dhcpEntry &dEnt = *(it->second.get());
    dEnt.rx(in, inLen);

  } else {

    std::cout << "2.dhcpEntry instantiated " << std::endl;
    /* New DHCP Client Request, create an entry for it. */
    std::unique_ptr<dhcpEntry> inst = std::make_unique<dhcpEntry>(this, 123, m_routerIP, m_dnsIP, m_lease, m_mtu, m_serverID, m_domainName);

    dhcpEntry &dEnt = *(inst.get());
    /* setting timer delegate */
    dEnt.set_start_timer(m_start_timer);
    dEnt.set_stop_timer(m_stop_timer);
    dEnt.set_reset_timer(m_reset_timer);
    /* setting downstream for packet transmitting. */
    dEnt.set_downstream(m_downstream);

    /* Feed to FSM now. */
    dEnt.rx(in, inLen);

    /*insert into unordered_map now.*/
    bool ret = m_dhcpUmapOnMAC.insert(std::pair<std::string, std::unique_ptr<dhcpEntry>>(MAC, std::move(inst))).second;

    if(!ret) {
      std::cout << "Insertion of dhcpEntry failed " << std::endl;
    }

  }

  return(0);

}

long mna::dhcp::server::timedOut(const void* txn)
{
  std::cout << "timedOut is invoked " << std::endl;
  dhcp_entry_onMAC_t::iterator it;

  const mna::dhcp::dhcpEntry *dEnt = reinterpret_cast<const mna::dhcp::dhcpEntry*>(txn);

  std::string MAC((const char *)dEnt->get_chaddr().data(), 6);

  it = m_dhcpUmapOnMAC.find(MAC);

  if(it != m_dhcpUmapOnMAC.end()) {
    it->second.reset(nullptr);
    m_dhcpUmapOnMAC.erase(it);
  }

  return(0);
}

/**
 * @brief this member method receives ethernet packet from lower layer and learns source/destination mac and protocol and stripped off
 *        ethernet Header and passes on to upper layer.
 * @param pointer to const byte buffer, received from lower layer
 * @param length of packet received from lower layer
 * @return upon success 0 else less than 0
 * */
int32_t mna::eth::ether::rx(const uint8_t* in, uint32_t inLen)
{
  mna::eth::ETH* pET = (mna::eth::ETH* )in;

  std::copy(std::begin(pET->src), std::end(pET->src), std::begin(m_src_mac));
  std::copy(std::begin(pET->dest), std::end(pET->dest), std::begin(m_dst_mac));
  proto(pET->proto);

  /*Identify the packet type and connect the object of protocol layer by using delegate*/
  return(m_upstream(&in[sizeof(mna::eth::ETH)], (inLen - sizeof(mna::eth::ETH))));
}

/**
 * @brief This member method receives IP packet from upper layer and computes the IP Header's checksum, prepend the
 *        Ethernet Header and passes to lower layer.
 * @param pointer to received byte buffer from upper layer with IP Header
 * @param length of received packet from upper layer
 * @return upon success 0 else less than 0.*/
int32_t mna::eth::ether::tx(uint8_t* out, size_t outLen)
{
  uint8_t rsp[1024];
  mna::eth::ETH* pETH = (mna::eth::ETH* )rsp;
  mna::ipv4::IP* pIP = (mna::ipv4::IP* )out;

  std::memset((void*)rsp, 0, sizeof(rsp));

  pIP->chksum = checksum((const uint16_t*)out, (sizeof(uint32_t) * pIP->len));

  /*Populate Ethernet Header now.*/
  std::memcpy((void*)pETH->dest, src_mac().data(), sizeof(pETH->dest));
  std::memcpy((void*)pETH->src, dst_mac().data(), sizeof(pETH->src));
  pETH->proto = proto();

  std::memcpy((void*)&rsp[sizeof(mna::eth::ETH)], out, outLen);

  outLen += sizeof(mna::eth::ETH);
  std::cout << "Inside " << __PRETTY_FUNCTION__ << std::endl;

  return(get_downstream()(rsp, outLen));
}

/**
 * @brief This member function computes the checksum of packet received from upper layer.
 * @param pointer to const byte buffer received from upper layer.
 * @param length of packet received from upper layer.
 * @return 2Bytes checksum.*/
uint16_t mna::eth::ether::checksum(const uint16_t* in, size_t inLen) const
{
  uint32_t sum = 0;

  while(inLen > 1) {

    sum += *in++;
    if(sum & 0x80000000U) {
      sum = (sum & 0xFFFF) + (sum >> 16);
    }

    inLen -= 2;
  }

  /*pkt_len is an odd*/
  if(inLen) {
    sum += (uint32_t ) *(uint8_t *)in;
  }

  /*wrapping up into 2 bytes*/
  while(sum >> 16) {
    sum = (sum & 0xFFFF) + (sum >> 16);
  }

  /*1's complement*/
  return (~sum);
}

/**
 * @brief This member method receives byte buffer from lower layer with IP Header and it learns source & destination IP and protocol
 *        and stripped off IP Header from received packet and passes to upper layer.
 * @param pointer to const byte buffer received from lower layer
 * @param length of received packet
 * @return upon success 0 else less than 0.*/
int32_t mna::ipv4::ip::rx(const uint8_t* in, uint32_t inLen)
{
  mna::ipv4::IP* pIP = (mna::ipv4::IP* )in;

  /*IP header length in 32 bit word. minimum value is (5 * 4) */
  uint32_t len = pIP->len * 4;
  src_ip(pIP->src_ip);
  dst_ip(pIP->dest_ip);
  proto(pIP->proto);

  std::cout << "ip::receive len "<< pIP->len << " src_ip " << std::hex << src_ip() << " dst_ip " << std::hex << dst_ip() << std::endl;
  return(m_upstream(&in[len], (inLen - len)));
}

/**
 * @brief This member function receives the byte buffer from upper layer and computes the UDP checksum
 *        and then prefix the IP Header to received packet
 * @param pointer to byte buffer received from upper layer
 * @param length of packet received from upper layer
 * @return upon success 0 else less than 0*/
int32_t mna::ipv4::ip::tx(uint8_t* out, size_t outLen)
{
  uint8_t rsp[1024];
  mna::ipv4::IP* pIP = (mna::ipv4::IP* )rsp;
  mna::transport::UDP* pUDP = (mna::transport::UDP* )out;

  std::memset((void*)rsp, 0, sizeof(mna::ipv4::IP));

  pUDP->chksum = compute_checksum(out, outLen);

  /*Populate IP Header now.*/
  pIP->len = 5;
  pIP->ver = 4;
  pIP->src_ip = dst_ip();
  pIP->dest_ip = src_ip();
  pIP->proto = proto();
  /*DF - Don't Fragment.*/
  pIP->flags = htons(1 << 14);
  pIP->ttl = 64;

  std::memcpy((void *)&rsp[sizeof(mna::ipv4::IP)], out, outLen);

  outLen += sizeof(mna::ipv4::IP);
  /*This includes IP Header length as well. */
  pIP->tot_len = htons(outLen);

  std::cout << "Inside " << __PRETTY_FUNCTION__ << std::endl;
  return(get_downstream()(rsp, outLen));
}

/**
 * @brief This member function prefix PSEUDO Header packet received from upper layer and computes
 *        the checksum.
 * @param pointer to byte buffer received from upper layer.
 * @param length of received byte buffer from upper layer.
 * @return 2Bytes checksum*/
uint16_t mna::ipv4::ip::compute_checksum(uint8_t* in, size_t inLen)
{
  uint8_t rsp[1024];
  mna::transport::PHDR* pseudoPtr = (mna::transport::PHDR* )rsp;

  std::memset((void *)rsp, 0, sizeof(rsp));
  /*Populating pseudo header now.*/
  pseudoPtr->src_ip = dst_ip();
  pseudoPtr->dest_ip = src_ip();
  pseudoPtr->reserve = 0;
  pseudoPtr->proto = proto();
  pseudoPtr->total_len = htons(inLen);

  std::memcpy((void *)&rsp[sizeof(mna::transport::PHDR)],
              (const void *)in,
              inLen);

  return(checksum((uint16_t*)rsp, (inLen + sizeof(mna::transport::PHDR))));
}

/**
 * @brief This member function computes the checksum of packet received from upper layer.
 * @param pointer to const byte buffer received from upper layer.
 * @param length of packet received from upper layer.
 * @return 2Bytes checksum.*/
uint16_t mna::ipv4::ip::checksum(const uint16_t* in, size_t inLen) const
{
  uint32_t sum = 0;

  while(inLen > 1) {

    sum += *in++;
    if(sum & 0x80000000U) {
      sum = (sum & 0xFFFF) + (sum >> 16);
    }

    inLen -= 2;
  }

  /*pkt_len is an odd*/
  if(inLen) {
    sum += (uint32_t ) *(uint8_t *)in;
  }

  /*wrapping up into 2 bytes*/
  while(sum >> 16) {
    sum = (sum & 0xFFFF) + (sum >> 16);
  }

  /*1's complement*/
  return (~sum);
}

/**
 * @brief This member method receives byte buffer from lower layer with UDP Header and it learns 
 *        source & destination port and stripped UDP Header and passes to upper layer.
 * @param pointer to const byte buffer from lower layer.
 * @param length of received packet.
 * @return upon success 0 else less than 0.*/
int32_t mna::transport::udp::rx(const uint8_t* in, uint32_t inLen)
{
  mna::transport::UDP* pUDP = (mna::transport::UDP* )in;
  src_port(pUDP->src_port);
  dst_port(pUDP->dest_port);

  std::cout << "udp::receive "<< std::endl;
  return(m_upstream(&in[sizeof(mna::transport::UDP)], (inLen - sizeof(mna::transport::UDP))));
}

/**
 * @brief This member function receives the application payload and prepend the UDP
 *        Header. The UDP Header Length is included along with received length from upper layer.
 * @param Pointer to upper layer byte buffer.
 * @param length of received byte buffer
 * @return upon success returns 0 else less than 0.*/
int32_t mna::transport::udp::tx(uint8_t* out, size_t outLen)
{
  uint8_t rsp[1024];

  std::memset((void *)rsp, 0, sizeof(rsp));
  mna::transport::UDP* pUDP = (mna::transport::UDP* )rsp;


  pUDP->src_port = dst_port();
  pUDP->dest_port = src_port();
  /* checksum will be calculated latter.*/
  pUDP->chksum = 0;

  /*copying the payload now.*/
  std::memcpy((void *)&rsp[sizeof(mna::transport::UDP)], out, outLen);

  outLen += sizeof(mna::transport::UDP);
  /*This is including UDP Header length as well.*/
  pUDP->len = htons(outLen);

  std::cout << "Inside " << __PRETTY_FUNCTION__ << std::endl;
  /*Response will have UDP Header + It's payload. Passes it to IP layer.*/
  return(get_downstream()(rsp, outLen));
}



#endif /* __PROTOCOL_CC__ */
