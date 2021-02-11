#ifndef __config_cc__
#define __config_cc__

#include "protocol.h"
#include "json.h"

mna::dhcp::serverConfig::serverConfig()
{
  parser::json *m_parser = parser::json::instance();
  std::string sName("/home/mnahmed/delegate/schema/vdhcp");
  m_parser->start(sName.c_str());
  parser::json::JSONValue& val = m_parser->value();
  m_parser->display(val);
  /*! Retrieving the respective value */
  parser::json::JSONValue& value = m_parser->json_value_at_key(m_parser->json_value_at_key(val, "profile"), "lease-time");
  m_parser->display(value);
  parser::json::JSONValue& value1 = m_parser->json_value_at_index(m_parser->json_value_at_key(m_parser->json_value_at_key(val, "dhcp"), "exclude-ip"), 1);
  m_parser->display(value1);
}

















#endif /*__config_cc__*/
