#ifndef __MIDDLEWARE_H__
#define __MIDDLEWARE_H__

#include <fcntl.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <linux/if_packet.h>

#include "ace/Reactor.h"
#include "ace/SString.h"
#include "ace/Hash_Map_Manager.h"
#include "ace/Null_Mutex.h"
#include "ace/Event_Handler.h"
#include "ace/Basic_Types.h"
#include "ace/SOCK_Dgram.h"
#include "ace/INET_Addr.h"
#include "ace/Message_Block.h"

#include "protocol.h"
#include "delegate.hpp"

namespace mna {

  enum common_t : uint32_t {
    SIZE_1KB = 1024,
    SIZE_1MB = (SIZE_1KB * SIZE_1KB),
    ETH_ALEN = 6,
    ETH_P_ALL = 0x0003,
  };

  class middleware : public ACE_Event_Handler {
    public:

      using timer_delegate_t = delegate<long (const void*)>;
      using upstream_delegate_t = delegate<int32_t (const uint8_t*, uint32_t)>;
      using downstream_delegate_t = delegate<int32_t (uint8_t*, size_t)>;

      /** This ctor is invoked when instantiated with non-const string.*/
      middleware(std::string& intf)
      {
        m_intf = std::move(intf);
        m_to_dispatch.reset();
        m_rx_dispatch.reset();
        m_tid = 0;
        m_handle = open_and_bind_intf();

        /*Creating the instance of respective protocol layer.*/
        ACE_NEW_NORETURN(m_s, mna::dhcp::server());
        ACE_NEW_NORETURN(m_udp, mna::transport::udp());
        ACE_NEW_NORETURN(m_ip, mna::ipv4::ip());
        ACE_NEW_NORETURN(m_et, mna::eth::ether(m_intf.c_str()));

      }

      /** This ctor will be invoked when instantiated with const string.*/
      middleware(std::string&& intf)
      {
        m_intf = intf;
        m_to_dispatch.reset();
        m_rx_dispatch.reset();
        m_tid = 0;
        m_handle = open_and_bind_intf();

        /*Creating the instance of respective protocol layer.*/
        ACE_NEW_NORETURN(m_s, mna::dhcp::server());
        ACE_NEW_NORETURN(m_udp, mna::transport::udp());
        ACE_NEW_NORETURN(m_ip, mna::ipv4::ip());
        ACE_NEW_NORETURN(m_et, mna::eth::ether(m_intf.c_str()));
      }

      middleware() = default;
      middleware(const middleware& ) = default;
      middleware(middleware&& ) = default;

      virtual ~middleware()
      {
        delete m_s;
        delete m_udp;
        delete m_ip;
        delete m_et;
      }

      ACE_INT32 handle_input(ACE_HANDLE handle) override;
      ACE_INT32 handle_signal(int signum, siginfo_t *s = 0, ucontext_t *u = 0) override;
      ACE_HANDLE handle_timeout(const ACE_Time_Value &tv, const void *act=0) override;
      ACE_HANDLE get_handle(void) const override;

      long start_timer(ACE_UINT32 delay, const void *act, ACE_Time_Value interval = ACE_Time_Value::zero);
      long start_timer(ACE_UINT32 delay, const void *act, bool periodicity);
      void stop_timer(long timerId);
      void reset_timer(long tId, ACE_UINT32 timeOutInSec);

      long process_timeout(const void *act);
      /*
       * @brief This member function opens the ethernet interface and bind to this ethernet interface.
       * @param none
       * @return none
       * */
      ACE_HANDLE open_and_bind_intf();

      ACE_INT32 get_index();

      void set_timer_dispatch(timer_delegate_t tmr)
      {
        m_to_dispatch = tmr;
      }

      timer_delegate_t get_timer_dispatch()
      {
        return(m_to_dispatch);
      }

      upstream_delegate_t get_rx_dispatch()
      {
        return(m_rx_dispatch);
      }

      void set_rx_dispatch(upstream_delegate_t rx)
      {
        m_rx_dispatch = rx;
      }

      downstream_delegate_t get_tx_dispatch()
      {
        return(m_tx_dispatch);
      }

      void set_tx_dispatch(downstream_delegate_t tx)
      {
        m_tx_dispatch = tx;
      }

      int32_t rx(const uint8_t*, uint32_t);
      int32_t tx(uint8_t*, size_t);

      mna::dhcp::server& dhcp() const
      {
        return(*m_s);
      }

      mna::transport::udp& udp() const
      {
        return(*m_udp);
      }

      mna::ipv4::ip& ip() const
      {
        return(*m_ip);
      }

      mna::eth::ether& eth() const
      {
        return(*m_et);
      }

    private:

      std::string m_intf;
      /*! socket fd */
      ACE_HANDLE m_handle;
      /*! UDP Socket */
      ACE_SOCK_Dgram m_sock_dgram;
      /* upstream interface to */
      upstream_delegate_t m_rx_dispatch;
      downstream_delegate_t m_tx_dispatch;
      /*! runnining number for timerID */
      long m_tid;

      /* Timeout Delegate Handler */
      timer_delegate_t m_to_dispatch;

      /*Pointers to layered protocol handler.*/
      mna::dhcp::server *m_s;
      mna::transport::udp *m_udp;
      mna::ipv4::ip *m_ip;
      mna::eth::ether *m_et;
  };
}












#endif /*__MIDDLEWARE_H__*/
