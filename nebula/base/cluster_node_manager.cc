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

#include "nebula/base/cluster_node_manager.h"

// #include <folly/Singleton.h>
#include <folly/Conv.h>

#include "nebula/base/process_util.h"

namespace nebula {
  
// 运行时节点信息
struct RuntimeNodeID {
  RuntimeNodeID() {
    host_name = GetHostName();
    process_id = GetProcessID();
    program_name = GetProcessName(process_id);
    g_str_runtime_node_id = program_name + "@" + host_name + "/" + folly::to<std::string>(process_id);
  }
  
  std::string host_name;      // 主机名
  std::string program_name;   // 进程名
  uint32_t process_id {0};        // 进程ID
  
  static std::string g_str_runtime_node_id;
};

std::string RuntimeNodeID::g_str_runtime_node_id;

// TODO(@benqi)
// 实现比较土
// 估计有更好的实现方式
const std::string& GetRuntimeNodeID() {
  static RuntimeNodeID g_runtime_node_id;
  return RuntimeNodeID::g_str_runtime_node_id;
}


}
