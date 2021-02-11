#ifndef __json_h__
#define __json_h__

#include "ace/Basic_Types.h"
#include "ace/SString.h"
#include "ace/Log_Msg.h"

namespace parser {

  class json {
    public:
      enum value_type_t : uint8_t {
        VALUE_TYPE_BOOL_FALSE = 0,
        VALUE_TYPE_BOOL_TRUE = 1,
        VALUE_TYPE_INTEGER = 2,
        VALUE_TYPE_DOUBLE = 3,
        VALUE_TYPE_STRING = 4,
        VALUE_TYPE_OBJECT = 5,
        VALUE_TYPE_ARRAY = 6,
        VALUE_TYPE_TRUE = 7,
        VALUE_TYPE_FALSE = 8,
        VALUE_TYPE_NUM = 9,
        VALUE_TYPE_NULL = 10
      };

      struct JSONMember ;
      struct JSONArray ;
      struct JSONElement ;
      struct JSONValue ;

      struct JSONMembers
      {
        JSONMember *m_member;
        JSONMembers *m_next;
      };

      struct JSONObject
      {
        JSONMembers *m_members;
      };

      struct JSONMember
      {
        JSONValue *m_key;
        JSONValue *m_value;
      };

      struct JSONArray
      {
        ~JSONArray()
        {
          m_elements = nullptr;
        }

        JSONArray()
        {
        }

        JSONElement *m_elements;
      };

      struct JSONElement
      {
        JSONValue *m_value;
        JSONElement *m_next;
      };

      struct JSONValue
      {
        JSONValue()
        {
          m_ivalue = 0;
        }

        ~JSONValue()
        {
        }

        value_type_t m_type;
        union {
          int        m_ivalue;
          double     m_dvalue;
          char       *m_svalue;
          char       *m_nvalue;
          JSONObject *m_ovalue;
          JSONArray  *m_avalue;
        };
      };

      ~json();
      JSONValue *json_new_value(void);
      JSONValue *json_new_value_int(int i);
      JSONValue *json_new_value_double(double d);
      JSONValue *json_new_value_str(char *str);
      JSONValue *json_new_value(char *str, int size);
      JSONValue *json_new_value_object(JSONObject *object);
      JSONValue *json_new_value_array(JSONArray *array);
      JSONValue *json_new_value_bool(bool tOrF);
      JSONValue *json_new_value(std::nullptr_t nullp);

      JSONElement *json_new_element(JSONValue *value);
      JSONArray *json_new_array(JSONElement *element);
      JSONMember *json_new_member(JSONValue *key, JSONValue *value);

      JSONObject *json_new_object(JSONMembers *members);
      JSONMembers *json_add_member_in_members(JSONMembers *members, JSONMember *member);
      JSONMembers *json_new_members(JSONMember *member);

      JSONObject *json_new_object(void);
      JSONArray *json_new_array(void);

      void json_free(JSONValue *value);
      void json_free(JSONElement *element);
      void json_free(JSONArray *array);
      void json_free(JSONMember *member);
      void json_free(JSONObject *object);
      void json_free(JSONMembers *members);

      JSONElement *json_value_add_element(JSONElement *element, JSONValue *value);
      JSONMember *json_value_add_member(JSONMember *member, JSONValue *key, JSONValue *value);

      JSONValue &json_value_at_index(JSONValue &value, int index);
      JSONValue &json_value_at_key(JSONValue &value, const char *key);

      /*Array operator overloading.*/
      JSONValue &operator [] (int index);
      JSONValue &operator [] (const char *key);
      JSONValue &at(const char *key);
      JSONValue &at(int index);

      JSONValue &value(void);
      void value(JSONValue *value);

      static json *instance(void);
      static void destroy(void);
      static json *get_instance(void);
      int start(const ACE_TCHAR *fname);
      int stop(void);

      void display(JSONValue &value);
      void display(JSONObject *object);
      void display(JSONArray *array);
      void display(JSONElement *element);
      void display(JSONMembers *members);

      json(JSONValue *);

    private:
      static json *m_instance;
      static ACE_UINT32 m_ref;
      JSONValue *m_value;
      json();

  };
}

#endif /*__json_h__*/
