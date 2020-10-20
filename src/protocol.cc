#ifndef __PROTOCOL_CC__
#define __PROTOCOL_CC__

#include <protocol.h>

int32_t mna::dhcp::OnDiscover::receive(void* parent, const char* inPtr, uint32_t inLen)
{
  std::cout << "6.OnDiscover::receive ---> " << inPtr << "inLen " << inLen << std::endl;
  dhcpEntry *dEnt = reinterpret_cast<dhcpEntry *>(parent);
  /** move to Next State. */
  dEnt->setState<OnRequest>(dEnt->instRequest());
  /* Start Processing DHCP DISCOVER Request.*/
  return(REQUEST);
}

void mna::dhcp::OnDiscover::onEntry()
{
  std::cout << "5.OnDiscover::onEntry is invoked " << std::endl;
}

void mna::dhcp::OnDiscover::onExit()
{
  std::cout << "5.OnDiscover::onExit is invoked " << std::endl;
}


void mna::dhcp::OnRequest::onEntry()
{
  std::cout << "5.OnRequest::onEntry is invoked " << std::endl;
}

void mna::dhcp::OnRequest::onExit()
{
  std::cout << "5.OnRequest::onExit is invoked " << std::endl;
}

int32_t mna::dhcp::OnRequest::receive(void* parent, const char* inPtr, uint32_t inLen)
{
  std::cout << "OnRequest::receive ---> " << inPtr << "inLen " << inLen << std::endl;
  dhcpEntry *dEnt = reinterpret_cast<dhcpEntry *>(parent);
  dEnt->setState<OnRelease>(dEnt->instRelease());
  /* Start Processing DHCP DISCOVER Request.*/
  return(RELEASE);
}

void mna::dhcp::OnRelease::onEntry()
{
  std::cout << "5.OnRelease::onEntry is invoked " << std::endl;
}

void mna::dhcp::OnRelease::onExit()
{
  std::cout << "5.OnRelease::onExit is invoked " << std::endl;
}

int32_t mna::dhcp::OnRelease::receive(void* parent, const char* inPtr, uint32_t inLen)
{
  std::cout << "Onrelease::receive ---> " << inPtr << "inLen " << inLen << std::endl;
  dhcpEntry *dEnt = reinterpret_cast<dhcpEntry *>(parent);
  dEnt->setState<OnDiscover>(dEnt->instDiscover());
  /* Start Processing DHCP DISCOVER Request.*/
  return(RELEASE);
}

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

        if(it == m_elemDefUMap.end()) {

          /*Not found in the MAP.*/
          elm.m_tag = in[offset++];
          elm.m_len = in[offset++];
          memcpy(elm.m_val.data(), &in[offset], elm.m_len);
          offset += elm.m_len;

          /*Add it into MAP now.*/
          m_elemDefUMap.insert(std::pair<uint8_t, element_def_t>(tag, elm));

        } else {

          /*Found in the Map , Update with new contents received now.*/
          elm = it->second;
          elm.m_tag = in[offset++];
          elm.m_len = in[offset++];
          memcpy(elm.m_val.data(), &in[offset], elm.m_len);
          offset += elm.m_len;

        }
    }
  }
  return(0);
}

int32_t mna::dhcp::dhcpEntry::rx(const char* in, uint32_t inLen)
{
  uint8_t *opt = (uint8_t *)&in[sizeof(mna::dhcp::dhcp_t)];

  parseOptions(opt, (inLen - sizeof(mna::dhcp::dhcp_t)));

  /** Kick the state machine now. */
  std::cout << "3.dhcpEntry::rx is invoked " << std::endl;
  int32_t ret = getState().rx(this, in, inLen);
  /* move FSM into next State now.*/
  switch (ret) {
    case REQUEST:
      std::cout << "OnRequest State is set " << std::endl;
      /* Initializing the State Machine. */
      setState<OnRequest>(m_instRequest);
    break;
  }

  return(0);
}


int32_t mna::dhcp::server::rx(const char* in, uint32_t inLen)
{
  std::cout << "1.server::rx received REQ " <<std::endl;
  dhcp_entry_onMAC_t::const_iterator it;

  const uint8_t *clientMAC = ((dhcp_t *)in)->chaddr;
  uint8_t len = ((dhcp_t *)in)->hlen;

  std::string MAC = std::string((const char *)clientMAC, len);

  it = m_dhcpUmapOnMAC.find(MAC);

  if(it != m_dhcpUmapOnMAC.end()) {

    std::cout << "2.dhcpEntry Instance is found " << std::endl;
    /* DHCP Client Entry is found. */
    dhcpEntry dEnt = it->second;
    /* Feed to FSM now. */
    dEnt.rx(in, inLen);

  } else {

    std::cout << "2.dhcpEntry instantiated " << std::endl;
    /* New DHCP Client Request, create an entry for it. */
    dhcpEntry dEnt(123, m_routerIP, m_dnsIP, m_lease, m_mtu, m_serverID, m_domainName);

    bool ret = m_dhcpUmapOnMAC.insert(std::pair<std::string, dhcpEntry>(MAC, dEnt)).second;

    if(!ret) {
      std::cout << "Insertion of dhcpEntry failed " << std::endl;
    }

    dEnt.rx(in, inLen);
  }

  return(0);

}

int main()
{

  char req[255];
  mna::dhcp::dhcp_t* pReq = (mna::dhcp::dhcp_t *)req;
  pReq->op = 1;
  uint8_t MAC[] = {0x08, 0x00, 0x27, 0xa8, 0x34, 0xc3};

  std::memcpy((void *)pReq->chaddr, (const char *)MAC, 6);

  mna::dhcp::server s;

  s.rx(req, 128);
  s.rx(req, 128);

  return(0);
}











#endif /* __PROTOCOL_CC__ */
