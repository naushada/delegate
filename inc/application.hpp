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
  return(0);
}

template<typename Derived>
ACE_HANDLE mna::tcp::client::middleware<Derived>::handle_timeout(const ACE_Time_Value &tv, const void *act)
{
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











#endif /*__application_hpp__*/
