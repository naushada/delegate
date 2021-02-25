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
          if(!len) {
            /*! by doig so ACE_Reactor shall invoke handle_close hook method. A length 0 means peer has closed the connection.*/
            return(-1);
          }

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

        int32_t handle() const
        {
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

        int32_t on_close(int32_t h)
        {
          return(0);
        }

      private:
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
          return(ret);
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

        int32_t handle() const
        {
          return(m_listener.get_handle());
        }

        int32_t on_close(int32_t h)
        {
          return(0);
        }

      private:
        receive_t m_rx;
        ACE_SOCK_Stream m_newConn;
        ACE_SOCK_Acceptor m_listener;
        ACE_INET_Addr m_myAddr;
        //std::unordered_map<int32_t, std::unique_ptr<mna::tcp::client>> m_actConn;
    };
  }

  namespace udp {
    class client final : public common<mna::udp::client> {
      public:
        using receive_t = delegate<int32_t (uint8_t*, ssize_t)>;

        client(std::string peerAddr, uint16_t peerPort, std::string myAddr, uint16_t myPort)
        {
          myAddr += ":" + std::to_string(myPort);
          ACE_INET_Addr addr(myAddr.c_str());
          m_myAddr.set(addr);

          peerAddr += ":" + std::to_string(peerPort);
          addr.set_address(peerAddr.c_str(), peerAddr.length());
          m_remoteAddr.set(addr);
          m_udpConn.open(m_myAddr);
          ACE_Reactor::instance()->register_handler(this, ACE_Event_Handler::READ_MASK);
        }

        int32_t on_receive(int32_t handle)
        {
          int32_t len = 0;
          std::array<uint8_t, 2048> arr;

          len = m_udpConn.recv((void *)arr.data(), arr.max_size(), m_remoteAddr);

          if(len < 0) {
            ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Failed to established connection\n")));
          }
          else {
            get_rx()(arr.data(), len);
          }

          std::cout << __PRETTY_FUNCTION__ << std::endl;
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

        int32_t to_send(uint8_t* out, ssize_t outLen)
        {
          ssize_t len = 0;
          len = m_udpConn.send((const void *)out, outLen, m_remoteAddr);

          if(len < 0) {
            ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l send failed to peer \n")));
          }

          return(len);
        }

        int32_t handle() const
        {
          return(m_udpConn.get_handle());
        }

        void set_rx(receive_t rx)
        {
          m_rx = rx;
        }

        receive_t get_rx() const
        {
          return(m_rx);
        }

        int32_t on_close(int32_t h)
        {
          return(0);
        }

      private:
        receive_t m_rx;
        ACE_SOCK_Dgram m_udpConn;
        ACE_INET_Addr m_remoteAddr;
        ACE_INET_Addr m_myAddr;
    };

    class server final : public common<mna::udp::server> {
      public:
        using receive_t = delegate<int32_t (uint8_t*, ssize_t)>;

        server(std::string myAddr, uint16_t myPort)
        {
          myAddr += ":" + std::to_string(myPort);
          ACE_INET_Addr self(myAddr.c_str());
          m_myAddr.set(self);
          m_udpConn.open(m_myAddr);
          ACE_Reactor::instance()->register_handler(this, ACE_Event_Handler::READ_MASK);
        }

        int32_t on_receive(int32_t handle)
        {
          ssize_t len = 0;
          std::array<uint8_t, 2048> arr;

          len = m_udpConn.recv((void *)arr.data(), arr.max_size(), m_remoteAddr);
          if(len < 0) {
            ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l recv failed from peer \n")));
          }
          else {
            get_rx()(arr.data(), len);
          }

          std::cout << __PRETTY_FUNCTION__ << std::endl;
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

        int32_t on_close(int32_t h)
        {
          return(0);
        }

        int32_t to_send(uint8_t* out, ssize_t outLen)
        {
          ssize_t len = 0;
          len = m_udpConn.send((const void *)out, outLen, m_remoteAddr);

          if(len < 0) {
            ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l send failed to peer \n")));
          }

          return(len);
        }

        int32_t handle() const
        {
          return(m_udpConn.get_handle());
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
        receive_t m_rx;
        ACE_SOCK_Dgram m_udpConn;
        ACE_INET_Addr m_remoteAddr;
        ACE_INET_Addr m_myAddr;
    };
  }
}

template<typename Derived>
ACE_INT32 mna::common<Derived>::handle_input(ACE_HANDLE handle)
{
  Derived& child = static_cast<Derived&>(*this);
  return(child.on_receive(handle));
}

template<typename Derived>
ACE_INT32 mna::common<Derived>::handle_signal(int signum, siginfo_t *s, ucontext_t *u)
{
  Derived& child = static_cast<Derived&>(*this);
  return(child.on_signal(signum, s, u));
}

template<typename Derived>
ACE_HANDLE mna::common<Derived>::handle_timeout(const ACE_Time_Value &tv, const void *act)
{
  Derived& child = static_cast<Derived&>(*this);
  return(child.on_timeout(act));
}

template<typename Derived>
ACE_HANDLE mna::common<Derived>::handle_close(ACE_HANDLE h, ACE_Reactor_Mask mask)
{
  ACE_Reactor::instance()->remove_handler(h, ACE_Event_Handler::READ_MASK | ACE_Event_Handler::DONT_CALL);


  Derived& child = static_cast<Derived&>(*this);
  child.on_close(h);

  if(std::is_pointer<decltype(this)>::value) {
    delete this;
  }

  return(0);
}

template<typename Derived>
ACE_HANDLE mna::common<Derived>::get_handle(void) const
{
  const Derived& child = static_cast<const Derived&>(*this);
  return(child.handle());
}




#endif /*__service_provider_hpp__*/
