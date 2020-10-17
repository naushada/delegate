#ifndef __PROTOCOL_CC__
#define __PROTOCOL_CC__

#include <protocol.h>



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
