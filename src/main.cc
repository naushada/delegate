#ifndef __MAIN_CC__
#define __MAIN_CC__

#include "protocol.h"
#include "middleware.h"

ACE_UINT8 loop_forever(void)
{
  ACE_Time_Value to(5);

  while(1)
  {
    ACE_Reactor::instance()->handle_events(to);
  }

  return(0);
}

int main(int count, char* param[])
{

  char req[255];
  mna::dhcp::dhcp_t* pReq = (mna::dhcp::dhcp_t *)req;
  pReq->op = 1;
  uint8_t MAC[] = {0x08, 0x00, 0x27, 0xa8, 0x34, 0xc3};

  std::memcpy((void *)pReq->chaddr, (const char *)MAC, 6);

  mna::dhcp::server s;

  s.rx(req, 128);
  s.rx(req, 128);

  mna::middleware mw("enp0s9");
  ACE_Reactor::instance()->register_handler(&mw, ACE_Event_Handler::READ_MASK);

  loop_forever();

  return(0);
}


#endif /*__MAIN_CC__*/
