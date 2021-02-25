#ifndef __service_provider_hpp__
#define __service_provider_hpp__

namespace mna {
  template<typename Derived>
  class common : public ACE_Event_Handler {
    public:
      common() = default;
      common(const common& ) = default;
      common(common&& ) = default;
      virtual ~common() = default;

      ACE_INT32 handle_close(ACE_HANDLE handle, ACE_Reactor_Mask close_mask) override;
      ACE_INT32 handle_input(ACE_HANDLE handle) override;
      ACE_INT32 handle_signal(int signum, siginfo_t *s = 0, ucontext_t *u = 0) override;
      ACE_HANDLE handle_timeout(const ACE_Time_Value &tv, const void *act=0) override;
      ACE_HANDLE get_handle(void) const override;

    private:

  };
}

namespace mna {
  namespace tcp {

    class client final : public common<mna::tcp::client> {
      public:
        virtual ~client() = default;

        using receive_t = delegate<int32_t (uint8_t*, ssize_t)>;

        client(std::string remoteAddress, uint16_t peerPort, std::string myAddress, uint16_t myPort = 0)
        {
          std::string address;

          address.clear();
          address = remoteAddress + ":" + std::to_string(peerPort);
          ACE_INET_Addr addr(address.c_str());
          m_remoteAddr.set(addr);

          /** local Address.*/
          address.clear();

          if(myPort) {
            address = myAddress + ":" + std::to_string(myPort);
            ACE_INET_Addr self(address.c_str());
            m_myAddr.set(self);
          }
          else {
            ACE_INET_Addr self(myAddress.c_str());
            m_myAddr.set(self);
          }
        }

        client(ACE_SOCK_Stream conn, ACE_INET_Addr peerAddr)
        {
          std::cout << __PRETTY_FUNCTION__ << std::endl;
          m_remoteAddr = peerAddr;
          m_remoteConn = conn;
          ACE_Reactor::instance()->register_handler(this, ACE_Event_Handler::READ_MASK);
        }

        int32_t to_connect()
        {
          int32_t ret = 0;
          ACE_Time_Value to(1,1);

          ret = m_myConn.connect(m_remoteConn, m_remoteAddr);

          if(ret < 0) {
            ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Connection to remoteHost %s Failed\n"), m_remoteAddr.get_host_name()));
            return(ret);
          }

          std::cout << __PRETTY_FUNCTION__ << std::endl;
          ACE_Reactor::instance()->register_handler(this, ACE_Event_Handler::READ_MASK);
          return(ret);
        }

        int32_t to_send(uint8_t* out, ssize_t outLen)
        {
          int32_t ret = 0;
          uint32_t offset = 0;
          ssize_t newLen = outLen - offset;

          do {

            ret = m_remoteConn.send_n((out + offset), newLen);

            if(ret < 0) {
              /**! Error*/
              ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l send_n failed ret = %d\n"),ret));
              /**! if < 0 is returned then ACE library will close the connection, so don't get it closed.*/
              return(0);
            }

            newLen -= ret;
            offset += ret;

          }while(newLen > 0);

          return(outLen);
        }

        int32_t on_receive(int32_t handle)
        {
          std::array<uint8_t, 2048> in;
          int32_t len = 0;
          len = m_remoteConn.recv(in.data(), in.max_size());

          std::cout << __PRETTY_FUNCTION__ << std::endl;

          /** Pass the received data to delegate which will process the request.*/
          get_rx()(in.data(), len);

          /** handle_close is invoked if -1 is returned.*/
          return(len);
        }

        int32_t on_signal(int signum, siginfo_t *s = 0, ucontext_t *u = 0)
        {
          return(0);
        }

        int32_t on_timeout(const void *act=0)
        {
          return(0);
        }

        void handle(int32_t fd)
        {
          m_handle = fd;
        }

        int32_t handle() const
        {
          //m_handle = m_remoteConn.get_handle();
          return(m_remoteConn.get_handle());
        }

        void set_rx(receive_t rx)
        {
          m_rx = rx;
        }

        receive_t get_rx() const
        {
          return(m_rx);
        }

      private:
        int32_t m_handle;
        receive_t m_rx;
        ACE_INET_Addr m_remoteAddr;
        ACE_INET_Addr m_myAddr;
        ACE_SOCK_Connector m_myConn;
        ACE_SOCK_Stream m_remoteConn;
    };

    class server final : public common<mna::tcp::server> {
      public:
        using receive_t = delegate<int32_t (uint8_t*, ssize_t)>;

        server(std::string myAddr, uint16_t myPort, receive_t rx)
        {
          int32_t reuseAddr = 1;
          std::string address;
          /** local Address.*/
          address.clear();

          address = myAddr + ":" + std::to_string(myPort);
          ACE_INET_Addr self(address.c_str());
          m_myAddr.set(self);
          m_listener.open(m_myAddr, reuseAddr);
          handle(m_listener.get_handle());
          ACE_Reactor::instance()->register_handler(this, ACE_Event_Handler::ACCEPT_MASK);
          m_rx = rx;
          std::cout << __PRETTY_FUNCTION__ << std::endl;
        }

        int32_t on_receive(int32_t handle)
        {
          ACE_INET_Addr remoteAddr;
          int32_t ret = 0;

          ret = m_listener.accept(m_newConn, &remoteAddr);
          if(ret < 0) {
            ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Failed to established connection\n")));
          }
          else {
            mna::tcp::client* connectedPeer = nullptr;
            /**! Connection is established successfully.*/
            ACE_NEW_RETURN(connectedPeer, mna::tcp::client(m_newConn, remoteAddr), -1);
            //std::unique_ptr<mna::tcp::client> connectedPeer = std::make_unique<mna::tcp::client>(m_newConn, remoteAddr);
            connectedPeer->set_rx(m_rx);
            //m_actConn.insert(std::make_pair<int32_t, std::unique_ptr<mna::tcp::client>>(m_newConn.get_handle(), std::move(connectedPeer)));
          }
          std::cout << __PRETTY_FUNCTION__ << std::endl;
        }

        int32_t on_signal(int signum, siginfo_t *s = 0, ucontext_t *u = 0)
        {
          return(0);
        }

        int32_t on_timeout(const void *act=0)
        {
          return(0);
        }

        int32_t to_send(uint8_t* out, ssize_t outLen)
        {
          return(0);
        }

        void handle(int32_t h)
        {
          m_handle = h;
        }

        int32_t handle() const
        {
          return(m_listener.get_handle());
        }

      private:
        receive_t m_rx;
        ACE_HANDLE m_handle;
        ACE_SOCK_Stream m_newConn;
        ACE_SOCK_Acceptor m_listener;
        ACE_INET_Addr m_myAddr;
        //std::unordered_map<int32_t, std::unique_ptr<mna::tcp::client>> m_actConn;
    };
  }
#if 0
  namespace udp {
    class server final : public common<mna::udp::server> {
      public:
        server(std::string myAddr, uint16_t myPort);
        int32_t to_send(uint8_t* out, ssize_t outLen);
        int32_t on_receive(int32_t handle);
        int32_t on_signal(int signum, siginfo_t *s = 0, ucontext_t *u = 0);
        int32_t on_timeout(const void *act=0);
      private:
        ACE_HANDLE m_handle;
        ACE_SOCK_Dgram m_udpConn;
        ACE_INET_Addr m_remoteAddr;
        ACE_INET_Addr m_myAddr;
    };

    class client final : public common<mna::udp::client> {
      public:
        client(std::string peerAddr, uint16_t peerPort, std::string myAddr, uint16_t myPort = 0);
        int32_t to_send(uint8_t* out, ssize_t outLen);
        int32_t on_receive(int32_t handle);
        int32_t on_signal(int signum, siginfo_t *s = 0, ucontext_t *u = 0);
        int32_t on_timeout(const void *act=0);
      private:
        ACE_HANDLE m_handle;
        ACE_SOCK_Dgram m_udpConn;
        ACE_INET_Addr m_remoteAddr;
        ACE_INET_Addr m_myAddr;
    };
  }
#endif
}

template<typename Derived>
ACE_INT32 mna::common<Derived>::handle_input(ACE_HANDLE handle)
{
#if 0
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
#endif

  Derived& child = static_cast<Derived&>(*this);
  child.on_receive(handle);

  return(0);
}

template<typename Derived>
ACE_INT32 mna::common<Derived>::handle_signal(int signum, siginfo_t *s, ucontext_t *u)
{
  Derived& child = static_cast<Derived&>(*this);
  child.on_signal(signum, s, u);
  return(0);
}

template<typename Derived>
ACE_HANDLE mna::common<Derived>::handle_timeout(const ACE_Time_Value &tv, const void *act)
{
  Derived& child = static_cast<Derived&>(*this);
  child.on_timeout(act);
  return(0);
}

template<typename Derived>
ACE_HANDLE mna::common<Derived>::handle_close(ACE_HANDLE h, ACE_Reactor_Mask mask)
{
  ACE_Reactor::instance()->remove_handler(h, ACE_Event_Handler::READ_MASK | ACE_Event_Handler::DONT_CALL);

  if(std::is_pointer<decltype(this)>::value) {
    delete this;
  }

  return(0);
}

template<typename Derived>
ACE_HANDLE mna::common<Derived>::get_handle(void) const
{
  std::cout << "this Fn " << __PRETTY_FUNCTION__ << std::endl;
  const Derived& child = static_cast<const Derived&>(*this);
  return(child.handle());
}

#if 0
template<typename Derived>
int32_t mna::common<Derived>::to_connect()
{
  int32_t connStatus = 0;

  connStatus = m_myConn.connect(m_remoteConn, m_remoteAddr);

  if(connStatus < 0) {

    ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Connection to remoteHost %s Failed\n"), m_remoteAddr.get_host_name()));

  } else {

    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l connected to host %s \n"), m_remoteAddr.get_host_name()));
    return(m_remoteConn.get_handle());

  }

  return(connStatus);
}

template<typename Derived>
int32_t mna::common<Derived>::to_send(uint8_t* out, ssize_t outLen)
{
  int32_t status = 0;

  Derived& child = static_cast<Derived&>(*this);
  child.to_send();
#if 0
  status = m_remoteConn.send_n(out, outLen);

  if(status < 0) {
    ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l sent Failed\n")));
  } else {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l sent success \n")));
  }
#endif
  return(status);
}

template<typename Derived>
void mna::common<Derived>::set_handle(ACE_HANDLE handle)
{
  m_handle = handle;
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

    class udp final : public mna::udp::client::middleware<mna::client::udp> {
      public:
        using receive_t = delegate<int32_t (uint8_t* , ssize_t len)>;

        udp(std::string myIPAddress, std::string remoteAddress, uint16_t peerPort) : middleware(myIPAddress, remoteAddress, peerPort)
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
    };
  }
}

#endif






#endif /*__service_provider__*/
