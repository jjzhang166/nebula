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

#ifndef STORAGE_STORAGE_UTIL_H_
#define STORAGE_STORAGE_UTIL_H_

#include "nebula/storage/database_util.h"
#include "nebula/storage/base_database.h"

struct BaseSqlQuery {
  virtual ~BaseSqlQuery() = default;
  virtual bool SerializeToQuery(std::string& query_string) const { return true; }
};

struct BaseQueryResult {
  enum RCODE {
    CONTINUE = 0,   // 继续
    BREAK = 1,      // 停止
    ERROR =2,       // 转换出错
  };
  virtual ~BaseQueryResult() = default;
  
  // 可能会调用多次
  virtual int ParseFrom(db::QueryAnswer& answ) { return BREAK; }
};

struct QueryWithResult : public BaseSqlQuery, public BaseQueryResult {
  virtual ~QueryWithResult() = default;
};

// <0: -1: 连接出错
//     -2: 数据库语句有问题
//     -3: 反序列化有问题
// =0: 无数据
// >0: 查询处n条记录
int SqlQuery(const std::string& db_name, QueryWithResult& q);

int64_t SqlExecuteInsertID(const std::string& db_name, const BaseSqlQuery& query);
int SqlExecute(const std::string& db_name, const BaseSqlQuery& query);

enum QResultCode {
  CONTINUE = 0,   // 继续
  BREAK = 1,      // 停止
  ERROR =2,       // 转换出错
};

using MakeQueryStringFunc = std::function<void(std::string& query_string)>;
using MakeQueryResultFunc = std::function<int(db::QueryAnswer& answ)>;

int DoStorageQuery(const std::string& db_name, MakeQueryStringFunc make_q, MakeQueryResultFunc make_r);
int64_t DoStorageInsertID(const std::string& db_name, MakeQueryStringFunc make_q);
int DoStorageExecute(const std::string& db_name, MakeQueryStringFunc make_q);

#endif

