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

#include "nebula/storage/storage_util.h"

#include <string>
#include <map>

#include "nebula/storage/conn_pool_manager.h"

// std::vector<CdbConnPoolManager*> g_db_conn_pools;
// db::CdbConnPoolManager* g_db_conn_pool = nullptr;
// db::CdbConnPoolManager* g_db_conn_pool = nullptr;

std::shared_ptr<db::CdbConnPoolManager> Make(const std::string db_name) {
  
  db::DBAddrInfo db_addr;
  db_addr.host = "localhost";
  db_addr.port = 3307;
  db_addr.user = "root";
  db_addr.passwd = "";
  db_addr.db_name = db_name;
  db_addr.db_type = "mysql";
  
  auto db_conn_pool = std::make_shared<db::CdbConnPoolManager>();
  db_conn_pool->Initialize(db_addr);
  return db_conn_pool;
}

db::CdbConnPoolManager* GetDBConnPool(const std::string& db_name) {
  static std::map<std::string, std::shared_ptr<db::CdbConnPoolManager>> g_db_conn_pools;
  
  auto it = g_db_conn_pools.find(db_name);
  if (it==g_db_conn_pools.end()) {
    auto db_conn_pool = Make(db_name);
    g_db_conn_pools[db_name] = db_conn_pool;
    return db_conn_pool.get();
  } else {
    return it->second.get();
  }
}

int SqlQuery(const std::string& db_name, QueryWithResult& q) {
  int rv = 0;
  
  std::string query_string;
  q.SerializeToQuery(query_string);
  DCHECK(!query_string.empty());
  // TODO(@benqi): 这里可能会出错返回
  auto db_conn_pool = GetDBConnPool(db_name);
  db::ScopedPtr_DatabaseConnection db_conn(db_conn_pool);
  // -1
  
  std::unique_ptr<db::QueryAnswer> answ(db_conn->Query(query_string));
  if (answ.get() != NULL) {
    // bool is_empty;
    rv = static_cast<int>(answ->GetRowCount());
    // LOG(INFO) << "rv = " << rv;
    if(rv > 0) {
      while (answ->FetchRow()) {
        auto r = q.ParseFrom(*answ);
        if (r == BaseQueryResult::BREAK) {
          // 只处理一个
          rv = 1;
          break;
        } else if (r == BaseQueryResult::ERROR) {
          rv = -3;
          break;
        } else if (r == BaseQueryResult::CONTINUE) {
          // 继续
        }
      }
    }
  } else {
    rv = -2;
  }

  return rv;
}

int64_t SqlExecuteInsertID(const std::string& db_name, const BaseSqlQuery& query) {
  std::string query_string;
  query.SerializeToQuery(query_string);
  DCHECK(!query_string.empty());

  // TODO(@benqi): 这里可能会出错返回，出错返回-1234567
  auto db_conn_pool = GetDBConnPool(db_name);
  db::ScopedPtr_DatabaseConnection db_conn(db_conn_pool);
  return db_conn->ExecuteInsertID(query_string);
}

int SqlExecute(const std::string& db_name, const BaseSqlQuery& query) {
  std::string query_string;
  query.SerializeToQuery(query_string);
  DCHECK(!query_string.empty());

  // TODO(@benqi): 这里可能会出错返回，出错返回-1234567
  auto db_conn_pool = GetDBConnPool(db_name);

  db::ScopedPtr_DatabaseConnection db_conn(db_conn_pool);
  return db_conn->Execute(query_string);
}
