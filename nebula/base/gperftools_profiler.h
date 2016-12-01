/**
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

// gperftools(https://code.google.com/p/gperftools)
// 简单封装
//

#ifndef NEBULA_BASE_GPERFTOOLS_PROFILER_H_
#define NEBULA_BASE_GPERFTOOLS_PROFILER_H_

#include "nebula/base/basictypes.h"

#include <stdio.h>
#include <stdint.h>
#include <string>

#ifdef USE_GPERFTOOLS_PROFILER
#include "nebula/gperftools/profiler.h"
#endif

namespace nebula {
  
// gperftools profiler简单封装
class GPerftoolsProfiler {
public:
  enum ProfilerState {
    kProfilerState_Disable = 0, // 未启用profiler, 未定义USE_GPERFTOOLS_PROFILER
    kProfilerState_Ready,       // 启用profiler，但还未采集过数据，即还未运行过ProfilerStart()
    kProfilerState_Running,     // 正在采集数据，已经运行过ProfilerStart()，但还未运行ProfilerStop()
    kProfilerState_Stop         // 已经采集过数据，但现在处于停止状态，已经运行了ProfilerStop()
  };
  
  GPerftoolsProfiler();
  ~GPerftoolsProfiler();

  // 开始采集数据
  bool ProfilerStart();
  // 停止采集数据
  void ProfilerStop();
  
  // 获取已经采集到的数据
  // 只在kProfilerState_Stop状态时才有数据
  bool ToProfilerData(std::string* data);
  
  // 是否停止状态
  bool IsStopState() const {
    return state_ == kProfilerState_Stop;
  }
  
private:
  int         state_;   // ProfilerState
  
#ifdef USE_GPERFTOOLS_PROFILER
  uint32_t    pid_;
  // std::string exe_full_path_;
  // std::string exe_name_;
#endif
};
  
}

#endif // NEBULA_BASE_GPERFTOOLS_PROFILER_H_

