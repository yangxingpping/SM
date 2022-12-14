// generated by ./ddl2cpp ./comm_db.sql comm_db comm_db
#ifndef COMM_DB_COMM_DB_H
#define COMM_DB_COMM_DB_H

#include <sqlpp11/table.h>
#include <sqlpp11/data_types.h>
#include <sqlpp11/char_sequence.h>

namespace comm_db
{
  namespace Users_
  {
    struct Id
    {
      struct _alias_t
      {
        static constexpr const char _literal[] =  "id";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
          {
            T id;
            T& operator()() { return id; }
            const T& operator()() const { return id; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::can_be_null>;
    };
    struct Username
    {
      struct _alias_t
      {
        static constexpr const char _literal[] =  "username";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
          {
            T username;
            T& operator()() { return username; }
            const T& operator()() const { return username; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::varchar, sqlpp::tag::can_be_null>;
    };
    struct Nickname
    {
      struct _alias_t
      {
        static constexpr const char _literal[] =  "nickname";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
          {
            T nickname;
            T& operator()() { return nickname; }
            const T& operator()() const { return nickname; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::varchar, sqlpp::tag::can_be_null>;
    };
    struct Avatar
    {
      struct _alias_t
      {
        static constexpr const char _literal[] =  "avatar";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
          {
            T avatar;
            T& operator()() { return avatar; }
            const T& operator()() const { return avatar; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::varchar, sqlpp::tag::can_be_null>;
    };
    struct Passwd
    {
      struct _alias_t
      {
        static constexpr const char _literal[] =  "passwd";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
          {
            T passwd;
            T& operator()() { return passwd; }
            const T& operator()() const { return passwd; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::varchar, sqlpp::tag::can_be_null>;
    };
    struct Salt
    {
      struct _alias_t
      {
        static constexpr const char _literal[] =  "salt";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
          {
            T salt;
            T& operator()() { return salt; }
            const T& operator()() const { return salt; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::varchar, sqlpp::tag::can_be_null>;
    };
  } // namespace Users_

  struct Users: sqlpp::table_t<Users,
               Users_::Id,
               Users_::Username,
               Users_::Nickname,
               Users_::Avatar,
               Users_::Passwd,
               Users_::Salt>
  {
    struct _alias_t
    {
      static constexpr const char _literal[] =  "Users";
      using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
      template<typename T>
      struct _member_t
      {
        T Users;
        T& operator()() { return Users; }
        const T& operator()() const { return Users; }
      };
    };
  };
} // namespace comm_db
#endif
