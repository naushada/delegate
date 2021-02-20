#ifndef __MIDDLEWARE_H__
#define __MIDDLEWARE_H__
#include <string>

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
#include "ace/SOCK_Stream.h"
#include "ace/SOCK_Connector.h"
#include "ace/INET_Addr.h"
#include "ace/Message_Block.h"

#include "protocol.h"
#include "delegate.hpp"
#include "application.hpp"

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
        m_intf = intf;
        m_to_dispatch.reset();
        m_rx_dispatch.reset();
        m_tid = 0;
        m_handle = open_and_bind_intf();

        /*Creating the instance of respective protocol layer.*/
        m_s = std::make_unique<mna::dhcp::server>();
        m_udp = std::make_unique<mna::transport::udp>();
        m_ip = std::make_unique<mna::ipv4::ip>();
        m_et = std::make_unique<mna::eth::ether>(m_intf.c_str());

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
        m_s = std::make_unique<mna::dhcp::server>();
        m_udp = std::make_unique<mna::transport::udp>();
        m_ip = std::make_unique<mna::ipv4::ip>();
        m_et = std::make_unique<mna::eth::ether>(m_intf.c_str());
      }

      middleware() = default;
      middleware(const middleware& ) = default;
      middleware(middleware&& ) = default;

      virtual ~middleware()
      {
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
        return(*(m_s.get()));
      }

      mna::transport::udp& udp() const
      {
        return(*(m_udp.get()));
      }

      mna::ipv4::ip& ip() const
      {
        return(*(m_ip.get()));
      }

      mna::eth::ether& eth() const
      {
        return(*(m_et.get()));
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
      std::unique_ptr<mna::dhcp::server> m_s;
      std::unique_ptr<mna::transport::udp> m_udp;
      std::unique_ptr<mna::ipv4::ip> m_ip;
      std::unique_ptr<mna::eth::ether> m_et;
  };
}


namespace mna {
  namespace client {
    class ddns final : public mna::tcp::client::middleware<mna::client::ddns> {
      public:
        ddns(std::string myIPAddress, std::string remoteAddress, uint16_t peerPort) : middleware(myIPAddress, remoteAddress, peerPort)
        {
        }

        int32_t on_receive(uint8_t *in, ssize_t inLen) {
          std::cout << "This Fn - " << __PRETTY_FUNCTION__ << std::endl;
        }

        int32_t on_timeout(uint8_t *in, ssize_t inLen) {
          std::cout << "This Fn - " << __PRETTY_FUNCTION__ << std::endl;
        }

        int32_t on_signal(uint8_t *in, ssize_t inLen) {
          std::cout << "This Fn - " << __PRETTY_FUNCTION__ << std::endl;
        }

        int32_t on_send(uint8_t *in, ssize_t inLen) {
          std::cout << "This Fn - " << __PRETTY_FUNCTION__ << std::endl;
        }

      private:

    };

    class http final : public mna::tcp::client::middleware<mna::client::http> {
      public:
        http(std::string myIPAddress, std::string remoteAddress, uint16_t peerPort) :middleware(myIPAddress, remoteAddress, peerPort)
        {
        }

        int32_t on_receive(uint8_t *in, ssize_t inLen) {
          std::cout << "This Fn - " << __PRETTY_FUNCTION__ << std::endl;
        }

        int32_t on_timeout(uint8_t *in, ssize_t inLen) {
          std::cout << "This Fn - " << __PRETTY_FUNCTION__ << std::endl;
        }

        int32_t on_signal(uint8_t *in, ssize_t inLen) {
          std::cout << "This Fn - " << __PRETTY_FUNCTION__ << std::endl;
        }

        int32_t on_send(uint8_t *in, ssize_t inLen) {
          std::cout << "This Fn - " << __PRETTY_FUNCTION__ << std::endl;
        }

      private:

    };
  }
}









#endif /*__MIDDLEWARE_H__*/
