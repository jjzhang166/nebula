#  Copyright (c) 2016, https://github.com/nebula-im/nebula
#  All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

#include_directories(/usr/local/include/mysql /usr/local/include)

set (SRC_LIST
 ./base_database.h
 ./conn_pool_manager.cc
 ./conn_pool_manager.h
 ./database_dll.h
 ./database_serializable.h
 ./database_util.cc
 ./database_util.h
 ./mysql/mysql_database.cc
 ./mysql/mysql_database.h
 ./mysql/mysql_dll.cc
 ./db_base_object.cc
 ./db_base_object.h
 ./storage_util.cc
 ./storage_util.h

 ./redis/redis_conn.cc
 ./redis/redis_conn.h
 ./redis/redis_pool.cc
 ./redis/redis_pool.h
)

#link_directories(
#/usr/local/lib    
#)


add_library (storage ${SRC_LIST})
target_link_libraries (storage -lmysqlclient -lhiredis)

#add_subdirectory(test)

