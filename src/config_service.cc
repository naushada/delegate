#ifndef __config_service_cc__
#define __config_service_cc__

#include "protocol_layer.h"
#include "json.h"

mna::dhcp::serverConfig::serverConfig(std::string sname)
{
  parser::json m_parser;
  m_parser.parse(sname.c_str());
  parser::json::JSONValue& val = *m_parser.value();
  m_parser.display(val);
  /*! Retrieving the respective value */
  parser::json::JSONValue& dhcp = m_parser.json_value_at_key(val, "dhcp");

  /*! dhcp's elements*/
  parser::json::JSONValue& vrf = m_parser.json_value_at_key(val, m_parser.get_str(m_parser.json_value_at_key(dhcp, "virtual-network")).c_str());
  parser::json::JSONValue& profile = m_parser.json_value_at_key(val, m_parser.get_str(m_parser.json_value_at_key(dhcp, "profile-name")).c_str());

  mask(m_parser.get_str(m_parser.json_value_at_key(dhcp, "subnet-mask")));
  ip(m_parser.get_str(m_parser.json_value_at_key(dhcp, "ip")));
  hostName(m_parser.get_str(m_parser.json_value_at_key(dhcp, "host-name")));
  startIP(m_parser.get_str(m_parser.json_value_at_key(dhcp, "start-ip")));
  endIP(m_parser.get_str(m_parser.json_value_at_key(dhcp, "end-ip")));

  int32_t idx = 0;
  parser::json::JSONValue* exIPArr = m_parser.json_value_at_index(m_parser.json_value_at_key(dhcp, "exclude-ip"), idx++);
  for(; exIPArr != nullptr; ++idx) {
    excludeIP(m_parser.get_str(*exIPArr));
    exIPArr = m_parser.json_value_at_index(m_parser.json_value_at_key(dhcp, "exclude-ip"), idx);
  }

  /*! Populating Profile's fields now. */
  mtu(m_parser.get_int(m_parser.json_value_at_key(profile, "mtu")));
  primaryDns(m_parser.get_str(m_parser.json_value_at_key(profile, "primary-dns-ip")));
  secondaryDns(m_parser.get_str(m_parser.json_value_at_key(profile, "secondary-dns-ip")));
  domainName(m_parser.get_str(m_parser.json_value_at_key(profile, "domain-name")));
  lease(m_parser.get_int(m_parser.json_value_at_key(profile, "lease-time")));

  /*! vrf - virtual routing function*/
  type(m_parser.get_str(m_parser.json_value_at_key(vrf, "type")));
  port(m_parser.get_str(m_parser.json_value_at_key(vrf, "port")));
  pmtu(m_parser.get_int(m_parser.json_value_at_key(vrf, "mtu")));
}


mna::ddns::config::config(std::string sname)
{
  parser::json m_parser;
  m_parser.parse(sname.c_str());
  parser::json::JSONValue& val = *m_parser.value();
  m_parser.display(val);
  /*! Retrieving the respective value */
  parser::json::JSONValue& vddns_peer = m_parser.json_value_at_key(val, "vddns-peer");
  parser::json::JSONValue& vddns_instance = m_parser.json_value_at_key(val, "vddns-instance");

  /*! vddns-peer's elements*/
  parser::json::JSONValue& profile = m_parser.json_value_at_key(val, m_parser.get_str(m_parser.json_value_at_key(vddns_peer, "ddns-server")).c_str());

  instance().bindAddress(m_parser.get_str(m_parser.json_value_at_key(vddns_instance, "bind-address")));
  instance().periodicUpdate(m_parser.get_int(m_parser.json_value_at_key(vddns_instance, "periodic-update-after")));

  /*! Populating Profile's fields now. */
  int32_t idx = 0;
  vddnsPeer peerConfig;
  parser::json::JSONValue* peerArr = nullptr;
  for(peerArr = m_parser.json_value_at_index((m_parser.json_value_at_key(profile, "service-provider")), idx++); peerArr != nullptr; ++idx)
  {
    peerConfig.name(m_parser.get_str(m_parser.json_value_at_key(*peerArr, "name")));
    peerConfig.userId(m_parser.get_str(m_parser.json_value_at_key(*peerArr, "user-id")));
    peerConfig.password(m_parser.get_str(m_parser.json_value_at_key(*peerArr, "password")));
    peerConfig.domainName(m_parser.get_str(m_parser.json_value_at_key(*peerArr, "domain-name")));
    peerConfig.peerPort(m_parser.get_int(m_parser.json_value_at_key(*peerArr, "port-number")));

    int32_t subIdx = 0;
    peerConfig.hostName().clear();
    parser::json::JSONValue* hostArr = m_parser.json_value_at_index(m_parser.json_value_at_key(*peerArr, "host-name"), subIdx++);
    for(; hostArr != nullptr; ++subIdx)
    {
      peerConfig.hostName(m_parser.get_str(*hostArr));
      hostArr = m_parser.json_value_at_index(m_parser.json_value_at_key(*peerArr, "host-name"), subIdx);
    }

    peer().push_back(peerConfig);
    peerArr = m_parser.json_value_at_index((m_parser.json_value_at_key(profile, "service-provider")), idx);
  }
}















#endif /*__config_service_cc__*/
