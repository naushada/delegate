#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <iostream>
#include <delegate.hpp>
#include <array>
#include <unordered_map>
#include <algorithm>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <mutex>
#include <memory>
#include <arpa/inet.h>

namespace mna {
  class FSM {

    public:
      /* Type Aliasing */
      /**
       * @brief : parent is the instance of a class that instantiating this FSM.
       *          the next argument is the received requiest buffer and the last
       *          argument is the length of received request.
       * */
      using receive_delegate_t = delegate<int32_t (void* parent, const uint8_t*, uint32_t)>;
      using state_delegate_t = delegate<void (void* parent)>;

      FSM(void* parent)
      {
        on_receive.reset();
        on_exit.reset();
        on_entry.reset();
        m_parent = parent;
      }

      FSM(const FSM& fsm) = default;
      FSM(FSM&& fsm) = default;

      FSM& operator=(const FSM& fsm)
      {
        on_receive = fsm.on_receive;
        on_exit = fsm.on_exit;
        on_entry = fsm.on_entry;
        return(*this);
      }

      ~FSM()
      {
        on_receive.reset();
        on_exit.reset();
        on_entry.reset();
        m_parent = nullptr;
      }

      /**
       * @brief This member function is used to set the State in FSM. This member function accepts the instance of a class
       *        and that class has to define receive as well as onEntry and onExit member function.Those member functions
       *        are assigned to delegate.
       * @param inst reference to an instance to the class
       * @return none.
       * */
      template<typename C>
      void setState(C& inst)
      {
        if(on_receive && on_exit) {
          /* invoking upon state exit */
          on_exit(m_parent);
          on_exit.reset();
          on_receive.reset();
          on_entry.reset();
        }

        /* assigning the respective delegates */
        on_receive = receive_delegate_t::from<C, &C::receive>(inst);
        on_exit = state_delegate_t::from<C, &C::onExit>(inst);
        on_entry = state_delegate_t::from<C, &C::onEntry>(inst);

        /* invoking upon state entry */
        on_entry(m_parent);
      }

      /**
       * @brief This member function is used to get the reference of the instance of state class. The state is represented
       *        calss and that class has to  implement certain member method.
       * @param none
       * @return Reference to FSM instance.
       * */
      FSM& getState()
      {
        return(*this);
      }

      /**
       * @brief This member function is invoked from respective state which inturn invokes delegates.
       * @param pointer to parent who has instantiated the FSM class.
       * @param pointer to const char of input buffer.
       * @param length of received length.
       * @return whatever deledagte returns.
       * */
      int32_t rx(const uint8_t* inPtr, uint32_t inLen) const
      {
        return(on_receive(m_parent, inPtr, inLen));
      }

      private:
        receive_delegate_t on_receive;
        state_delegate_t on_entry;
        state_delegate_t on_exit;
        void* m_parent;
  };

  namespace eth {

    typedef struct ETH
    {
      /*destination MAC Address.*/
      char dest[6];
      /*source MAC Address.*/
      char src[6];
      /*Packet Type ID.*/
      uint16_t proto;
    }__attribute__((packed))ETH;

    enum proto_t : uint16_t {
      IPv4 = 0x0800,
      IPv6 = 0x86DD,
      ARP = 0x0806,
      EAPOL = 0x888E,
      PPP = 0x880B
    };

    class ether {
      public:
        using upstream_t = delegate<int32_t (const uint8_t*, uint32_t)>;
        using downstream_t = delegate<int32_t (uint8_t*, size_t)>;

        ether(const std::string& intf) : m_intf(intf)
        {
          m_src_mac.fill(0);
          m_dst_mac.fill(0);
        }

        ether(const ether& ) = default;
        ether(ether&& ) = default;
        ~ether() = default;

        int32_t rx(const uint8_t* ethPacket, uint32_t packetLen);
        int32_t tx(uint8_t* ethPacket, size_t packetLen);
        uint16_t checksum(const uint16_t* in, size_t inLen) const;

        void set_upstream(upstream_t us)
        {
          m_upstream = us;
        }

        upstream_t& get_upstream()
        {
          return(m_upstream);
        }

        void set_downstream(downstream_t ds)
        {
          m_downstream = ds;
        }

        downstream_t& get_downstream()
        {
          return(m_downstream);
        }

        void src_mac(std::array<uint8_t, 6> smac)
        {
          m_src_mac = smac;
        }

        std::array<uint8_t, 6>& src_mac()
        {
          return(m_src_mac);
        }

        void dst_mac(std::array<uint8_t, 6> dmac)
        {
          m_dst_mac = dmac;
        }

        std::array<uint8_t, 6>& dst_mac()
        {
          return(m_dst_mac);
        }

        void index(uint32_t idx)
        {
          m_index = idx;
        }

        uint32_t index()
        {
          return(m_index);
        }

        uint16_t proto()
        {
          return(m_proto);
        }

        void proto(uint16_t pro)
        {
          m_proto = pro;
        }

      private:
        upstream_t m_upstream;
        downstream_t m_downstream;
        std::string m_intf;
        uint32_t m_index;
        std::array<uint8_t, 6> m_src_mac;
        std::array<uint8_t, 6> m_dst_mac;
        uint16_t m_proto;
    };

  }

  namespace ipv4 {

    typedef struct IP {
      uint32_t len:4;
      uint32_t ver:4;
      uint32_t tos:8;
      uint32_t tot_len:16;

      uint16_t id;
      /*Fragment Offset.*/
      uint16_t flags;

      uint32_t ttl:8;
      uint32_t proto:8;
      uint32_t chksum:16;

      uint32_t src_ip;
      uint32_t dest_ip;
    }__attribute__((packed))IP;

    enum proto_t : uint8_t {
      ICMP = 1,
      TCP = 6,
      UDP = 17,
      L2TP = 112
    };

    class ip {
      public:
        using upstream_t = delegate<int32_t (const uint8_t*, uint32_t)>;
        using downstream_t = delegate<int32_t (uint8_t*, size_t)>;

        ip() = default;
        ip(const ip& ) = default;
        ip(ip&& ) = default;
        ~ip() = default;

        int32_t rx(const uint8_t* ip, uint32_t ipLen);
        int32_t tx(uint8_t* ip, size_t ipLen);
        uint16_t checksum(const uint16_t* in, size_t inLen) const;
        uint16_t compute_checksum(uint8_t* in, size_t inLen);

        void set_upstream(upstream_t us)
        {
          m_upstream = us;
        }

        void set_downstream(downstream_t ds)
        {
          m_downstream = ds;
        }

        downstream_t& get_downstream()
        {
          return(m_downstream);
        }

        void src_ip(uint32_t sip)
        {
          m_src_ip = sip;
        }

        uint32_t src_ip()
        {
          return(m_src_ip);
        }

        void dst_ip(uint32_t dip)
        {
          m_dst_ip = dip;
        }

        uint32_t dst_ip()
        {
          return(m_dst_ip);
        }

        void proto(uint8_t pro)
        {
          m_proto = pro;
        }

        uint8_t proto() const
        {
          return(m_proto);
        }

      private:

        upstream_t m_upstream;
        downstream_t m_downstream;

        uint32_t m_src_ip;
        uint32_t m_dst_ip;
        uint8_t m_proto;
    };

  }

  namespace ipv6 {
    class ip {
    };

  }

  namespace transport {

    typedef struct UDP {
      uint16_t src_port;
      uint16_t dest_port;
      uint16_t len;
      uint16_t chksum;
    }__attribute__((packed))UDP;

    typedef struct TCP {
      uint16_t src_port;
      uint16_t dest_port;
      uint32_t seq_no;
      uint32_t ack_no;
      uint16_t data_off:4;
      uint16_t reserved:3;
      uint16_t ecn:3;
      uint16_t u:1;
      uint16_t a:1;
      uint16_t p:1;
      uint16_t r:1;
      uint16_t s:1;
      uint16_t f:1;
      uint16_t window;
      uint16_t check_sum;
      uint16_t u_pointer;
    }__attribute__((packed))TCP;

    /*pseudo TCP Header for calculating checksum.*/
    typedef struct PHDR {
      uint32_t src_ip;
      uint32_t dest_ip;
      uint32_t reserve:8;
      uint32_t proto:8;
      uint32_t total_len:16;
    }__attribute__((packed))PHDR;

    enum port_list : uint16_t {
      BOOTPS = 67,
      BOOTPC = 68,
      DNS = 53,
      HTTP = 80,
      RADIUS_AUTH = 1812,
      RADIUS_ACC = 1813,
      HTTPS = 443
    };

    class udp {
      public:
        using upstream_t = delegate<int32_t (const uint8_t*, uint32_t)>;
        using downstream_t = delegate<int32_t (uint8_t*, size_t)>;

        udp() = default;
        udp(const udp& ) = default;
        udp(udp&& ) = default;
        ~udp() = default;

        int32_t rx(const uint8_t* udp, uint32_t udpLen);
        int32_t tx(uint8_t* ip, size_t ipLen);

        void set_upstream(upstream_t us)
        {
          m_upstream = us;
        }

        void set_downstream(downstream_t ds)
        {
          m_downstream = ds;
        }

        downstream_t& get_downstream()
        {
          return(m_downstream);
        }

        void src_port(uint16_t port)
        {
          m_src_port = port;
        }

        uint16_t src_port() const
        {
          return(m_src_port);
        }

        void dst_port(uint16_t port)
        {
          m_dst_port = port;
        }

        uint16_t dst_port() const
        {
          return(m_dst_port);
        }

      private:
        upstream_t m_upstream;
        downstream_t m_downstream;
        uint16_t m_src_port;
        uint16_t m_dst_port;
    };

    class tcp {
      public:
        using upstream_t = delegate<int32_t (const uint8_t*, uint32_t)>;
        using downstream_t = delegate<int32_t (uint8_t*, size_t)>;

        tcp() = default;
        tcp(const tcp& ) = default;
        tcp(tcp&& ) = default;
        ~tcp() = default;

        int32_t rx(const uint8_t* in, uint32_t inLen);
        int32_t tx(uint8_t* in, size_t inLen);

        void set_upstream(upstream_t us)
        {
          m_upstream = us;
        }

        void set_downstream(downstream_t ds)
        {
          m_downstream = ds;
        }

        downstream_t& get_downstream()
        {
          return(m_downstream);
        }

        void src_port(uint16_t port)
        {
          m_src_port = port;
        }

        uint16_t src_port() const
        {
          return(m_src_port);
        }

        void dst_port(uint16_t port)
        {
          m_dst_port = port;
        }

        uint16_t dst_port() const
        {
          return(m_dst_port);
        }

      private:
        upstream_t m_upstream;
        downstream_t m_downstream;
        uint16_t m_src_port;
        uint16_t m_dst_port;
    };

    class tun {
    };

  }

  namespace dns {
  }

  class arp {
  };

  /**
   * @brief
   * */
  namespace dhcp {

    enum message_type_t : uint8_t {
      /*DHCP Message Type*/
      DISCOVER = 1,
      OFFER = 2,
      REQUEST = 3,
      DECLINE = 4,
      ACK = 5,
      NACK = 6,
      RELEASE = 7,
      INFORM = 8
    };

    enum option_t : uint8_t {
      /*DHCP OPTIONS*/
      PADD = 0,
      SUBNET_MASK = 1,
      ROUTER = 3,
      TIME_SERVER = 4,
      NAME_SERVER = 5,
      DNS = 6,
      LOG_SERVER = 7,
      QUOTE_SERVER = 8,
      IMPRESS_SERVER = 10,
      ROUTER_LOCATION_SERVER = 11,
      HOST_NAME = 12,
      DOMAIN_NAME = 15,
      /*Interface MTU*/
      MTU = 26,
      BROADCAST_ADDRESS = 28,
      NIS_DOMAIN = 40,
      NIS = 41,
      NTP_SERVER = 42,
      VENDOR_SPECIFIC_INFO = 43,
      REQUESTED_IP_ADDRESS = 50,
      IP_LEASE_TIME = 51,
      OVERLOAD = 52,
      MESSAGE_TYPE = 53,
      SERVER_IDENTIFIER = 54,
      PARAMETER_REQUEST_LIST = 55,
      MESSAGE = 56,
      MESSAGE_SIZE = 57,
      RENEWAL_TIME_T1 = 58,
      RENEWAL_TIME_T2 = 59,
      CLASS_IDENTIFIER = 60,
      CLIENT_IDENTIFIER = 61,
      RAPID_COMMIT = 80,
      AUTO_CONFIGURE = 116,
      END = 255

    };

    /** Element TLV received in request (DISCOVER, REQUEST) */
    struct element_def_t {

      uint8_t m_tag;
      uint8_t m_len;
      std::array<uint8_t, 255> m_val;

      public:

        ~element_def_t() = default;

        element_def_t()
        {
          m_tag = 0;
          m_len = 0;
          m_val.fill(0);
        }

        /*Forcing a copy constructor to be generated by the compiler.*/
        element_def_t(const element_def_t& elm)
        {
          set_tag(elm.get_tag());
          set_len(elm.get_len());
          set_val(elm.get_val());
        }

        element_def_t(element_def_t&& elem) noexcept = default;

        element_def_t& operator=(element_def_t&& elem) = default;

        element_def_t& operator=(const element_def_t& elem)
        {
          m_tag = elem.m_tag;
          m_len = elem.m_len;
          m_val = elem.m_val;
          return *this;
        }

        /**
         * @brief by adding const qualifier ensures that this method does not
         * modify the private data member.This is a way to make this as pointer to const.*/
        const std::array<uint8_t, 255>& get_val() const
        {
          return(m_val);
        }

        uint8_t get_tag() const
        {
          return(m_tag);
        }

        uint8_t get_len() const
        {
          return(m_len);
        }

        void set_tag(uint8_t tag)
        {
          m_tag = tag;
        }

        void set_len(uint8_t len)
        {
          m_len = len;
        }

        void set_val(const std::array<uint8_t, 255> &val)
        {
          m_val = val;
        }
    };

    class server;
    class dhcpEntry;
    class OnDiscover {
      public:

        OnDiscover() = default;
        OnDiscover(const OnDiscover& ) = default;
        OnDiscover(OnDiscover&& ) = default;

        ~OnDiscover() = default;

        void onEntry(void* parent);
        void onExit(void* parent);

        /**
         * @brief This is a delegate member function which is invoked from FSM Class.
         * */
        int32_t receive(void* parent, const uint8_t *inPtr, uint32_t inLen);

        static OnDiscover& instance()
        {
          /* NB: static variable is instantiated only once.*/
          std::call_once(m_flag, []() { m_instance = std::make_unique<OnDiscover>();});
          return(*m_instance.get());
        }

      private:

        inline static std::once_flag  m_flag;
        inline static std::unique_ptr<OnDiscover> m_instance;
    };

    class OnRequest {
      public:

        OnRequest() = default;
        OnRequest(const OnRequest& ) = default;
        OnRequest(OnRequest&& ) = default;

        ~OnRequest() = default;

        void onEntry(void* );
        void onExit(void* );

        /**
         * @brief This is a delegate member function which is invoked from FSM Class.
         * */
        int32_t receive(void* parent, const uint8_t *inPtr, uint32_t inLen);

        static OnRequest& instance()
        {
          std::call_once(m_flag, []() { m_instance = std::make_unique<OnRequest>();});
          return(*m_instance.get());
        }

      private:

        inline static std::once_flag  m_flag;
        inline static std::unique_ptr<OnRequest> m_instance;
    };

    class OnRelease {
      public:

        OnRelease() = default;
        OnRelease(const OnRelease& ) = default;
        OnRelease(OnRelease&& ) = default;
        ~OnRelease() = default;

        void onEntry(void* );
        void onExit(void* );

        /**
         * @brief This is a delegate member function which is invoked from FSM Class.
         * */
        int32_t receive(void* parent, const uint8_t *inPtr, uint32_t inLen);

        static OnRelease& instance()
        {
          std::call_once(m_flag, []() { m_instance = std::make_unique<OnRelease>();});
          return(*m_instance.get());
        }

      private:

        inline static std::once_flag  m_flag;
        inline static std::unique_ptr<OnRelease> m_instance;
    };

    class OnInform {
      public:
        OnInform() = default;
        OnInform(const OnInform& ) = default;
        OnInform(OnInform&& ) = default;
        ~OnInform() = default;

        void onEntry(void* );
        void onExit(void* );

        /**
         * @brief This is a delegate member function which is invoked from FSM Class.
         * */
        int32_t receive(void* parent, const uint8_t *inPtr, uint32_t inLen);

        static OnInform& instance()
        {
          std::call_once(m_flag, []() { m_instance = std::make_unique<OnInform>();});
          return(*m_instance.get());
        }

      private:

        inline static std::once_flag  m_flag;
        inline static std::unique_ptr<OnInform> m_instance;
    };

    class OnDecline {
    };


    class OnLeaseExpire {
      public:
        OnLeaseExpire() = default;
        OnLeaseExpire(const OnLeaseExpire& ) = default;
        OnLeaseExpire(OnLeaseExpire&& ) = default;
        ~OnLeaseExpire() = default;

        void onEntry(void* );
        void onExit(void* );

        /**
         * @brief This is a delegate member function which is invoked from FSM Class.
         * */
        int32_t receive(void* parent, const uint8_t *inPtr, uint32_t inLen);

        static OnLeaseExpire& instance()
        {
          std::call_once(m_flag, []() { m_instance = std::make_unique<OnLeaseExpire>();});
          return(*m_instance.get());
        }

      private:

        inline static std::once_flag  m_flag;
        inline static std::unique_ptr<OnLeaseExpire> m_instance;
    };

    typedef struct {
      uint8_t op;
      uint8_t htype;
      uint8_t hlen;
      uint8_t hops;
      /*Random Transaction ID.*/
      uint32_t xid;
      uint16_t secs;
      uint16_t flags;
      uint32_t ciaddr;
      uint32_t yiaddr;
      uint32_t siaddr;
      uint32_t giaddr;
      /*client's MAC Address.*/
      uint8_t chaddr[16];
      uint8_t sname[64];
      uint8_t file[128];
    }__attribute__((packed))dhcp_t;


    using element_def_UMap_t = std::unordered_map<uint8_t, element_def_t>;

    struct profile {
      uint16_t m_mtu;
      std::string m_primaryDns;
      std::string m_secondaryDns;
      std::string m_domainName;
      uint32_t m_lease;
    };

    struct virtualNw {
      std::string m_type;
      /*! Name of the ethernet port.*/
      std::string m_port;
      uint16_t m_mtu;
    };

    struct configParam {
      profile m_profile;
      virtualNw m_virtualNw;
      std::string m_subnetMask;
      std::string m_ip;
      std::string m_hostName;
      std::string m_startIP;
      std::string m_endIP;
      std::vector<std::string> m_excludeIPs;
    };

    class serverConfig {
      public:
        serverConfig(const serverConfig& ) = default;
        serverConfig(serverConfig&& ) = default;
        ~serverConfig() = default;
        serverConfig& operator=(configParam& rhs)
        {
          std::swap(m_config, rhs);
          return *this;
        }

        serverConfig(std::string fname);

        std::string& type()
        {
          return(m_config.m_virtualNw.m_type);
        }

        void type(std::string m)
        {
          m_config.m_virtualNw.m_type = m;
        }

        std::string& port()
        {
          return(m_config.m_virtualNw.m_port);
        }

        void port(std::string m)
        {
          m_config.m_virtualNw.m_port = m;
        }

        uint16_t pmtu()
        {
          return(m_config.m_virtualNw.m_mtu);
        }

        void pmtu(uint16_t m)
        {
          m_config.m_virtualNw.m_mtu = m;
        }

        std::vector<std::string>& excludeIP()
        {
          return(m_config.m_excludeIPs);
        }

        void excludeIP(std::string m)
        {
          m_config.m_excludeIPs.push_back(m);
        }


        std::string& startIP()
        {
          return(m_config.m_startIP);
        }

        void startIP(std::string m)
        {
          m_config.m_startIP = m;
        }

        std::string& endIP()
        {
          return(m_config.m_endIP);
        }

        void endIP(std::string m)
        {
          m_config.m_endIP = m;
        }

        std::string& ip()
        {
          return(m_config.m_ip);
        }

        void ip(std::string m)
        {
          m_config.m_ip = m;
        }

        std::string& mask()
        {
          return(m_config.m_subnetMask);
        }

        void mask(std::string m)
        {
          m_config.m_subnetMask = m;
        }

        uint16_t mtu() const
        {
          return(m_config.m_profile.m_mtu);
        }

        void mtu(uint16_t m)
        {
          m_config.m_profile.m_mtu = m;
        }

        uint32_t lease() const
        {
          return(m_config.m_profile.m_lease);
        }

        void lease(uint32_t l)
        {
          m_config.m_profile.m_lease = l;
        }

        std::string& hostName()
        {
          return(m_config.m_hostName);
        }

        void hostName(std::string h)
        {
          m_config.m_hostName = h;
        }

        std::string& primaryDns()
        {
          return(m_config.m_profile.m_primaryDns);
        }

        void primaryDns(std::string pdns)
        {
          m_config.m_profile.m_primaryDns = pdns;
        }

        std::string& secondaryDns()
        {
          return(m_config.m_profile.m_secondaryDns);
        }

        void secondaryDns(std::string sdns)
        {
          m_config.m_profile.m_secondaryDns = sdns;
        }

        std::string& domainName()
        {
          return(m_config.m_profile.m_domainName);
        }

        void domainName(std::string dname)
        {
          m_config.m_profile.m_domainName = dname;
        }

        void fillIPPool(std::string startIP, std::string endIP, std::string subnetMASK)
        {
          struct in_addr IP_s;
          struct in_addr IP_e;
          struct in_addr MASK;

          uint32_t hostAddr;
          uint32_t nwAddr;

          m_poolIPs.clear();
          m_allocatedIPs.clear();

          /*! transforming from dotted notation into decimal/integer number.*/
          inet_aton(startIP.c_str(), &IP_s);
          inet_aton(endIP.c_str(), &IP_e);
          inet_aton(subnetMASK.c_str(), &MASK);

          hostAddr = IP_s.s_addr & MASK.s_addr;
          nwAddr = IP_s.s_addr & (~MASK.s_addr);
          std::cout << "hostAddr " << hostAddr << " nwAddr " << nwAddr << std::endl;

        }

      private:
        configParam m_config;
        /*! Assigned IP Pools*/
        std::vector<std::string> m_allocatedIPs;
        /*! IP to be allocated from Pools*/
        std::vector<std::string> m_poolIPs;
    };

    class dhcpEntry {
      public:
        using downstream_t = delegate<int32_t (uint8_t* in, size_t inLen)>;

        using start_timer_t = delegate<long (uint32_t, const void*, bool)>;
        using stop_timer_t = delegate<void (long)>;
        using reset_timer_t = delegate<void (long, uint32_t)>;

        /** This UMap stores DHCP Option received in DISCOVER/REQUEST. */
        element_def_UMap_t m_elemDefUMap;

        dhcpEntry(const dhcpEntry& fsm) = default;
        dhcpEntry(dhcpEntry&& fsm) = default;

        dhcpEntry()
        {
          m_fsm = std::make_unique<FSM>(this);
          m_parent = nullptr;
        }

        ~dhcpEntry()
        {
        }

        dhcpEntry(server* parent, uint32_t clientIP);
#if 0
        dhcpEntry(server* parent, uint32_t clientIP)
        {
          m_fsm = std::make_unique<FSM>(this);
          m_parent = parent;
          std::swap(m_clientIP, clientIP);
          char hname[255];
          gethostname(hname, sizeof(hname));
          parent->m_config.hostName().assign((const char* )hname);
          /* Initializing the State Machine. */
          setState(OnDiscover::instance());
        }
#endif
        FSM& getState() const
        {
          return((*m_fsm.get()).getState());
        }

        template<typename C>
        void setState(C& inst)
        {
          (*m_fsm.get()).setState<C>(inst);
        }

        int32_t rx(const uint8_t* in, uint32_t inLen);

        int32_t parseOptions(const uint8_t* in, uint32_t inLen);
        int32_t buildAndSendResponse(const uint8_t* in, uint32_t inLen);
        int32_t tx(uint8_t* out, size_t outLen);

        /** Timer related API. */
        long startTimer(uint32_t delay, const void* txn);
        void stopTimer(long tid);

        std::array<uint8_t, 6> get_chaddr() const
        {
          return(m_chaddr);
        }

        long get_tid() const
        {
          return(m_tid);
        }

        void set_tid(long tid)
        {
          m_tid = tid;
        }

        void set_start_timer(start_timer_t st)
        {
          m_start_timer = st;
        }

        start_timer_t& get_start_timer()
        {
          return(m_start_timer);
        }

        void set_stop_timer(stop_timer_t st)
        {
          m_stop_timer = st;
        }

        stop_timer_t& get_stop_timer()
        {
          return(m_stop_timer);
        }

        void set_reset_timer(reset_timer_t rt)
        {
          m_reset_timer = rt;
        }

        void set_downstream(downstream_t ds)
        {
          m_downstream = ds;
        }

        downstream_t& get_downstream()
        {
          return(m_downstream);
        }

      private:

        start_timer_t m_start_timer;
        stop_timer_t m_stop_timer;
        reset_timer_t m_reset_timer;

        downstream_t m_downstream;
        /* Per DHCP Client State Machine. */
        std::unique_ptr<FSM> m_fsm;
        /*backpointer to dhcp server.*/
        server* m_parent;
        /* The IP address allocated/Offered to DHCP Client. */
        uint32_t m_clientIP;
        /* Unique transaction ID of message received. */
        uint32_t m_xid;
        /* The DHCP Client MAC Address. */
        std::array<uint8_t, 6> m_chaddr;
        /** The timer ID*/
        long m_tid;
    };

    using dhcp_entry_onMAC_t = std::unordered_map<std::string, std::unique_ptr<dhcpEntry>>;
    using dhcp_entry_onIP_t = std::unordered_map<uint32_t, dhcpEntry>;

    class server {

      public:

        using upstream_t = delegate<int32_t (const uint8_t* in, uint32_t inLen)>;
        using downstream_t = delegate<int32_t (uint8_t* in, size_t inLen)>;

        using start_timer_t = delegate<long (uint32_t, const void*, bool)>;
        using stop_timer_t = delegate<void (long)>;
        using reset_timer_t = delegate<void (long, uint32_t)>;
        using to_timer_t = delegate<long (const void*)>;

        dhcp_entry_onMAC_t m_dhcpUmapOnMAC;
        dhcp_entry_onIP_t m_dhcpUmapOnIP;

        server(const server& ) = default;
        server(server&& ) = default;

        server()
        {
        }

        ~server()
        {
          #if 0
          dhcp_entry_onMAC_t::const_iterator it;
          for(it = m_dhcpUmapOnMAC.begin(); it != m_dhcpUmapOnMAC.end(); ++it) {
            dhcpEntry *dEnt = it->second;
            delete dEnt;
          }
          #endif
        }

        int32_t rx(const uint8_t* in, uint32_t inLen);
        long timedOut(const void* txn);

        void set_upstream(upstream_t us)
        {
          m_upstream = us;
        }

        void set_start_timer(start_timer_t st)
        {
          m_start_timer = st;
        }

        void set_stop_timer(stop_timer_t st)
        {
          m_stop_timer = st;
        }

        void set_reset_timer(reset_timer_t rt)
        {
          m_reset_timer = rt;
        }

        void set_downstream(downstream_t ds)
        {
          m_downstream = ds;
        }

        downstream_t& get_downstream()
        {
          return(m_downstream);
        }

        serverConfig& config()
        {
          return(*m_config.get());
        }

        void set_config(std::unique_ptr<serverConfig> cfg)
        {
          m_config = std::move(cfg);
        }

      private:

        start_timer_t m_start_timer;
        stop_timer_t m_stop_timer;
        reset_timer_t m_reset_timer;

        upstream_t m_upstream;
        downstream_t m_downstream;

        std::unique_ptr<serverConfig> m_config;
    };
  }

}


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
        using receive_t = delegate<int32_t (const uint8_t* in, ssize_t inLen)>;
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

        void tx(send_t r)
        {
          m_tx = r;
        }

        send_t tx()
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
        int32_t buildWanIPUpdateRequest(std::string& req, vddnsPeer& peer);
        int32_t processWanIPUpdateResponse(std::string& req);

      private:
        std::unique_ptr<config> m_config;
        std::string m_wanIP;
        receive_t m_rx;
        send_t m_tx;
        connect_t m_connect;
    };
  }
}










#endif /*__PROTOCOL_H__*/
