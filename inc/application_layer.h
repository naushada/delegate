#ifndef __application_layer_h__
#define __application_layer_h__


namespace mna {
  namespace ddns {
    struct vddnsInstance {

      std::string m_bindAddress;
      uint32_t m_periodicUpdate;

      vddnsInstance()
      {
        m_bindAddress.clear();
        m_periodicUpdate = 0;
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

  namespace dns {
    enum opcode_t : uint8_t {
      QUERY = 0,
      INVERSE_QUERY = 1,
      STATUS = 2
    };

    enum rr_code_t : uint8_t {
      /*RR-Code*/
      RCODE_NO_ERROR = 0,
      RCODE_FORMAT_ERROR = 1,
      RCODE_SERVER_FAILURE = 2,
      RCODE_NAME_ERROR = 3,
      RCODE_NOT_IMPLEMENTED = 4,
      RCODE_REFUSED = 5
    };

    enum rr_type_t : uint8_t {
      /*RR-Type*/
      RRTYPE_A = 1,
      RRTYPE_NA = 2,
      RRTYPE_MD = 3,
      RRTYPE_MF = 4,
      RRTYPE_CNAME = 5,
      RRTYPE_SOA = 6,
      RRTYPE_MB = 7,
      RRTYPE_MG = 8,
      RRTYPE_MR = 9,
      RRTYPE_AWKS = 11,
      RRTYPE_PTR = 12,
      RRTYPE_HINFO = 13,
      RRTYPE_MX = 14,
      RRTYPE_TXT = 15,
      RRTYPE_ALL = 255
    };

    enum rr_class_t : uint8_t {
      /*RR-Class*/
      RRCLASS_IN = 1,
      RRCLASS_CS = 2,
      RRCLASS_CH = 3,
      RRCLASS_HS = 4,
      RRCLASS_AXFR = 5,
      RRCLASS_MAILB = 6,
      RRCLASS_MAILA = 7,
      RRCLASS_ANY = 255
    };

    struct queryHdr_t {
      uint8_t m_len;
      std::array<uint8_t, 255> m_value;

      queryHdr_t()
      {
        m_len = 0;
        m_value.fill(0);
      }

      ~queryHdr_t() = default;

      void len(uint8_t l)
      {
        m_len =l;
      }

      uint8_t len(void)
      {
        return(m_len);
      }

      std::array<uint8_t, 255> value(void)
      {
        return(m_value);
      }

      void value(std::array<uint8_t, 255> val)
      {
        m_value = val;
      }
    };

    struct queryData_t {
      std::vector<queryHdr_t> m_queryHdrVec;
      /*Query Type.*/
      uint16_t m_qtype;
      /*Query Class.*/
      uint16_t m_qclass;

      queryData_t()
      {
        m_qtype = 0;
        m_qclass = 0;
        m_queryHdrVec.clear();
      }

      ~queryData_t() = default;

      void qtype(uint16_t type)
      {
        m_qtype = type;
      }

      uint16_t qtype(void)
      {
        return (m_qtype);
      }

      uint16_t qclass(void)
      {
        return(m_qclass);
      }

      void qclass(uint16_t cls)
      {
        m_qclass = cls;
      }
    };

    typedef struct DNS {
      /*unique transaction id to map request into response.*/
      uint16_t xid;
      /*Recursion desired.*/
      uint8_t rd:1;
      /*Is message truncated.*/
      uint8_t tc:1;
      /*Is it authoritative Answer*/
      uint8_t aa:1;
      /*Query Type.*/
      uint8_t opcode:4;
      /*request(0), response(1)*/
      uint8_t qr:1;

      /*Response Code*/
      uint8_t rcode:4;
      /*RFU*/
      uint8_t z:3;
      /*Recursion Available.*/
      uint8_t ra:1;
      /*number of entries in Query section.*/
      uint16_t qdcount;
      /*Number of RR (Resource Record) in Answer Section.*/
      uint16_t ancount;
      /*Number of Name Server Record.*/
      uint16_t nscount;
      /*Resource Record in Additional Section.*/
      uint16_t arcount;
    }__attribute__((packed))DNS;

    class server {
      public:
        using receive_t = delegate<int32_t (uint8_t* in, uint16_t inLen)>;

        server(std::string domainName, std::string ip) : m_domainName(domainName), m_ipAddr(ip)
        {
        }

        ~server() = default;

        std::string& domainName(void)
        {
          return(m_domainName);
        }

        void domainName(std::string dName)
        {
          m_domainName = dName;
        }

        std::string& ip(void)
        {
          return(m_ipAddr);
        }

        void ip(std::string ip)
        {
          m_ipAddr = ip;
        }

        uint16_t xid()
        {
          return(m_xid);
        }

        void xid(uint16_t id)
        {
          m_xid = id;
        }

        uint8_t rd()
        {
          return(m_rd);
        }

        void rd(uint8_t d)
        {
          m_rd = d;
        }

        void rx(receive_t r)
        {
          m_rx = r;
        }

        receive_t rx()
        {
          return(m_rx);
        }

        void tx(receive_t r)
        {
          m_tx = r;
        }

        receive_t tx()
        {
          return(m_tx);
        }

        void set_chostIP(std::string hname, uint32_t ip)
        {
          m_chostToIPUMap[hname] = ip;
        }

        uint32_t get_chostIP(std::string hname)
        {
          return(m_chostToIPUMap[hname]);
        }

        int32_t buildQDSection(uint8_t& qdcount, std::array<uint8_t, 1024>& inRef);
        int32_t buildNSSection(std::string& name, std::array<uint8_t, 1024>& inRef);
        int32_t buildRRSection(std::string& name, uint32_t ip, std::array<uint8_t, 1024>& inRef);
        int32_t processQdcount(const std::array<uint8_t, 2048>& in, uint32_t inLen, uint16_t qdcount);
        int32_t processRequest(const uint8_t* in, uint32_t inLen);
        int32_t buildDnsResponse(std::array<uint8_t, 2048>& outRef);

      private:
        uint16_t m_xid;
        uint8_t m_rd;
        receive_t m_rx;
        receive_t m_tx;
        std::string m_domainName;
        std::string m_ipAddr;
        std::vector<queryData_t> m_qDataVec;
        /**! dhcp client host name to IP Address map.*/
        std::unordered_map<std::string, uint32_t> m_chostToIPUMap;
    };
  }
}










#endif /*__application_layer_h__*/
