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

namespace mna {

  enum common_t : uint32_t {
    SIZE_1KB = 1024,
    SIZE_1MB = (SIZE_1KB * SIZE_1KB),
    ETH_ALEN = 6,
    ETH_P_ALL = 0x0003,
  };

  class middleware : public ACE_Event_Handler {
    public:

      /** This ctor is invoked when instantiated with non-const string.*/
      middleware(std::string& intf)
      {
        m_mb = nullptr;
        m_intf = std::move(intf);
        m_handle = open_and_bind_intf();
      }

      /** This ctor will be invoked when instantiated with const string.*/
      middleware(std::string&& intf)
      {
        m_mb = nullptr;
        m_intf = intf;
        m_handle = open_and_bind_intf();
      }

      middleware(const middleware& ) = default;
      middleware(middleware&& ) = default;

      virtual ~middleware()
      {
        if(m_mb) m_mb->release();
      }

      ACE_INT32 handle_input(ACE_HANDLE handle) override;
      ACE_INT32 handle_signal(int signum, siginfo_t *s = 0, ucontext_t *u = 0) override;
      ACE_HANDLE handle_timeout(const ACE_Time_Value &tv, const void *act=0) override;
      ACE_HANDLE get_handle(void) const override;

      long start_timer(ACE_UINT32 delay, const void *act, ACE_Time_Value interval = ACE_Time_Value::zero);
      void stop_timer(long timerId);
      void reset_timer(long tId, ACE_UINT32 timeOutInSec);

      ACE_INT32 process_timeout(const void *act);
      /*
       * @brief This member function opens the ethernet interface and bind to this ethernet interface.
       * @param none
       * @return none
       * */
      ACE_HANDLE open_and_bind_intf();

      ACE_INT32 get_index();
    private:

      std::string m_intf;
      /*! socket fd */
      ACE_HANDLE m_handle;
      /*! UDP Socket */
      ACE_SOCK_Dgram m_sock_dgram;
      ACE_Message_Block *m_mb;

      /*! runnining number for timerID*/
      long m_tid;
  };
}












#endif /*__MIDDLEWARE_H__*/
