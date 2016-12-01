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

#include "nebula/base/process_util.h"

#include <unistd.h>
#include <stdio.h>

#ifdef __APPLE__
#include <libproc.h>
#include <pthread.h>
#endif

#include <sys/types.h>
#include <sys/syscall.h>

namespace nebula {

// 进程id
uint32_t GetProcessID() {
  return (uint32_t) getpid();
}

// 进程id
uint32_t GetThreadID() {
#ifdef __APPLE__
  return pthread_mach_thread_np(pthread_self());
#else
  return syscall(__NR_gettid);
#endif
}

// 进程可执行路径
std::string GetProcessExecutablePath(uint32_t pid) {
  std::string execute_path;
  
#ifdef __APPLE__
  char pathbuf[PROC_PIDPATHINFO_MAXSIZE];
  if (proc_pidpath(pid, pathbuf, sizeof(pathbuf))) {
    execute_path = pathbuf;
  }
#else
  FILE *fptr;
  char cmd[255] = {'\0'};
  char name[255];
  sprintf(cmd,"readlink /proc/%d/exe",pid);
  if((fptr = popen(cmd,"r")) != NULL) {
    if(fgets(name,255,fptr) != NULL) {
      execute_path = name;
    }
  }
  
  pclose(fptr);
#endif
  
  return execute_path;
}

std::string GetProcessName(uint32_t pid) {
  std::string exe_path = GetProcessExecutablePath(pid);
  
  size_t pos=exe_path.find_last_of('/');
  if(pos != std::string::npos) {
    return exe_path.substr(pos+1);
  } else {
    return  exe_path;
  }
}

std::string GetSelfProcessExecutablePath() {
  uint32_t pid = GetProcessID();
  return GetProcessExecutablePath(pid);
}

std::string GetSelfProcessName() {
  uint32_t pid = GetProcessID();
  return GetProcessName(pid);
}
  
std::string GetHostName() {
  char hostname[64] = {0};
  int retcode = gethostname(hostname, sizeof(hostname));
  if ( retcode != 0 ) {
    return std::string("");
  }
  return std::string(hostname);
}

}

