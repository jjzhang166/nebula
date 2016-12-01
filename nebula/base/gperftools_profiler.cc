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

#include "nebula/base/gperftools_profiler.h"

#include <string>

#include "nebula/base/process_util.h"

namespace nebula {

#ifdef USE_GPERFTOOLS_PROFILER
static bool ReadFileToString(const char* path,
                      std::string* contents) {
  FILE* file = fopen(path, "rb");
  if (!file) {
    return false;
  }
  
  char buf[1 << 16];
  size_t len;
  bool read_status = true;
  
  // Many files supplied in |path| have incorrect size (proc files etc).
  // Hence, the file is read sequentially as opposed to a one-shot read.
  while ((len = fread(buf, 1, sizeof(buf), file)) > 0) {
    contents->append(buf, len);
  }
  
  read_status = ferror(file);
  fclose(file);
  
  return read_status;
}
#endif

GPerftoolsProfiler::GPerftoolsProfiler() {
    
#ifdef USE_GPERFTOOLS_PROFILER
  pid_ = base::GetProcessID();
  // exe_full_path_ = process_util::GetSelfProcessExecutablePath();
  // exe_name_ = process_util::GetSelfProcessName();
  state_ = kProfilerState_Ready;
#else
  state_ = kProfilerState_Disable;
#endif
}

GPerftoolsProfiler::~GPerftoolsProfiler() {
  ProfilerStop();
}

bool GPerftoolsProfiler::ProfilerStart() {
#ifdef USE_GPERFTOOLS_PROFILER
  if (state_ == kProfilerState_Running) {
    return false;
  }
  
  char buf[1024];
  snprintf(buf, 1024, "pprof.%d.prof", pid_);
  ::ProfilerStart(buf);
  state_ = kProfilerState_Running;
  return true;
#else
  return false;
#endif
}

void GPerftoolsProfiler::ProfilerStop() {
#ifdef USE_GPERFTOOLS_PROFILER
  if(state_ == kProfilerState_Running) {
    ::ProfilerFlush();
    ::ProfilerStop();
    
 /*
    char cmd[2048]={0};
    snprintf(cmd,2047,"pprof --callgrind  %s %s_%d.prof >%s_%d.callgrind"
             ,_programBin.c_str(),_programName.c_str(),_pid,_programName.c_str(),_pid);
    printf("cmd=%s\n",cmd);
    system(cmd);
    running_=false;
  */
    state_=kProfilerState_Stop;
  }
#endif
}

bool GPerftoolsProfiler::ToProfilerData(std::string* data) {
#ifdef USE_GPERFTOOLS_PROFILER
  if (data == NULL) {
    return false;
  }
  
  data->clear();
  if (state_ == kProfilerState_Stop) {
    char buf[1024];
    snprintf(buf, 1024,"pprof.%d.prof", pid_);
    return ReadFileToString(buf, data);
  } else {
    return false;
  }
  
#else
  return false;
#endif
}
  
}
