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

#ifndef DB_DATABASE_SERIALIZABLE_H_
#define DB_DATABASE_SERIALIZABLE_H_

#include "nebula/storage/database_util.h"

namespace db {

class QueryParam;
class QueryAnswer;
  
class DatabaseSerializable {
public:
	virtual ~DatabaseSerializable() {}

	virtual bool ParseFromDatabase(QueryAnswer& answ) { return true; }
  virtual bool SerializeToDatabase(QueryParam& param) const { return true; }
};


}

#define DB_GET_RETURN_COLUMN(in, out) \
	result = answ.GetColumn(in, &(out)); \
	if (!result) { \
		LOG(ERROR) << "Parse column " << #in << " data error."; \
		break; \
	}

#define DB_GET_COLUMN(in, out) \
	if (!answ.ColumnIsNull(in)) { \
		out = answ.GetColumn(in); \
	}

#endif
