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

#ifndef NEBULA_NET_BASE_SERVICE_CONFIG_H_
#define NEBULA_NET_BASE_SERVICE_CONFIG_H_

#include "nebula/base/configurable.h"
#include "nebula/base/configuration.h"

namespace nebula {
  
struct ServiceConfig : public Configurable {
  virtual ~ServiceConfig() = default;
  
  // Override from Configurable
  bool SetConf(const std::string& conf_name, const Configuration& conf) override;
  
  std::string ToString() const;
  void PrintDebug() const;
  
  std::string name;   // 服务名
  std::string type;   // 服务类型
  std::string proto;  // 协议：默认为zproto
  std::string hosts;  // 主机地址（单台机器使用的多个IP采用‘,’分割）
  uint32_t    port;   // 端口号
  
  // 1. 对于tcp_server/http_server为最大连接数，未设置默认为40960
  // 2. 对于tcp_client为连接池大小，未设置默认为1
  uint32_t max_conn_cnt {40960};
};

using ServiceConfigPtr = std::shared_ptr<ServiceConfig>;
  
struct ServicesConfig : public Configurable {
  virtual ~ServicesConfig() = default;

  static std::vector<std::shared_ptr<ServiceConfig>> ToServiceConfigs(const Configuration& conf);

  bool SetConf(const std::string& conf_name, const Configuration& conf) override {
    service_configs = ToServiceConfigs(conf);
    return true;
  }
  
  void PrintDebug() const {
    for(auto& v : service_configs) {
      v->PrintDebug();
    }
  }
  
  std::vector<std::shared_ptr<ServiceConfig>> service_configs;
};
  
}

#endif
