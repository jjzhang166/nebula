/*
 *  Copyright (c) 2016, https://github.com/zhatalk
 *  All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef DB_CONN_POOL_MANAGER_H_
#define DB_CONN_POOL_MANAGER_H_

//#define USE_DATABASE_DLL

#include "nebula/base/basictypes.h"

#include <string>
#include <vector>
#include <mutex>

#include <glog/logging.h>

#ifdef USE_DATABASE_DLL
#include "nebula/base/dyn_lib.h"
#endif

#include "nebula/storage/base_database.h"


namespace db {

class CdbConnPoolManager {
public:
  CdbConnPoolManager() :
    conned_pointer_(0),
#ifdef USE_DATABASE_DLL
    dyn_lib_(NULL),
#endif
    auto_commit_(true) {
  }
  
  ~CdbConnPoolManager();
  
  // 返回连接mysql数量
	//size_t Initialize(const std::string& conn_string, int min_conn_count=2, int max_conn_count=5);
	size_t Initialize(const DBAddrInfo& db_addr, bool auto_commit = true);
	// 小心使用，必须所有的连接回收以后才能执行
  void Shutdown();
  
	BaseDatabase* GetFreeConnection();
  void SetFreeConnection(BaseDatabase* conn);
  
  const std::string& GetDbType() const {
    return db_addr_.db_type;
  }
  
private:
  friend class ScopedPtr_DatabaseConnection;
  struct DatabaseConnection {
    
    enum DBConnState {
      kDBConnState_Null = 0,    // 因为未知原因导致db未创建
      kDBConnState_Created = 1,  // 已创建但未连接到数据库
      kDBConnState_Idle = 2,    // 已连接空闲
      kDBConnState_Busy = 3,    // 已连接正在提供服务
    };
    
    DatabaseConnection() :
      state(kDBConnState_Null),
      conn(NULL) {
    }
    
    // 是否空闲
    DBConnState state;
    // is_idle;
    BaseDatabase* conn;
  };
  
  //std::string conn_string_;
  DBAddrInfo db_addr_;

  // 同步
  std::mutex mutex_;
  std::vector<DatabaseConnection> conn_pool_;
  
  // 当前已经创建的连接数
  int conned_pointer_;
  
  // std::string db_type_;
  
#ifdef USE_DATABASE_DLL
  base::DynLib* dyn_lib_;
#endif

  bool auto_commit_;
};

class ScopedPtr_DatabaseConnection {
public:
  explicit ScopedPtr_DatabaseConnection(CdbConnPoolManager* db) {
    DCHECK(db);
    db_ = db;
    db_conn_ = db_->GetFreeConnection();
  }
  
  ~ScopedPtr_DatabaseConnection() {
    release();
  }
  
  BaseDatabase& operator*() const {
    DCHECK(db_);
    if (db_conn_==NULL) {
      db_conn_ = db_->GetFreeConnection();
    }
    return *(db_conn_);
  }
  
  BaseDatabase* operator->() const {
    DCHECK(db_);
    if (db_conn_==NULL) {
      db_conn_ = db_->GetFreeConnection();
    }
    return db_conn_;
  }
  
  BaseDatabase* get() const {
    DCHECK(db_);
    if (db_conn_==NULL) {
      db_conn_ = db_->GetFreeConnection();
    }
    return db_conn_;
  }
  
  operator bool() const {
    return db_conn_ !=0;
  }

  bool operator! () const {
    return db_conn_ == 0;
  }

  void release() {
    DCHECK(db_);
    if (db_conn_) {
      db_->SetFreeConnection(db_conn_);
      db_conn_ = NULL;
    }
  }
  
private:
  CdbConnPoolManager* db_;
  mutable BaseDatabase* db_conn_;
  
  DISALLOW_COPY_AND_ASSIGN(ScopedPtr_DatabaseConnection);
};

}

#endif /* defined(DB_CONN_POOL_MANAGER_H_) */
