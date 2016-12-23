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

#include <iostream>
#include <folly/Format.h>

#include "nebula/base/testing/testing_util.h"
#include "nebula/storage/conn_pool_manager.h"
#include "nebula/storage/storage_util.h"

struct AppTestEntity {
  uint32_t app_id;
  uint32_t org_id;
  std::string app_name;
  std::string product_name;
  std::string descr;
  int status;
  uint32_t created_at;
  uint32_t updated_at;
  
  std::string ToString() const {
    return folly::sformat("{{app_id: {}, org_id: {}, app_name: {}, product_name: {}, descr: {}, status: {}, created_at: {}, updated: {}}}",
                          app_id,
                          org_id,
                          app_name,
                          product_name,
                          descr,
                          status,
                          created_at,
                          updated_at);
  }
};

//////////////////////////////////////////////////////////////////////////////////////////
struct LoadAppEntity  : public QueryWithResult {
  LoadAppEntity(uint32_t _app_id,
                AppTestEntity& _app)
  : app_id(_app_id),
    app_entity(_app) {}
  
  bool SerializeToQuery(std::string& query_string) const override;
  int ParseFrom(db::QueryAnswer& answ) override;
  
  uint32_t app_id;
  AppTestEntity& app_entity;
};

bool LoadAppEntity::SerializeToQuery(std::string& query_string) const {
  folly::format(&query_string, "SELECT app_id,org_id,app_name,product_name,descr,status,created_at,updated_at FROM apps WHERE app_id={}",
                app_id);
  return !query_string.empty();
}

int LoadAppEntity::ParseFrom(db::QueryAnswer& answ) {
  // LOG(INFO) << "ParseFrom";
  int result = CONTINUE;
  do {
    DB_GET_RETURN_COLUMN(0, app_entity.app_id);
    DB_GET_RETURN_COLUMN(1, app_entity.org_id);
    DB_GET_COLUMN(2, app_entity.app_name);
    DB_GET_COLUMN(3, app_entity.product_name);
    DB_GET_COLUMN(4, app_entity.descr);
    DB_GET_RETURN_COLUMN(5, app_entity.status);
    DB_GET_RETURN_COLUMN(6, app_entity.created_at);
    DB_GET_RETURN_COLUMN(7, app_entity.updated_at);
  } while (0);
  return result;
}

void StorageTest() {
  uint32_t app_id = 1;
  AppTestEntity app_entity;
  LoadAppEntity load_app_entity(app_id, app_entity);
  auto rv = SqlQuery("nebula-platform", load_app_entity);
  std::cout << "rv: " << rv << std::endl;
  std::cout << app_entity.ToString() << std::endl;
}

#include "nebula/storage/redis/redis_conn.h"

void RedisConnTest() {
  RedisAddrInfo addr;
  RedisConn redis_conn;
  redis_conn.Open(addr);
  
  LOG(INFO) << "test_incr_000: " << redis_conn.incr("test_incr_000");
  LOG(INFO) << "test_incr_000: " << redis_conn.incr("test_incr_000", 20);
  LOG(INFO) << "test_incr_000: " << redis_conn.decr("test_incr_000", 20);
  LOG(INFO) << "test_incr_000: " << redis_conn.decr("test_incr_000");
  
  LOG(INFO) << "test_incr_000: " << redis_conn.hincr("htest_001", "fld");
  LOG(INFO) << "test_incr_000: " << redis_conn.hincr("htest_001", "fld", 200);
  LOG(INFO) << "test_incr_000: " << redis_conn.hdecr("htest_001", "fld");
  LOG(INFO) << "test_incr_000: " << redis_conn.hdecr("htest_001", "fld", 200);

}

// static nebula::TestingFuncManager g_testing_storage(StorageTest);
static nebula::TestingFuncManager g_testing_redis(RedisConnTest);


int main(int argc, char* argv[]) {
  
  // SelfRegisterFactoryManagerTest();
  
  nebula::TestingFuncManager::Testing();
}
