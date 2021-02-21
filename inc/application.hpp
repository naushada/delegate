#ifndef __application_hpp__
#define __application_hpp__

namespace mna {
  namespace tcp {

    namespace server {
      template<typename Derived>
      class middleware : public ACE_Event_Handler {
        public:
          ACE_INT32 handle_input(ACE_HANDLE handle) override;
          ACE_INT32 handle_signal(int signum, siginfo_t *s = 0, ucontext_t *u = 0) override;
          ACE_HANDLE handle_timeout(const ACE_Time_Value &tv, const void *act=0) override;
          ACE_HANDLE get_handle(void) const override;
        private:
          /*! socket fd */
          ACE_HANDLE m_handle;
          /*! UDP Socket */
          ACE_SOCK_Stream m_sock_stream;
      };
    }

    namespace client {
      template<typename Derived>
      class middleware : public ACE_Event_Handler {
        public:
          middleware(std::string myIPAddress, std::string remoteAddress, uint16_t peerPort);
          int32_t to_connect();
          int32_t to_send(uint8_t* out, ssize_t outLen);

          ACE_INT32 handle_input(ACE_HANDLE handle) override;
          ACE_INT32 handle_signal(int signum, siginfo_t *s = 0, ucontext_t *u = 0) override;
          ACE_HANDLE handle_timeout(const ACE_Time_Value &tv, const void *act=0) override;
          ACE_HANDLE get_handle(void) const override;

        private:

          /*! socket fd */
          ACE_HANDLE m_handle;
          /*! UDP Socket */
          ACE_SOCK_Stream m_remoteConn;
          ACE_INET_Addr m_remoteAddr;
          ACE_INET_Addr m_myAddr;
          ACE_SOCK_Connector m_myConn;
      };
    }
  }

  namespace udp {
    namespace server {
      template<typename Derived>
      class middleware : public ACE_Event_Handler {
        public:
          ACE_INT32 handle_input(ACE_HANDLE handle) override;
          ACE_INT32 handle_signal(int signum, siginfo_t *s = 0, ucontext_t *u = 0) override;
          ACE_HANDLE handle_timeout(const ACE_Time_Value &tv, const void *act=0) override;
          ACE_HANDLE get_handle(void) const override;
        private:
          /*! socket fd */
          ACE_HANDLE m_handle;
          /*! UDP Socket */
          ACE_SOCK_Dgram m_sock_dgram;
      };
    }

    namespace client {
      template<typename Derived>
      class middleware : public ACE_Event_Handler {
        public:
          ACE_INT32 handle_input(ACE_HANDLE handle) override;
          ACE_INT32 handle_signal(int signum, siginfo_t *s = 0, ucontext_t *u = 0) override;
          ACE_HANDLE handle_timeout(const ACE_Time_Value &tv, const void *act=0) override;
          ACE_HANDLE get_handle(void) const override;
        private:
          /*! socket fd */
          ACE_HANDLE m_handle;
          /*! UDP Socket */
          ACE_SOCK_Dgram m_sock_dgram;
      };
    }
  }
}

template<typename Derived>
mna::tcp::client::middleware<Derived>::middleware(std::string myIPAddress, std::string remoteAddress, uint16_t peerPort)
{
  std::string peerAddr;

  peerAddr.clear();
  peerAddr = remoteAddress + ":" + std::to_string(peerPort);
  ACE_INET_Addr addr(peerAddr.c_str());
  m_remoteAddr.set(addr);

  m_myAddr.set_address(myIPAddress.c_str(), myIPAddress.length());
}

template<typename Derived>
ACE_INT32 mna::tcp::client::middleware<Derived>::handle_input(ACE_HANDLE handle)
{
  ssize_t len = 0;
  std::array<uint8_t, 2048>buffer;
  buffer.fill(0);

  len = m_remoteConn.recv(buffer.data(), buffer.max_size());
  if(len > 0) {
    Derived& child = static_cast<Derived&>(*this);
    child.on_receive(buffer.data(), len);
  }


  if(!len) {
    /*! connection is closed now.*/
    m_remoteConn.close();
  }

  return(0);
}

template<typename Derived>
ACE_INT32 mna::tcp::client::middleware<Derived>::handle_signal(int signum, siginfo_t *s, ucontext_t *u)
{
  Derived& child = static_cast<Derived&>(*this);
  child.on_signal(signum, s, u);
  return(0);
}

template<typename Derived>
ACE_HANDLE mna::tcp::client::middleware<Derived>::handle_timeout(const ACE_Time_Value &tv, const void *act)
{
  Derived& child = static_cast<Derived&>(*this);
  child.on_timeout(act);
  return(0);
}

template<typename Derived>
ACE_HANDLE mna::tcp::client::middleware<Derived>::get_handle(void) const
{
  std::cout << "this Fn " << __PRETTY_FUNCTION__ << std::endl;
  std::cout << "The handle is " << m_remoteConn.get_handle() << std::endl;
  return(m_remoteConn.get_handle());
}

template<typename Derived>
int32_t mna::tcp::client::middleware<Derived>::to_connect()
{
  int32_t connStatus = 0;

  connStatus = m_myConn.connect(m_remoteConn, m_remoteAddr);

  if(connStatus < 0) {
    ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Connection to remoteHost %s Failed\n"), m_remoteAddr.get_host_name()));
  } else {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l connected to host %s \n"), m_remoteAddr.get_host_name()));
  }

  return(connStatus);
}


template<typename Derived>
int32_t mna::tcp::client::middleware<Derived>::to_send(uint8_t* out, ssize_t outLen)
{
  int32_t status = 0;

  status = m_remoteConn.send_n(out, outLen);

  if(status < 0) {
    ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l sent Failed\n")));
  } else {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l sent success \n")));
  }

  return(status);
}


namespace mna {
  namespace client {
    class tcp final : public mna::tcp::client::middleware<mna::client::tcp> {
      public:
        using receive_t = delegate<int32_t (uint8_t* , ssize_t len)>;
        using connect_t = delegate<int32_t ()>;

        tcp(std::string myIPAddress, std::string remoteAddress, uint16_t peerPort) : middleware(myIPAddress, remoteAddress, peerPort)
        {
        }

        int32_t on_receive(uint8_t *in, ssize_t inLen) {
          get_rx()(in, inLen);
        }

        int32_t on_timeout(const void* act) {
          std::cout << "This Fn - " << __PRETTY_FUNCTION__ << std::endl;
        }

        int32_t on_signal(int signum, siginfo_t* s, ucontext_t* u) {
          std::cout << "This Fn - " << __PRETTY_FUNCTION__ << std::endl;
        }

        int32_t on_send(uint8_t *in, ssize_t inLen) {
          to_send(in, inLen);
        }

        int32_t on_connect() {
          to_connect();
        }

        void set_connect(connect_t r)
        {
          m_connect = r;
        }

        connect_t get_connect()
        {
          return(m_connect);
        }

        void set_rx(receive_t r)
        {
          m_rx = r;
        }

        receive_t get_rx()
        {
          return(m_rx);
        }

        void set_tx(receive_t r)
        {
          m_tx = r;
        }

        receive_t get_tx()
        {
          return(m_tx);
        }

      private:
        receive_t m_rx;
        receive_t m_tx;
        connect_t m_connect;

    };

    class http final : public mna::tcp::client::middleware<mna::client::http> {
      public:
        http(std::string myIPAddress, std::string remoteAddress, uint16_t peerPort) :middleware(myIPAddress, remoteAddress, peerPort)
        {
        }

        int32_t on_receive(uint8_t *in, ssize_t inLen) {
          std::cout << "This Fn - " << __PRETTY_FUNCTION__ << std::endl;
          std::string resp((const char *)in, inLen);
          std::cout << "The response is " << resp.c_str() << std::endl;
        }

        int32_t on_timeout(const void* act) {
          std::cout << "This Fn - " << __PRETTY_FUNCTION__ << std::endl;
        }

        int32_t on_signal(int signum, siginfo_t* s, ucontext_t* u) {
          std::cout << "This Fn - " << __PRETTY_FUNCTION__ << std::endl;
        }

        int32_t on_send(uint8_t *in, ssize_t inLen) {
          std::cout << "This Fn - " << __PRETTY_FUNCTION__ << std::endl;
          to_send(in, inLen);
        }

        int32_t on_connect() {
          to_connect();
          std::cout << "This Fn - " << __PRETTY_FUNCTION__ << std::endl;
        }

      private:

    };
  }
}








#endif /*__application_hpp__*/
