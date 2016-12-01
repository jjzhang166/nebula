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

#ifndef NEBULA_NET_ENGINE_CLUSTER_MANAGER_H_
#define NEBULA_NET_ENGINE_CLUSTER_MANAGER_H_

// ClusterManager
class BaseClusterManager {
public:
  virtual ~BaseClusterManager() = default;
  
  // 在线或离线
  virtual bool DoOnline() = 0;
  virtual bool DoOffline() = 0;
  
  // 当前状态
  virtual bool GetOnlineStatus() = 0;
  
private:
};

#endif
