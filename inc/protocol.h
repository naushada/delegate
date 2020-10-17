#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <iostream>
#include <delegate.hpp>
#include <array>
#include <unordered_map>
#include <algorithm>
#include <cstring>

namespace mna {

  class FSM {

    public:
      /* Type Aliasing */
      /**
       * @brief : parent is the instance of a class that instantiating this FSM.
       *          the next argument is the received requiest buffer and the last
       *          argument is the length of received request.
       * */
      using receive_delegate_t = delegate<int32_t (void* parent, const char*, uint32_t)>;
      using state_delegate_t = delegate<void ()>;

      FSM()
      {
        on_receive.reset();
        on_exit.reset();
        on_entry.reset();
      }

      FSM(const FSM& fsm) = default;
      FSM(FSM&& fsm) = default;

      ~FSM() = default;

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
          on_exit();
          on_exit.reset();
          on_receive.reset();
          on_entry.reset();
      }

        /* assigning the respective delegates */
        on_receive = receive_delegate_t::from<C, &C::receive>(inst);
        on_exit = state_delegate_t::from<C, &C::onExit>(inst);
        on_entry = state_delegate_t::from<C, &C::onEntry>(inst);

        /* invoking upon state entry */
        on_entry();
      }

      /**
       * @brief
       * @param
       * @return
       * */
      FSM& getState()
      {
        return(*this);
      }

      /**
       * @brief
       * @param
       * @return
       * */
      int32_t rx(void* parent, const char *inPtr, uint32_t inLen) const
      {
        return(on_receive(parent, inPtr, inLen));
      }

      private:
        receive_delegate_t on_receive;
        state_delegate_t on_entry;
        state_delegate_t on_exit;

  };

  namespace eth {
  };

  namespace ipv4 {
  };

  namespace udp {
  };

  namespace tcp {
  };

  namespace tun {
  };

  namespace dns {
  };

  namespace arp {
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
      RELEASE = 7
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

      private:
        uint8_t m_tag;
        uint8_t m_len;
        std::array<uint8_t, 255> m_val;

      public:

        //element_def_t() = default;
        ~element_def_t() = default;

        element_def_t()
        {
          m_tag = 0;
          m_len = 0;
        }

        element_def_t(const element_def_t& elem) = default;
        element_def_t(element_def_t&& elem) = default;

        element_def_t& operator=(element_def_t&& elem) = default;

        element_def_t& operator=(element_def_t& elem)
        {
          std::swap(m_tag, elem.m_tag);
          std::swap(m_len, elem.m_len);
          std::swap(m_val, elem.m_val);

          return *this;
        }

        /**
         * @brief by adding const qualifier ensures that this method does not
         * modify the private data member.*/
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

    class dhcpEntry;
    class OnRequest;

    class OnDiscover {
      public:

        OnDiscover() = default;
        OnDiscover(const OnDiscover& ) = default;
        OnDiscover(OnDiscover&& ) = default;

        ~OnDiscover() = default;

        void onEntry()
        {
          std::cout << "5.OnDiscover::onEntry is invoked " << std::endl;
        }

        void onExit()
        {
          std::cout << "5.OnDiscover::onExit is invoked " << std::endl;
        }

        /**
         * @brief This is a delegate member function which is invoked from FSM Class.
         * */
        int32_t receive(void* parent, const char *inPtr, uint32_t inLen)
        {
          std::cout << "6.OnDiscover::receive ---> " << inPtr << "inLen " << inLen << std::endl;
          //dhcpEntry *dEnt = const_cast<dhcpEntry *>(reinterpret_cast<const dhcpEntry *>(parent));
          dhcpEntry *dEnt = (dhcpEntry*)(parent);
          //dEnt->setState<OnRequest>(dEnt->m_instRequest);
          //dEnt->setState<OnRequest>(dEnt->m_instRequest);
          /* Start Processing DHCP DISCOVER Request.*/
          return(REQUEST);
        }
    };

    class OnRequest {
      public:

        OnRequest() = default;
        OnRequest(const OnRequest& ) = default;
        OnRequest(OnRequest&& ) = default;

        ~OnRequest() = default;

        void onEntry()
        {
          std::cout << "OnRequest::onEntry is invoked " << std::endl;
        }

        void onExit()
        {
          std::cout << "OnRequest::onExit is invoked " << std::endl;
        }

        /**
         * @brief This is a delegate member function which is invoked from FSM Class.
         * */
        int32_t receive(void* parent, const char *inPtr, uint32_t inLen)
        {
          std::cout << "OnRequest::receive ---> " << inPtr << "inLen " << inLen << std::endl;
          //dhcpEntry *dEnt = const_cast<dhcpEntry *>(reinterpret_cast<const dhcpEntry *>(parent));
          //dEnt->getState().setState<OnRequest>(dEnt->m_instRequest);
          //dEnt->setState<OnRequest>(dEnt->m_instRequest);
          /* Start Processing DHCP DISCOVER Request.*/
          return(RELEASE);
       }
    };

    class OnRelease {
      public:

      OnRelease() = default;
      OnRelease(const OnRelease& ) = default;
      OnRelease(OnRelease&& ) = default;

      ~OnRelease() = default;

        void onEntry()
        {
          std::cout << "OnRelease::onEntry is invoked " << std::endl;
        }

        void onExit()
        {
          std::cout << "OnRelease::onExit is invoked " << std::endl;
        }

        /**
         * @brief This is a delegate member function which is invoked from FSM Class.
         * */
        int32_t receive(void* parent, const char *inPtr, uint32_t inLen)
        {
          std::cout << "OnRelease::receive " << std::endl;
          //dhcpEntry *dEnt = const_cast<dhcpEntry *>(reinterpret_cast<const dhcpEntry *>(parent));
          //dEnt->getState().setState<OnRequest>(dEnt->m_instRequest);
          //dEnt->setState<OnRequest>(dEnt->m_instRequest);
          /* Start Processing DHCP DISCOVER Request.*/
          return(DISCOVER);
        }

    };

    class OnInform {
    };

    class OnDecline {
    };


    class OnLeaseExpire {
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


    class dhcpEntry {
      public:

        dhcpEntry(const dhcpEntry& fsm) = default;
        dhcpEntry(dhcpEntry&& fsm) = default;

        dhcpEntry()
        {
          m_fsm = nullptr;
        }

        ~dhcpEntry()
        {
          //delete m_fsm;
          //m_fsm = nullptr;
        }


        dhcpEntry(uint32_t clientIP, uint32_t routerIP, uint32_t dnsIP,
                  uint32_t lease, uint32_t mtu, uint32_t serverID, std::string domainName)
        {
          m_fsm = new FSM;
          std::swap(m_clientIP, clientIP);
          std::swap(m_routerIP, routerIP);
          std::swap(m_dnsIP, dnsIP);
          std::swap(m_lease, lease);
          std::swap(m_mtu, mtu);
          std::swap(m_serverID, serverID);
          std::swap(m_domainName, domainName);

          /* Initializing the State Machine. */
          setState<OnDiscover>(m_instDiscover);
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

        int32_t rx(const char *in, uint32_t inLen)
        {
          /** Kick the state machine now. */
          std::cout << "3.dhcpEntry::rx is invoked " << std::endl;
          int32_t ret = getState().rx(this, in, inLen);

          /* move FSM into next State now.*/
          switch (ret) {
            case REQUEST:
              std::cout << "OnRequest State is set " << std::endl;
              /* Initializing the State Machine. */
              setState<OnRequest>(m_instRequest);
              break;
          }

          return(0);
        }

      private:
        /* Per DHCP Client State Machine. */
        FSM* m_fsm;

        /* Creating instance of */
        OnDiscover m_instDiscover;
        OnRequest m_instRequest;
        OnRelease m_instRelease;
        OnInform m_instInform;
        OnDecline m_instDecline;
        OnLeaseExpire m_instLeaseExpire;

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
        std::array<char, 6> m_chaddr;
        /* The Domain Name to be assigned to DHCP Client. */
        std::string m_domainName;
        /** DHCP Optional Parameters. */
        element_def_t m_dhcpParam;
    };


    using dhcp_entry_onMAC_t = std::unordered_map<std::string, dhcpEntry>;
    using dhcp_entry_onIP_t = std::unordered_map<uint32_t, dhcpEntry>;

    class server {

      public:
        using upstream_delegate_t = delegate<int32_t (const char *in, uint32_t inLen)>;

        dhcp_entry_onMAC_t m_dhcpUmapOnMAC;
        dhcp_entry_onIP_t m_dhcpUmapOnIP;

        server() = default;
        server(const server& ) = default;
        server(server&& ) = default;

        int32_t rx(const char *in, uint32_t inLen)
        {

          std::cout << "1.server::rx received REQ " <<std::endl;
          dhcp_entry_onMAC_t::const_iterator it;

          const uint8_t *clientMAC = ((dhcp_t *)in)->chaddr;
          uint8_t len = ((dhcp_t *)in)->hlen;

          std::string MAC = std::string((const char *)clientMAC, len);

          it = m_dhcpUmapOnMAC.find(MAC);

          if(it != m_dhcpUmapOnMAC.end()) {

            std::cout << "2.dhcpEntry Instance is found " << std::endl;
            /* DHCP Client Entry is found. */
            dhcpEntry dEnt = it->second;
            /* Feed to FSM now. */
            dEnt.rx(in, inLen);

          } else {

            std::cout << "2.dhcpEntry instantiated " << std::endl;
            /* New DHCP Client Request, create an entry for it. */
            dhcpEntry dEnt(123, m_routerIP, m_dnsIP, m_lease, m_mtu, m_serverID, m_domainName);
            bool ret = m_dhcpUmapOnMAC.insert(std::pair<std::string, dhcpEntry>(MAC, dEnt)).second;

            if(!ret) {
              std::cout << "Insertion of dhcpEntry failed " << std::endl;
            }

            dEnt.rx(in, inLen);
          }

          return(0);
        }

      private:

        upstream_delegate_t upstream;
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

  };

};










#endif /*__PROTOCOL_H__*/
