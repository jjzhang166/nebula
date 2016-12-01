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

#ifndef NEBULA_BASE_PROCESS_UTIL_H_
#define NEBULA_BASE_PROCESS_UTIL_H_

#include <stdint.h>
#include <string>

namespace nebula {
  
uint32_t GetProcessID();
uint32_t GetThreadID();
  
std::string GetProcessExecutablePath(uint32_t pid);
std::string GetProcessName(uint32_t pid);

std::string GetSelfProcessExecutablePath();
std::string GetSelfProcessName();

// 获取主机名
std::string GetHostName();
    
}

#endif // NEBULA_BASE_PROCESS_UTIL_H_
