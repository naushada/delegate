#ifndef __MIDDLEWARE_CC__
#define __MIDDLEWARE_CC__

#include "middleware.h"
#include "protocol.h"

mna::middleware* mna::middleware::m_instance = nullptr;

/*
 * @brief  This is the hook method for application to define this member function and is invoked by
 *         ACE Framework.
 * @param  handle in which read/recv/recvfrom to be called.
 * @return 0 for success else for failure.
 */
ACE_INT32 mna::middleware::handle_input(ACE_HANDLE handle)
{
  ACE_Message_Block *mb = nullptr;
  ACE_INET_Addr peer;
  size_t recv_len = -1;

  ACE_NEW_RETURN(mb, ACE_Message_Block(mna::SIZE_1MB), -1);

  do
  {
    ACE_OS::memset((void *)mb->wr_ptr(), 0, mna::SIZE_1MB);

    if(!(recv_len = m_sock_dgram.recv(mb->wr_ptr(), mna::SIZE_1MB, peer)))
    {
      ACE_ERROR((LM_ERROR, "Receive from peer 0x%X Failed\n", peer.get_port_number()));
      break;
    }

    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l recv_len is %u\n"), recv_len));
    /*Update the length now.*/
    mb->wr_ptr(recv_len);

    /* dispatch packet to the upstream */
    rx(reinterpret_cast<uint8_t*>(mb->rd_ptr()), (uint32_t)mb->length());

    /* Reclaim the heap memory now.*/
    mb->release();

  }while(0);

  return(0);
}

/*
 * @brief  This is the hook method of ACE Event Handler and is called by ACE Framework to retrieve the
 *         handle. The handle is nothing but it's fd - file descriptor.
 * @param  none
 * @return handle of type ACE_HANDLE
 */
ACE_HANDLE mna::middleware::get_handle(void) const
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l mna::middleware::get_handle %u\n"), m_handle));
  return(m_handle);
}

ACE_INT32 mna::middleware::handle_signal(int signum, siginfo_t *s, ucontext_t *u)
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l the signum is %u\n"), signum));
  //process_signal(signum);

  return(0);
}

/*
 * @brief  This is the hook method for application to process the timer expiry. This is invoked by
 *         ACE Framework upon expiry of timer.
 * @param  tv in sec and usec.
 * @param  argument which was passed while starting the timer.
 * @return 0 for success else for failure.
 */
ACE_HANDLE mna::middleware::handle_timeout(const ACE_Time_Value &tv, const void *arg)
{
  ACE_TRACE(("mna::middleware::handle_timeout"));
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l Timer is expired\n")));

  process_timeout(arg);
  return(0);
}

/*
 * @brief this member function is invoked to start the timer.
 * @param This is the duration for timer.
 * @param This is the argument passed by caller.
 * @param This is to denote the preodicity whether this timer is going to be periodic or not.
 * @return timer_id is return.
 * */
long mna::middleware::start_timer(ACE_UINT32 to,
                                  const void *act,
                                  ACE_Time_Value interval)
{
  ACE_TRACE(("mna::middleware::start_timer"));
  ACE_Time_Value delay(to);
  long tid = 0;

  tid = ACE_Reactor::instance()->schedule_timer(this,
                                                act,
                                                delay,
                                                interval/*After this interval, timer will be started automatically.*/);

  /*Timer Id*/
  return(tid);
}

/*
 * @brief this member function is invoked to start the timer.
 * @param This is the duration for timer.
 * @param This is the argument passed by caller.
 * @param This is to denote the preodicity whether this timer is going to be periodic or not.
 * @return timer_id is return.
 * */
long mna::middleware::start_timer(ACE_UINT32 to,
                                  const void *act,
                                  bool periodicity)
{
  ACE_TRACE(("mna::middleware::start_timer"));
  ACE_Time_Value delay(to);
  long tid = 0;

  tid = ACE_Reactor::instance()->schedule_timer(this,
                                                act,
                                                delay,
                                                ACE_Time_Value::zero/*After this interval, timer will be started automatically.*/);

  /*Timer Id*/
  return(tid);
}

void mna::middleware::stop_timer(long tId)
{
  ACE_Reactor::instance()->cancel_timer(tId);
}

long mna::middleware::process_timeout(const void *act)
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l In process_timeout %u\n"), act));
  get_timer_dispatch()(act);
  return(0);
}

mna::middleware* mna::middleware::instance()
{
  if(!m_instance) {
    m_instance = new mna::middleware();
  }

  return(m_instance);
}

/**
 * @brief this member method opens the ethernet interface and configures it to receive bcast packet,
 *        with complete ethernet packet.
 * @param none
 * @return upon success handle else < 0.
 * */
ACE_HANDLE mna::middleware::open_and_bind_intf()
{
  ACE_HANDLE handle = -1;
  const char option = 1;
  struct ifreq ifr;
  struct sockaddr_ll sa;
  struct packet_mreq mr;

  do
  {
    handle = ACE_OS::socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

    if(handle < 0)
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Creation of handle for RAW Socket Failed\n")));
      break;
    }

    ACE_OS::setsockopt(handle, SOL_SOCKET, O_NDELAY, &option, sizeof(option));
    ACE_OS::setsockopt(handle, SOL_SOCKET, SO_BROADCAST, &option, sizeof(option));
    ACE_OS::memset((void *)&ifr, 0, sizeof(ifr));
    ACE_OS::strncpy(ifr.ifr_name, m_intf.c_str(), (IFNAMSIZ - 1));

    if(ACE_OS::ioctl(handle, SIOCGIFFLAGS, &ifr) < 0)
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Retrieval of IFFLAG failed for handle %d intfName %s\n"), handle, ifr.ifr_name));
      ACE_OS::close(handle);
      break;
    }

    ifr.ifr_flags |= (IFF_PROMISC | IFF_NOARP);

    if(ACE_OS::ioctl(handle, SIOCSIFFLAGS, &ifr) < 0)
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Seting of PROMISC and NOARP failed for handle %d\n"), handle));
      ACE_OS::close(handle);
      break;
    }

    ACE_OS::memset((void *)&mr, 0, sizeof(mr));
    mr.mr_ifindex = get_index();
    mr.mr_type = PACKET_MR_PROMISC;

    if(ACE_OS::setsockopt(handle, SOL_PACKET, PACKET_ADD_MEMBERSHIP, (const char *)&mr, sizeof(mr)) < 0)
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Adding membership failed for handle %d\n"), handle));
      ACE_OS::close(handle);
      break;
    }

    ACE_OS::memset((void *)&sa, 0, sizeof(sa));
    sa.sll_family = AF_PACKET;
    sa.sll_protocol = htons(ETH_P_ALL);
    sa.sll_ifindex = get_index();

    if(ACE_OS::bind(handle, (struct sockaddr *)&sa, sizeof(sa)) < 0)
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l bind failed for handle %d\n"), handle));
      ACE_OS::close(handle);
      break;
    }

  }while(0);

  return(handle);
}

/**
 * @brief This member method retrieves the eth device index based on eth device name.
 * @param none
 * @return upon success eth index else < 0.
 * */
ACE_INT32 mna::middleware::get_index()
{
  ACE_HANDLE handle = -1;
  struct ifreq ifr;
  ACE_INT32 retStatus = -1;

  ACE_TRACE("mna::middleware::get_index");

  do
  {
    handle = ACE_OS::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if(handle < 0)
    {
      ACE_ERROR((LM_ERROR, "%Isocket creation failed\n"));
      break;
    }

    ACE_OS::memset((void *)&ifr, 0, sizeof(struct ifreq));
    ifr.ifr_addr.sa_family = AF_INET;
    ACE_OS::strncpy(ifr.ifr_name, (const char *)m_intf.c_str(), (IFNAMSIZ - 1));

    if(ACE_OS::ioctl(handle, SIOCGIFINDEX, &ifr) < 0)
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Retrieval of ethernet Index failed for handle %d\n"), handle));
      ACE_OS::close(handle);
      break;
    }

    ACE_OS::close(handle);
    retStatus = ifr.ifr_ifindex;

  }while(0);

  return(retStatus);
}

/**
 * @brief This is the main entry point to protocol interface, the ethernet packet is
 *        passed to ethernet handler with help of delegate.
 * @param pointer to const ethernet packet.
 * @param length of ethernet packet.
 * @return 0 upon success else < 0.
 * */
int32_t mna::middleware::rx(const uint8_t* in, uint32_t inLen)
{
  /*Identify the packet type and connect the object of protocol layer by using delegate*/
  mna::eth::ETH* pEth = (mna::eth::ETH* )in;

  do {

    if(!pEth) {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Pointer to ethernet packet is nullptr\n")));
      break;
    }

    switch(ntohs(pEth->proto)) {

      case mna::eth::IPv4: {

        mna::ipv4::IP* pIP = (mna::ipv4::IP* )&in[sizeof(mna::eth::ETH)];
        eth().set_upstream(mna::ipv4::ip::upstream_t::from(ip(), &mna::ipv4::ip::rx));

        switch(pIP->proto) {
          case mna::ipv4::ICMP: {

            //ip().set_upstream(mna::transport::tcp::upstream_t::from(tcp(), &mna::transport::tcp::rx));
            break;
          }

          case mna::ipv4::TCP: {

            mna::transport::TCP* pTCP = (mna::transport::TCP* )&pIP[sizeof(mna::ipv4::IP)];
            //ip().set_upstream(mna::transport::tcp::upstream_t::from(tcp(), &mna::transport::tcp::rx));
            switch(ntohs(pTCP->dest_port)) {
              case mna::transport::HTTP: {
                ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l the HTTP is %u\n"), 80));
                break;
              }
              case mna::transport::RADIUS_AUTH: {
                ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l the RADIUS_AUTH is %u\n"), 80));
                break;
              }
              case mna::transport::RADIUS_ACC: {
                ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l the RADIUS_ACC is %u\n"), 80));
                break;
              }
              case mna::transport::HTTPS: {
                ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l the HTTPS is %u\n"), 80));
                break;
              }
              default: {
                ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l the Default is %u\n"), 80));
                break;
              }

            }
            break;
          }

          case mna::ipv4::UDP: {
            ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l the porotocol is UDP \n")));

            ip().set_upstream(mna::transport::udp::upstream_t::from(udp(), &mna::transport::udp::rx));
            mna::transport::UDP* pUDP = (mna::transport::UDP* )&in[sizeof(mna::eth::ETH) + sizeof(mna::ipv4::IP)];

            switch(ntohs(pUDP->dest_port)) {

              case mna::transport::BOOTPS: {

                ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l the DHCP dest_port is %u\n"), ntohs(pUDP->dest_port)));
                udp().set_upstream(mna::dhcp::server::upstream_t::from(dhcp(), &mna::dhcp::server::rx));

                /*Updating timers' member function.*/
                dhcp().set_start_timer(mna::dhcp::server::start_timer_t::from(*this, &mna::middleware::start_timer));
                dhcp().set_stop_timer(mna::dhcp::server::stop_timer_t::from(*this, &mna::middleware::stop_timer));

                /*! Kick the processing of the request now.*/
                eth().rx(in, inLen);
                break;

              }

              case mna::transport::DNS: {
                ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l the DNS is \n")));
                break;
              }

              default:
                ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l the port is %u\n"), ntohs(pUDP->dest_port)));
                break;
            }

            break;
          }

          case mna::ipv4::L2TP: {

          }

          default: {
            ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l The IP Protocol is %d\n"), pIP->proto));
          }

      }

      break;
    }

      case mna::eth::IPv6:
        break;
      case mna::eth::ARP:
        break;
      case mna::eth::EAPOL:
        break;
      case mna::eth::PPP:
        break;
      default:
        break;
    }

  } while(0);

  return(0);
}

int32_t mna::middleware::tx(uint8_t* out, uint32_t inLen)
{
  return(0);
}


#endif /*__MIDDLEWARE_CC__*/
