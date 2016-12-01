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

#include "nebula/storage/db_base_object.h"

bool DBBaseObject::Load(DBResource& data) {
	op_type_ = DB_LOAD;
	return true;
}

bool DBBaseObject::AddNew(DBResource& data) {
	op_type_ = DB_ADDNEW;
	return true;
}

bool DBBaseObject::Delete(DBResource& data) {
	op_type_ = DB_DELETE;
	return true;
}

bool DBBaseObject::Save(DBResource& data) {
	op_type_ = DB_DELETE;
	return true;
}
