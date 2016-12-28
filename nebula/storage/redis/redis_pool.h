/*
 *  Copyright (c) 2016, https://github.com/nebula-im
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

#ifndef REDIS_REDIS_POOL_H_
#define REDIS_REDIS_POOL_H_

#include "nebula/base/basictypes.h"

#include <string>
#include <vector>
#include <mutex>

#include <glog/logging.h>

#include "nebula/storage/redis/redis_conn.h"

// TODO(@benqi): 功能和CdbConnPoolManager类似
//   抽象出通用代码，可支持后续的Mongo连接池等
class RedisConnPoolManager {
public:
  RedisConnPoolManager()
    : conned_pointer_(0) {
  }
  
  ~RedisConnPoolManager() {
    Shutdown();
  }
  
  size_t Initialize(const RedisAddrInfo& addr);
  // 小心使用，必须所有的连接回收以后才能执行
  void Shutdown();
  
  RedisConn* GetFreeConnection();
  void SetFreeConnection(RedisConn* conn);
  
private:
  friend class ScopedPtr_RedisConnection;
  struct RedisConnection {
    
    enum ConnState {
      kConnState_Null = 0,    // 因为未知原因导致db未创建
      kConnState_Created = 1,  // 已创建但未连接到数据库
      kConnState_Idle = 2,    // 已连接空闲
      kConnState_Busy = 3,    // 已连接正在提供服务
    };
    
    RedisConnection() :
      state(kConnState_Null),
      conn(NULL) {
    }
    
    // 是否空闲
    ConnState state;
    // is_idle;
    RedisConn* conn;
  };
  
  //std::string conn_string_;
  RedisAddrInfo addr_;
  
  // 同步
  std::mutex mutex_;
  std::vector<RedisConnection> conn_pool_;
  
  // 当前已经创建的连接数
  int conned_pointer_;
};

class ScopedPtr_RedisConnection {
public:
  explicit ScopedPtr_RedisConnection(RedisConnPoolManager* pool) {
    DCHECK(pool);
    pool_ = pool;
    conn_ = pool_->GetFreeConnection();
  }
  
  ~ScopedPtr_RedisConnection() {
    release();
  }
  
  RedisConn& operator*() const {
    DCHECK(pool_);
    if (conn_==NULL) {
      conn_ = pool_->GetFreeConnection();
    }
    return *(conn_);
  }
  
  RedisConn* operator->() const {
    DCHECK(pool_);
    if (conn_==NULL) {
      conn_ = pool_->GetFreeConnection();
    }
    return conn_;
  }
  
  RedisConn* get() const {
    DCHECK(pool_);
    if (conn_==NULL) {
      conn_ = pool_->GetFreeConnection();
    }
    return conn_;
  }
  
  operator bool() const {
    return conn_ !=0;
  }
  
  bool operator! () const {
    return conn_ == 0;
  }
  
  void release() {
    DCHECK(pool_);
    if (conn_) {
      pool_->SetFreeConnection(conn_);
      conn_ = NULL;
    }
  }
  
private:
  RedisConnPoolManager* pool_;
  mutable RedisConn* conn_;
  
  DISALLOW_COPY_AND_ASSIGN(ScopedPtr_RedisConnection);
};

RedisConnPoolManager* GetRedisConnPool(const std::string& name);

#endif // REDIS_REDIS_POOL_H_
