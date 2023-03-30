
#include "DBConnectManager.h"
#include "fmt/format.h"
#include "Configs.h"
#include "conf.h"
#include <exception>
#include <stdexcept>
#include <assert.h>
#include <cassert>
#include <memory>

using std::exception;
using std::runtime_error;
using std::to_string;
using std::make_shared;
using std::string;

namespace SMDB
{

 static shared_ptr<DBConnectManager> _inst = nullptr;

 shared_ptr<SMDB::DBConnectManager> DBConnectManager::getInst()
 {
     if (_inst == nullptr)
     {
         auto dbconf = SMCONF::getDBConfig()->_tcpClient;
         _inst = shared_ptr<DBConnectManager>(new DBConnectManager(dbconf._ip, dbconf._port));
         auto nngconf = SMCONF::getDBConfig()->_nngClient;
     }
     return _inst;
 }

SMDB::DBConnectManager& DBConnectManager::getInst2()
 {
     if (_inst == nullptr)
     {
         auto dbconf = SMCONF::getDBConfig()->_tcpClient;
         _inst = shared_ptr<DBConnectManager>(new DBConnectManager(dbconf._ip, dbconf._port));
     }
     return *_inst;
 }

DBConnectManager::DBConnectManager(string ip, uint16_t port)
    : _ip(ip)
    , _port(port)
  {
  }
}

