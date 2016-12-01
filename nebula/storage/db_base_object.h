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

// Author: wubenqi@gmail.com (wubenqi)
//

#ifndef DB_BASE_DB_OBJ_H_
#define DB_BASE_DB_OBJ_H_

#include <string>
#include "nebula/storage/database_serializable.h"

namespace db {
class CdbConnPoolManager;
}

struct DBResource : public db::DatabaseSerializable {
  virtual ~DBResource() = default;
};

class DBBaseObject {
public:
	enum DBOP_TYPE {
		DB_NONE,
		DB_LOAD,
		DB_SAVE,
		DB_ADDNEW,
		DB_DELETE,
	};

  DBBaseObject(void) = default;
  virtual ~DBBaseObject(void) = default;

	// int32 GetErrorCode();
	// std::string	 GetErrorMessage();
  
	inline uint32_t GetFieldCount() const { return field_count; }
	inline uint32_t GetRowCount() const { return row_count; }

	// 子类需要重载才能实现功能
  virtual bool Load(DBResource& data);
  virtual bool AddNew(DBResource& data);
  virtual uint64_t AddNewInsertID(DBResource& data);
  virtual bool Delete(DBResource& data);
  virtual bool Save(DBResource& data);

protected:
  DBOP_TYPE		op_type_ {DB_NONE};
  uint32_t field_count {0};
  uint32_t row_count {0};
  
  db::CdbConnPoolManager* db_conn_pool_ {nullptr};
};

#endif
