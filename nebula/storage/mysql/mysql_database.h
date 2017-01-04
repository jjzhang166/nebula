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

#ifndef DB_MYSQL_MYSQL_DATABASE_H_
#define DB_MYSQL_MYSQL_DATABASE_H_

#include "nebula/storage/base_database.h"

#ifdef OS_WIN
#include <winsock2.h>
#endif

#include <mysql/mysql.h>

namespace db {
  
class MySQLDatabase :
  public BaseDatabase {
public:
	MySQLDatabase();
	virtual ~MySQLDatabase();
  
  virtual folly::StringPiece GetDatabaseName() const {
    return "mysql";
  }
    
  //virtual bool Open(const base::StringPiece& conn_string);
  virtual void CloseDb();
    
	// 查询语句
	virtual QueryAnswer* Query(const folly::StringPiece& q_str);
  
	// 插入
	// 返回INSERT 操作产生的 ID
	virtual uint64_t ExecuteInsertID(const folly::StringPiece& q_str);
  
	// 插入和更新
	virtual int Execute(const folly::StringPiece& q_str);

  virtual uint64_t GetNextID(const char* table_name, const char* field_name);
    
	virtual bool BeginTransaction();
	virtual bool CommitTransaction();
	virtual bool RollbackTransaction();
  
  virtual int GetLastError();

protected:
  bool CheckConnection();
  virtual bool BuildConnection();
	virtual bool Ping();
  
private:
  int SafeQuery(const folly::StringPiece& q_str, int* err);
    
  st_mysql* my_;
  st_mysql mysql_;

  //int last_error_;
};

}

#endif /* defined(DB_MYSQL_MYSQL_DATABASE_H_) */
