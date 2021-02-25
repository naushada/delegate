#ifndef __service_user_h__
#define __service_user___


namespace mna {
  namespace ddns {
    struct vddnsInstance {

      std::string m_bindAddress;
      uint32_t m_periodicUpdate;

      vddnsInstance()
      {
        m_bindAddress.clear();
      }

      void bindAddress(std::string m)
      {
        m_bindAddress = m;
      }

      std::string bindAddress()
      {
        return(m_bindAddress);
      }

      void periodicUpdate(uint32_t m)
      {
        m_periodicUpdate = m;
      }

      uint32_t periodicUpdate()
      {
        return(m_periodicUpdate);
      }
    };

    struct vddnsPeer {
      /*! service provide*/
      std::string m_name;
      std::string m_userId;
      std::string m_password;
      /*! This domain name is provided by service-provider*/
      std::string m_domainName;
      uint16_t m_peerPort;
      std::vector<std::string> m_hostName;

      vddnsPeer()
      {
        m_name.clear();
        m_userId.clear();
        m_password.clear();
        m_domainName.clear();
        m_peerPort = 80;
        m_hostName.clear();
      }

      void name(std::string m)
      {
        m_name = m;
      }

      std::string name()
      {
        return(m_name);
      }

      void userId(std::string m)
      {
        m_userId = m;
      }

      std::string userId()
      {
        return(m_userId);
      }

      void password(std::string m)
      {
        m_password = m;
      }

      std::string password()
      {
        return(m_password);
      }

      void domainName(std::string m)
      {
        m_domainName = m;
      }

      std::string domainName()
      {
        return(m_domainName);
      }

      void peerPort(uint16_t m)
      {
        m_peerPort = m;
      }

      uint16_t peerPort()
      {
        return(m_peerPort);
      }

      void hostName(std::string h)
      {
        m_hostName.push_back(h);
      }

      std::vector<std::string>& hostName()
      {
        return(m_hostName);
      }
    };

    struct config {
      std::vector<vddnsPeer> m_peer;
      vddnsInstance m_instance;

      config(std::string sName);

      config(const config& ) = default;
      config(config&& ) = default;

      std::vector<vddnsPeer>& peer()
      {
        return(m_peer);
      }

      vddnsInstance& instance()
      {
        return(m_instance);
      }

      int32_t parse(std::string name);
    };

    class client {
      public:
        using receive_t = delegate<int32_t (uint8_t* in, ssize_t inLen)>;
        using send_t = delegate<int32_t (uint8_t* in, ssize_t inLen)>;
        using connect_t = delegate<int32_t ()>;

        client() = default;
        config& get_config()
        {
          return(*m_config.get());
        }

        void set_config(std::unique_ptr<config> cfg)
        {
          m_config = std::move(cfg);
        }

        std::string wanIP()
        {
          return(m_wanIP);
        }

        void wanIP(std::string m)
        {
          m_wanIP = m;
        }

        void rx(receive_t r)
        {
          m_rx = r;
        }

        receive_t rx()
        {
          return(m_rx);
        }

        void set_tx(send_t r)
        {
          m_tx = r;
        }

        send_t get_tx()
        {
          return(m_tx);
        }

        void set_connect(connect_t r)
        {
          m_connect = r;
        }

        connect_t get_connect()
        {
          return(m_connect);
        }

        int32_t buildWanIPRequest(std::string& req);
        int32_t processWanIPResponse(std::string& req);
        int32_t buildWanIPUpdateRequest(std::string& req, vddnsPeer& peer, std::string b64AuthStr);
        int32_t processWanIPUpdateResponse(std::string& req);
        int32_t on_receive(uint8_t* in, ssize_t inLen);

      private:
        std::unique_ptr<config> m_config;
        std::string m_wanIP;
        receive_t m_rx;
        send_t m_tx;
        connect_t m_connect;
    };
  }
}










#endif /*__service_user_h__*/
