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

set (SRC_LIST
  basictypes.h
  string_builder.h
  initializer.cc
  initializer.h
  self_register_factory_manager.h
  func_factory_manager.h
  exception.h
  base_daemon.cc
  base_daemon.h
  configurable.h
  configuration.cc
  configuration.h
  config_util.cc
  config_util.h
  plugin.h
  timer_manager.h
  cluster_node_manager.cc
  cluster_node_manager.h
  id_util.cc
  id_util.h
  config_manager.cc
  config_manager.h
  config/config_preprocessor.cc
  config/config_preprocessor.h
  config/import_resolver_if.h
  config/rendezvous_hash.cc
  config/rendezvous_hash.h
  config/file_observer.cc
  config/file_observer.h
  config/file_data_provider.cc
  config/file_data_provider.h
  gperftools_profiler.cc
  gperftools_profiler.h
  process_util.cc
  process_util.h
  linked_hash_map.h
  linked_map.h
  iterator.h
  map_util.h
  time_util.cc
  time_util.h
  dyn_lib.cc
  dyn_lib.h
  dyn_lib_manager.cc
  dyn_lib_manager.h
  stl_util.h
  io_buf_util.cc
  io_buf_util.h
  #readline_gets.cc
  #readline_gets.h

  reflection_util.h
  traits.h
  json/json.cc
  json/json.h
  json/dynamic_converter.h

  snowflake4cxx/id.cc
  snowflake4cxx/id.h

  logger/glog_util.cc
  logger/glog_util.h

  crypto_util/md5.cc
  crypto_util/md5.h
  crypto_util/base64.cc
  crypto_util/base64.h
  crypto_util/modp_b64/modp_b64.cc
  crypto_util/modp_b64/modp_b64.h
  crypto_util/modp_b64/modp_b64_data.h
  crypto_util/big_number.cc
  crypto_util/big_number.h
  crypto_util/sha1.cc
  crypto_util/sha1.h
  crypto_util/dh_util.cc
  crypto_util/dh_util.h
  crypto_util/crypto_util.cc
  crypto_util/crypto_util.h

  testing/testing_util.cc
  testing/testing_util.h
)

add_library(nebula-base STATIC ${SRC_LIST})
target_link_libraries(nebula-base proxygenhttpserver proxygenlib wangle folly boost_context-mt   boost_filesystem-mt boost_system-mt double-conversion glog gflags event ssl crypto pthread z inotify)

add_subdirectory(test)
