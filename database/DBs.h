
#pragma once

namespace SMDB
{
class  DBs
{
public:
    static DBs& getInst();
  public:
    DBs(); 
    void init();
    
};
}
