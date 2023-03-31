#pragma once
#include "databaseExport.h"
#include "date/tz.h"
#include <memory>
#include <string_view>
#include <functional>
using std::shared_ptr;
using std::string_view;
using std::string;

typedef std::function<bool(string_view token)> BeforeQueryFuncType;
typedef std::function<void()> AfterQueryFuncType;
namespace SMDB
{
    DATABASE_EXPORT bool beforeQuery(string_view token);
    DATABASE_EXPORT void  afterQuery();

    DATABASE_EXPORT bool  setDefaultTimeZone(string tz);
    DATABASE_EXPORT const string& getDefaultStrTimeZone();
    DATABASE_EXPORT const date::time_zone* getDefaultTimeZone();

    DATABASE_EXPORT void  addBeforeQuery(shared_ptr<BeforeQueryFuncType> func);
    DATABASE_EXPORT void  addAfterQuery(shared_ptr < AfterQueryFuncType> func);
    DATABASE_EXPORT void  init(bool dbnode=false);
}
