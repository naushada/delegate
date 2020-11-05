#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <iostream>
#include <delegate.hpp>
#include <array>
#include <unordered_map>
#include <algorithm>
#include <cstring>
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
       * @param reference to an instance to the class
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

        ether(const std::string& intf) : m_intf(intf)
        {
        }

        ether(const ether& ) = default;
        ether(ether&& ) = default;
        ~ether() = default;

        int32_t rx(const uint8_t* ethPacket, uint32_t packetLen)
        {
          std::cout << "eth::receive "<< std::endl;
          return(m_upstream(ethPacket, packetLen));
        }

        void set_upstream(upstream_t us)
        {
          m_upstream = us;
        }

        upstream_t& get_upstream()
        {
          return(m_upstream);
        }

      private:
        upstream_t m_upstream;
        std::string m_intf;
        uint32_t m_index;
    };

  }

  namespace ipv4 {
    class ip {
      public:
        using upstream_t = delegate<int32_t (const uint8_t*, uint32_t)>;

        ip() = default;
        ip(const ip& ) = default;
        ip(ip&& ) = default;
        ~ip() = default;

        int32_t rx(const uint8_t* ip, uint32_t ipLen)
        {
          std::cout << "ip::receive "<< std::endl;
          return(m_upstream(ip, ipLen));
        }

        void set_upstream(upstream_t us)
        {
          m_upstream = us;
        }

      private:
        upstream_t m_upstream;
    };

  }

  namespace ipv6 {
    class ip {
    };

  }

  namespace transport {

    class udp {
      public:
        using upstream_t = delegate<int32_t (const uint8_t*, uint32_t)>;

        udp() = default;
        udp(const udp& ) = default;
        udp(udp&& ) = default;
        ~udp() = default;

        int32_t rx(const uint8_t* udp, uint32_t udpLen)
        {
          std::cout << "udp::receive "<< std::endl;
          return(m_upstream(udp, udpLen));
        }

        void set_upstream(upstream_t us)
        {
          m_upstream = us;
        }

      private:
        upstream_t m_upstream;
    };

    class tcp {
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
          static OnDiscover m_instance;
          return(m_instance);
        }
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
          static OnRequest m_instance;
          return(m_instance);
        }
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
          static OnRelease m_instance;
          return(m_instance);
        }
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
          static OnInform m_instance;
          return(m_instance);
        }
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
          static OnLeaseExpire m_instance;
          return(m_instance);
        }
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


    class dhcpEntry {
      public:
        /** This UMap stores DHCP Option received in DISCOVER/REQUEST. */
        element_def_UMap_t m_elemDefUMap;

        dhcpEntry(const dhcpEntry& fsm) = default;
        dhcpEntry(dhcpEntry&& fsm) = default;

        dhcpEntry()
        {
          m_fsm = new FSM(this);
        }

        ~dhcpEntry()
        {
        }


        dhcpEntry(server* parent, uint32_t clientIP, uint32_t routerIP, uint32_t dnsIP,
                  uint32_t lease, uint32_t mtu, uint32_t serverID, std::string domainName)
        {
          m_fsm = new FSM(this);
          m_parent = parent;
          std::swap(m_clientIP, clientIP);
          std::swap(m_routerIP, routerIP);
          std::swap(m_dnsIP, dnsIP);
          std::swap(m_lease, lease);
          std::swap(m_mtu, mtu);
          std::swap(m_serverID, serverID);
          std::swap(m_domainName, domainName);

          /* Initializing the State Machine. */
          setState(OnDiscover::instance());
        }

        FSM& getState() const
        {
          return(m_fsm->getState());
        }

        template<typename C>
        void setState(C& inst)
        {
          m_fsm->setState<C>(inst);
        }

        int32_t rx(const uint8_t* in, uint32_t inLen);

        int32_t parseOptions(const uint8_t* in, uint32_t inLen);
        int32_t buildAndSendResponse(const uint8_t* in, uint32_t inLen);
        int32_t tx(uint8_t* out, uint32_t outLen);

        /** Timer related API. */
        long startTimer(uint32_t delay, const void* txn);
        void stopTimer(long tid);

        uint32_t get_lease() const
        {
          return(m_lease);
        }

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

      private:

        /* Per DHCP Client State Machine. */
        FSM* m_fsm;
        /*backpointer to dhcp server.*/
        server* m_parent;
        /* The IP address allocated/Offered to DHCP Client. */
        uint32_t m_clientIP;
        /* Unique transaction ID of message received. */
        uint32_t m_xid;
        /* The Router IP for DHCP Client. */
        uint32_t m_routerIP;
        /* The Domain Name Server IP. */
        uint32_t m_dnsIP;
        /* The validit of Offered IP address to DHCP Client. */
        uint32_t m_lease;
        /* The size of Ethernet Packet. */
        uint32_t m_mtu;
        /* The DHCP Server Identifier - Which is IP Address. */
        uint32_t m_serverID;
        /* The DHCP Client MAC Address. */
        std::array<uint8_t, 6> m_chaddr;
        /* The Domain Name to be assigned to DHCP Client. */
        std::string m_domainName;
        /* Name of Machine on which DHCP server is running. */
        std::string m_hostName;
        /** The timer ID*/
        long m_tid;
    };

    using dhcp_entry_onMAC_t = std::unordered_map<std::string, dhcpEntry*>;
    using dhcp_entry_onIP_t = std::unordered_map<uint32_t, dhcpEntry>;

    class server {

      public:
        using upstream_t = delegate<int32_t (const uint8_t* in, uint32_t inLen)>;

        dhcp_entry_onMAC_t m_dhcpUmapOnMAC;
        dhcp_entry_onIP_t m_dhcpUmapOnIP;

        server() = default;
        server(const server& ) = default;
        server(server&& ) = default;

        ~server()
        {
          dhcp_entry_onMAC_t::const_iterator it;
          for(it = m_dhcpUmapOnMAC.begin(); it != m_dhcpUmapOnMAC.end(); ++it) {
            dhcpEntry *dEnt = it->second;
            delete dEnt;
          }
        }

        int32_t rx(const uint8_t* in, uint32_t inLen);
        long timedOut(const void* txn);
        void set_upstream(upstream_t us)
        {
          m_upstream = us;
        }

      private:

        upstream_t m_upstream;
        /* The Router IP for DHCP Client. */
        uint32_t m_routerIP;
        /* The Domain Name Server IP. */
        uint32_t m_dnsIP;
        /* The validit of Offered IP address to DHCP Client. */
        uint32_t m_lease;
        /* The size of Ethernet Packet. */
        uint32_t m_mtu;
        /* The DHCP Server Identifier - Which is IP Address. */
        uint32_t m_serverID;
        /* The Domain Name to be assigned to DHCP Client. */
        std::string m_domainName;
    };

  }

}










#endif /*__PROTOCOL_H__*/
