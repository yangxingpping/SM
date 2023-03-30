// generated by ./ddl2cpp ./dbTest.sql dbTest dbTest
#ifndef DBTEST_DBTEST_H
#define DBTEST_DBTEST_H

#include <sqlpp11/table.h>
#include <sqlpp11/data_types.h>
#include <sqlpp11/char_sequence.h>

namespace dbTest
{
  namespace Forsumgroup_
  {
    struct Name
    {
      struct _alias_t
      {
        static constexpr const char _literal[] =  "name";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
          {
            T name;
            T& operator()() { return name; }
            const T& operator()() const { return name; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::varchar, sqlpp::tag::can_be_null>;
    };
    struct Ed
    {
      struct _alias_t
      {
        static constexpr const char _literal[] =  "ed";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
          {
            T ed;
            T& operator()() { return ed; }
            const T& operator()() const { return ed; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::time_point, sqlpp::tag::can_be_null>;
    };
    struct Price
    {
      struct _alias_t
      {
        static constexpr const char _literal[] =  "price";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
          {
            T price;
            T& operator()() { return price; }
            const T& operator()() const { return price; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::can_be_null>;
    };
  } // namespace Forsumgroup_

  struct Forsumgroup: sqlpp::table_t<Forsumgroup,
               Forsumgroup_::Name,
               Forsumgroup_::Ed,
               Forsumgroup_::Price>
  {
    struct _alias_t
    {
      static constexpr const char _literal[] =  "forsumgroup";
      using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
      template<typename T>
      struct _member_t
      {
        T forsumgroup;
        T& operator()() { return forsumgroup; }
        const T& operator()() const { return forsumgroup; }
      };
    };
  };
} // namespace dbTest
#endif
