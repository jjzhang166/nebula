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

#ifndef NEBULA_STORAGE_BASE_DATABASE_H_
#define NEBULA_STORAGE_BASE_DATABASE_H_

#include <folly/Range.h>

#include "nebula/base/stl_util.h"

namespace db {
  
class QueryAnswer {
public:
  QueryAnswer()
  : column_count_(0),
    row_count_(0) {
  }
  
  virtual ~QueryAnswer() {}
  
  //------------------------------------------------------------------------
  virtual bool FetchRow() = 0;
  virtual void PrintDebugFieldNames() const = 0;
  
  virtual const char* GetColumnName(uint32_t clm) const = 0;

  std::string GetColumnValue(uint32_t clm) const {
    std::string ret;
    GetColumn(clm, &ret);
    return ret;
  }

  std::string GetColumnValue(const folly::StringPiece& clm) const {
    std::string ret;
    GetColumn(clm, &ret);
    return ret;
  }

  //------------------------------------------------------------------------
  virtual bool GetColumn(uint32_t clm, std::string* val) const = 0;
  virtual uint32_t GetColumnLength(uint32_t clm) const = 0;
  virtual bool ColumnIsNull(uint32_t clm) const = 0;
  
  virtual bool GetColumn(uint32_t clm, uint64_t* val) const = 0;
  virtual bool GetColumn(uint32_t clm, int64_t* val) const = 0;
  virtual bool GetColumn(uint32_t clm, uint32_t* val) const = 0;
  virtual bool GetColumn(uint32_t clm, int* val) const = 0;
  virtual bool GetColumn(uint32_t clm, float* val) const = 0;
  virtual bool GetColumn(uint32_t clm, bool* val) const = 0;
  
  //------------------------------------------------------------------------
  virtual bool GetColumn(const folly::StringPiece& clm, std::string* val) const {
    uint32_t index = GetIndexByFieldName(clm);
    if (index < column_count_) {
      return GetColumn(index, val);
    } else {
      return false;
    }
  }
  virtual uint32_t GetColumnLength(const folly::StringPiece& clm) const {
    uint32_t index = GetIndexByFieldName(clm);
    if (index < column_count_) {
      return GetColumnLength(index);
    } else {
      return 0;
    }
  }
  
  virtual bool ColumnIsNull(const folly::StringPiece& clm) const {
    bool result = false;
    uint32_t index = GetIndexByFieldName(clm);
    if (index < column_count_) {
      result = ColumnIsNull(index);
    }
    return result;
  }
  
  virtual bool GetColumn(const folly::StringPiece& clm, uint64_t* val) const {
    bool result = false;
    uint32_t index = GetIndexByFieldName(clm);
    if (index < column_count_) {
      result = GetColumn(index, val);
    }
    return result;
  }
  
  virtual bool GetColumn(const folly::StringPiece& clm, int64_t* val) const {
    bool result = false;
    uint32_t index = GetIndexByFieldName(clm);
    if (index < column_count_) {
      result = GetColumn(index, val);
    }
    return result;
  }
  
  virtual bool GetColumn(const folly::StringPiece& clm, uint32_t* val) const {
    bool result = false;
    uint32_t index = GetIndexByFieldName(clm);
    if (index < column_count_) {
      result = GetColumn(index, val);
    }
    return result;
  }
  
  virtual bool GetColumn(const folly::StringPiece& clm, int* val) const {
    bool result = false;
    uint32_t index = GetIndexByFieldName(clm);
    if (index < column_count_) {
      result = GetColumn(index, val);
    }
    return result;
  }
  
  virtual bool GetColumn(const folly::StringPiece& clm, float* val) const {
    bool result = false;
    uint32_t index = GetIndexByFieldName(clm);
    if (index < column_count_) {
      result = GetColumn(index, val);
    }
    return result;
  }
  
  virtual bool GetColumn(const folly::StringPiece& clm, bool* val) const {
    bool result = false;
    uint32_t index = GetIndexByFieldName(clm);
    if (index < column_count_) {
      result = GetColumn(index, val);
    }
    return result;
  }
  
  //------------------------------------------------------------------------
  inline uint32_t GetColumnCount() const { return column_count_; }
  inline uint64_t GetRowCount() const { return row_count_; }
  
protected:
  virtual uint32_t GetIndexByFieldName(const folly::StringPiece& name) const {
    return column_count_;
  }
  
  uint32_t column_count_;
  uint64_t row_count_;
};
  
//------------------------------------------------------------------------
  
struct DBAddrInfo {
  DBAddrInfo()
    : port(0),
      min_conn_count(2),
      max_conn_count(5) {
  }
  
  // bool ParseFromConnString(const base::StringPiece& conn_string);
  // bool ParseFromConfigFile(const base::ConfigFile& config_file, const char* section_name);
  
  // void PrintDebugString();
  
  void Destroy() {
    port = 0;
    min_conn_count = 2;
    max_conn_count = 5;
    
    STLClearObject(&host);
    STLClearObject(&user);
    STLClearObject(&passwd);
    STLClearObject(&db_name);
    STLClearObject(&db_type);
  }
  
  std::string host;
  int port;
  std::string user;
  std::string passwd;
  std::string db_name;
  std::string db_type;
  int min_conn_count;
  int max_conn_count;
};

// 数据库操作有错误，打印在日志里
class BaseDatabase {
public:
//  enum DBError {
//    kDBErrorUnknown = 0,
//    kDBErrorDupEntry,
//    kDBErrorNotInited,
//    kDBErrorQueryIsNull,
//  };

  //static const base::StringPiece kEmptyStringPiece;
  BaseDatabase() :
    auto_commit_(true) {
  }
  
  virtual ~BaseDatabase() {
    CloseDb();
  }
  
  virtual folly::StringPiece GetDatabaseName() const = 0;
  
  //virtual bool Open(const base::StringPiece& conn_string);
  virtual bool Open(const DBAddrInfo& db_addr, bool auto_commit = true) {
    auto_commit_ = auto_commit;
    db_addr_ = db_addr;
    return BuildConnection();
  }
  
  virtual void CloseDb(){}
  
  // 查询语句
  // 返回值如果为NULL,有两种情况:
  //  1. 无记录
  //  2. 执行查询时发生了错误
  // 如果为NULL，我们可以调用GetLastError()来检查是否是无记录还是发生了其它错误
  virtual QueryAnswer* Query(const folly::StringPiece& q_str) = 0;
  
  // 插入
  // 返回INSERT 操作产生的 ID
  // 如果返回值为0,出错
  virtual uint64_t ExecuteInsertID(const folly::StringPiece& q_str) = 0;
  
  // 插入，更新和删除
  // 返回值为>=0，成功，受影响的行数
  // <0 出错
  virtual int Execute(const folly::StringPiece& q_str) = 0;
  
  virtual bool BeginTransaction() { return false; }
  virtual bool CommitTransaction() { return false; }
  virtual bool RollbackTransaction() { return false; }
  
  virtual bool AutoCommintOn() { return false; }
  virtual bool AutoCommintOff() { return false; }
  
  virtual int GetLastError() = 0;
  
  // 数据库自增ID
  virtual uint64_t GetNextID(const char* table_name, const char* field_name=NULL) = 0;
  
protected:
  //virtual bool CheckConnection() = 0;
  virtual bool BuildConnection() = 0;
  virtual bool Ping() { return true; }
  
  DBAddrInfo db_addr_;
  bool auto_commit_;
};
  
}

#define DB_GET_RETURN_COLUMN(in, out) \
	if (!answ.GetColumn(in, &(out))) { \
		LOG(ERROR) << "Parse column " << #in << " data error."; \
    result = ERROR; \
		break; \
	}

#define DB_GET_COLUMN(in, out) \
	if (!answ.ColumnIsNull(in)) { \
		out = answ.GetColumnValue(in); \
	}

#endif /* _SQLDB_H_ */

