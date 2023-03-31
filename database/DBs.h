
#pragma once

/**
 * @file DBs.h classes for deal connection from all other app instance for request databases.
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-01-04
 * 
 * @copyright Copyright (c) 2022
 * 
 */


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
