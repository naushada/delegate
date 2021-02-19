#ifndef __json_cc__
#define __json_cc__

#include <iostream>
#include "json.h"
#include "jsonparser.hpp"
#include "jsonlexer.hh"

#include "ace/Log_Msg.h"

parser::json::json(JSONValue *value)
{
  m_value = value;
}

parser::json::json()
{
  m_value = nullptr;
}

parser::json::~json()
{
  stop();
  /*reclaim the heap memory now.*/
  m_value = nullptr;
}

parser::json::JSONValue* parser::json::value(void)
{
  return(m_value);
}

void parser::json::value(parser::json::JSONValue *value)
{
  m_value = value;
}

int parser::json::stop(void)
{
  json_free(m_value);
  m_value = nullptr;
  return(0);
}

int parser::json::parse(const ACE_TCHAR *fname)
{
  int ret = -1;

  FILE *in = nullptr;

  yyscan_t scanner;

  if(fname)
  {
    in = ACE_OS::fopen(fname, "r");
    if(!in)
    {
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l opening of file %s failed\n"), fname));
      return(0);
    }
  }
  else
    in = stdin;

  yylex_init_extra(this, &scanner);
  yyset_in(in, scanner);

  if((ret = yyparse(scanner, *this)))
    delete m_value;

  yylex_destroy(scanner);

  if(in != stdin)
  {
    ACE_OS::fclose(in);
    in = nullptr;
  }

  return(ret);
}

parser::json::JSONValue* parser::json::json_new_value(void)
{
  JSONValue *value = nullptr;

  ACE_NEW_RETURN(value, JSONValue(), nullptr);
  return(value);
}

parser::json::JSONValue* parser::json::json_new_value_int(int i)
{
  JSONValue *value = json_new_value();

  if(value)
  {
    value->m_type = parser::json::VALUE_TYPE_INTEGER;
    value->m_ivalue = i;
  }

  return(value);
}

parser::json::JSONValue* parser::json::json_new_value_double(double d)
{
  JSONValue *value = json_new_value();

  if(value)
  {
    value->m_type = parser::json::VALUE_TYPE_DOUBLE;
    value->m_ivalue = d;
  }

  return(value);
}

parser::json::JSONValue* parser::json::json_new_value_str(char *str)
{

  JSONValue *value = json_new_value();

  if(value)
  {
    value->m_type = parser::json::VALUE_TYPE_STRING;
    value->m_svalue = ACE_OS::strdup(str);
  }

  return(value);
}

parser::json::JSONValue* parser::json::json_new_value(char *str, int size)
{
  return(nullptr);
}

parser::json::JSONValue* parser::json::json_new_value_object(JSONObject *object)
{
  if(nullptr == object)
    return(nullptr);

  JSONValue *value = json_new_value();
  value->m_type = parser::json::VALUE_TYPE_OBJECT;
  value->m_ovalue = object;

  return(value);
}

parser::json::JSONValue* parser::json::json_new_value_array(JSONArray *array)
{
  if(nullptr == array)
    return(nullptr);

  JSONValue *value = json_new_value();
  if(nullptr == value)
    return(nullptr);

  value->m_type = parser::json::VALUE_TYPE_ARRAY;
  value->m_avalue = array;

  return(value);
}

parser::json::JSONValue* parser::json::json_new_value_bool(bool tOF)
{
  JSONValue *value = json_new_value();

  if(true == tOF)
  {
    value->m_type = parser::json::VALUE_TYPE_BOOL_TRUE;
  }
  else
  {
    value->m_type = parser::json::VALUE_TYPE_BOOL_FALSE;
  }

  return(value);
}

parser::json::JSONValue* parser::json::json_new_value(std::nullptr_t nullp)
{
  JSONValue *value = json_new_value();

  if(nullptr == value)
    return(nullptr);

  value->m_type = parser::json::VALUE_TYPE_NULL;
  value->m_nvalue = ACE_OS::strdup("null");

  return(value);
}

parser::json::JSONElement* parser::json::json_new_element(JSONValue *value)
{
  JSONElement *element = nullptr;

  ACE_NEW_RETURN(element, JSONElement(), 0);

  element->m_value = value;
  element->m_next = nullptr;

  return(element);
}

parser::json::JSONArray* parser::json::json_new_array(JSONElement *element)
{
  JSONArray *array = nullptr;

  ACE_NEW_RETURN(array, JSONArray(), 0);

  array->m_elements = element;

  return(array);
}

parser::json::JSONMember* parser::json::json_new_member(JSONValue *key, JSONValue *value)
{
  JSONMember *member = nullptr;

  ACE_NEW_RETURN(member, JSONMember(), 0);

  member->m_key = key;
  member->m_value = value;
  return(member);
}

parser::json::JSONObject* parser::json::json_new_object(JSONMembers *members)
{
  JSONObject *object = nullptr;

  if(nullptr == members)
    return(nullptr);

  ACE_NEW_NORETURN(object, JSONObject());

  object->m_members = members;

  return(object);
}

parser::json::JSONMembers* parser::json::json_add_member_in_members(JSONMembers *members, JSONMember *member)
{
  if(nullptr == members)
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l members is nullptr\n")));
    return(json_new_members(member));
  }

  JSONMembers *m = nullptr;

  for(m = members; m->m_next != nullptr; m = m->m_next)
    ;

  m->m_next = json_new_members(member);

  return(members);
}

parser::json::JSONMembers* parser::json::json_new_members(JSONMember *member)
{
  JSONMembers *members = nullptr;

  ACE_NEW_RETURN(members, JSONMembers(), 0);

  members->m_member = member;
  members->m_next = nullptr;

  return(members);
}

parser::json::JSONObject* parser::json::json_new_object(void)
{
  JSONObject *object = nullptr;

  ACE_NEW_RETURN(object, JSONObject(), 0);
  object->m_members = nullptr;

  return(object);
}

parser::json::JSONArray* parser::json::json_new_array(void)
{
  JSONArray *array = nullptr;

  ACE_NEW_RETURN(array, JSONArray(), 0);

  array->m_elements = nullptr;

  return(array);
}

void parser::json::json_free(JSONValue *value)
{
  if(nullptr == value)
    return;

  switch(value->m_type)
  {
  case parser::json::VALUE_TYPE_STRING:
    /*use free and not delete because memory was allocated by
     * strdup which usages malloc.*/
    ACE_OS::free((void *)value->m_svalue);
    break;

  case parser::json::VALUE_TYPE_NULL:
    /*use free and not delete because memory was allocated
     *by strdup which usages malloc.*/
    ACE_OS::free((void *)value->m_nvalue);
    break;

  case parser::json::VALUE_TYPE_OBJECT:
    json_free(value->m_ovalue);
    break;

  case parser::json::VALUE_TYPE_ARRAY:
    json_free(value->m_avalue);
    break;

  default:
    break;
  }

  delete value;
}

void parser::json::json_free(JSONElement *element)
{
  if(nullptr == element)
    return;

  json_free(element->m_next);
  json_free(element->m_value);
  delete element;
}

void parser::json::json_free(JSONArray *array)
{
  if(nullptr == array)
    return;

  json_free(array->m_elements);
  delete array;
}

void parser::json::json_free(JSONMember *member)
{
  if(nullptr == member)
    return;

  json_free(member->m_key);
  json_free(member->m_value);
  delete member;

}

void parser::json::json_free(JSONMembers *members)
{
  if(nullptr == members)
    return;

  json_free(members->m_next);
  json_free(members->m_member);
  delete members;

}

void parser::json::json_free(JSONObject *object)
{
  if(nullptr == object)
    return;

  json_free(object->m_members);
  delete object;
}

parser::json::JSONElement* parser::json::json_value_add_element(JSONElement *element, JSONValue *value)
{
  if(nullptr == element)
    return(json_new_element(value));

  JSONElement *e = nullptr;
  for(e = element; e->m_next != nullptr; e = e->m_next)
    ;

  e->m_next = json_new_element(value);

  return(element);
}

parser::json::JSONValue* parser::json::operator[](int index)
{
  return(json_value_at_index(*value(), index));
}

parser::json::JSONValue* parser::json::at(int index)
{
  return(json_value_at_index(*value(), index));
}

/*
 * @brief
 * @param
 * @param
 *
 * @return either nullptr or value at given index.
 * */
parser::json::JSONValue* parser::json::json_value_at_index(JSONValue &value, int index)
{
  JSONElement *e = nullptr;

  for(e = value.m_avalue->m_elements; index && (e != nullptr); index--, e = e->m_next)
    ;

  if(e != nullptr)
    return(e->m_value);

  return(nullptr);
}

parser::json::JSONValue& parser::json::operator[](const char *key)
{
  return(json_value_at_key(*value(), key));
}

parser::json::JSONValue& parser::json::at(const ACE_TCHAR *key)
{
  return(json_value_at_key(*value(), key));
}

parser::json::JSONValue& parser::json::json_value_at_key(JSONValue &value, const char *key)
{
  JSONMembers *m = nullptr;

  for(m = value.m_ovalue->m_members; m != nullptr; m = m->m_next)
  {
    if((m->m_member->m_key->m_type == parser::json::VALUE_TYPE_STRING) &&
       (!strcmp(m->m_member->m_key->m_svalue, key)))
    {
      return(*m->m_member->m_value);
    }
  }
}

void parser::json::display(JSONValue &value)
{

  switch(value.m_type)
  {
  case parser::json::VALUE_TYPE_STRING:
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l %s \n"), get_str(value).c_str()));
    break;
  case parser::json::VALUE_TYPE_OBJECT:
    display(value.m_ovalue);
    break;
  case parser::json::VALUE_TYPE_ARRAY:
    display(value.m_avalue);
    break;
  case parser::json::VALUE_TYPE_INTEGER:
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l %d \n"), get_int(value)));
    break;
  default:
    ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Type not supported\n")));

  }
}

void parser::json::display(JSONMembers *members)
{
  if(nullptr == members)
    return;

  if(members->m_member)
  {
    display(*members->m_member->m_key);
    display(*members->m_member->m_value);
    display(members->m_next);
  }

}

void parser::json::display(JSONObject *object)
{
  if(nullptr == object)
    return;

  display(object->m_members);
}

void parser::json::display(JSONElement *element)
{
  if(nullptr == element)
    return;

  display(*element->m_value);
  display(element->m_next);
}

void parser::json::display(JSONArray *array)
{
  if(nullptr == array)
    return;

  display(array->m_elements);
}

int32_t parser::json::get_int(JSONValue& val) const
{
  return(val.m_ivalue);
}

std::string parser::json::get_str(JSONValue& val)
{
  return(std::string(val.m_svalue));
}



#endif /*__json_cc__*/
