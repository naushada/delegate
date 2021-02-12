#ifndef __config_cc__
#define __config_cc__

#include "protocol.h"
#include "json.h"

mna::dhcp::serverConfig::serverConfig(std::string sname)
{
  parser::json *m_parser = parser::json::instance();
  m_parser->parse(sname.c_str());
  parser::json::JSONValue& val = m_parser->value();
  m_parser->display(val);
  /*! Retrieving the respective value */
  parser::json::JSONValue& dhcp = m_parser->json_value_at_key(val, "dhcp");

  /*! dhcp's elements*/
  parser::json::JSONValue& vrf = m_parser->json_value_at_key(val, m_parser->get_str(m_parser->json_value_at_key(dhcp, "virtual-network")).c_str());
  parser::json::JSONValue& profile = m_parser->json_value_at_key(val, m_parser->get_str(m_parser->json_value_at_key(dhcp, "profile-name")).c_str());

  mask(m_parser->get_str(m_parser->json_value_at_key(dhcp, "subnet-mask")));
  ip(m_parser->get_str(m_parser->json_value_at_key(dhcp, "ip")));
  hostName(m_parser->get_str(m_parser->json_value_at_key(dhcp, "host-name")));
  startIP(m_parser->get_str(m_parser->json_value_at_key(dhcp, "start-ip")));
  endIP(m_parser->get_str(m_parser->json_value_at_key(dhcp, "end-ip")));

  excludeIP(m_parser->get_str(m_parser->json_value_at_index(m_parser->json_value_at_key(dhcp, "exclude-ip"), 0)));
  excludeIP(m_parser->get_str(m_parser->json_value_at_index(m_parser->json_value_at_key(dhcp, "exclude-ip"), 1)));
  excludeIP(m_parser->get_str(m_parser->json_value_at_index(m_parser->json_value_at_key(dhcp, "exclude-ip"), 2)));
  excludeIP(m_parser->get_str(m_parser->json_value_at_index(m_parser->json_value_at_key(dhcp, "exclude-ip"), 3)));
  excludeIP(m_parser->get_str(m_parser->json_value_at_index(m_parser->json_value_at_key(dhcp, "exclude-ip"), 4)));

  /*! Populating Profile's fields now. */
  mtu(m_parser->get_int(m_parser->json_value_at_key(profile, "mtu")));
  primaryDns(m_parser->get_str(m_parser->json_value_at_key(profile, "primary-dns-ip")));
  secondaryDns(m_parser->get_str(m_parser->json_value_at_key(profile, "secondary-dns-ip")));
  domainName(m_parser->get_str(m_parser->json_value_at_key(profile, "domain-name")));
  lease(m_parser->get_int(m_parser->json_value_at_key(profile, "lease-time")));

  /*! vrf - virtual routing function*/
  type(m_parser->get_str(m_parser->json_value_at_key(vrf, "type")));
  port(m_parser->get_str(m_parser->json_value_at_key(vrf, "port")));
  pmtu(m_parser->get_int(m_parser->json_value_at_key(vrf, "mtu")));

  std::cout << "Value of port " << domainName().c_str() << std::endl;
  //m_parser->display(value);
  parser::json::JSONValue& value1 = m_parser->json_value_at_index(m_parser->json_value_at_key(m_parser->json_value_at_key(val, "dhcp"), "exclude-ip"), 1);
  m_parser->display(value1);
}

















#endif /*__config_cc__*/
