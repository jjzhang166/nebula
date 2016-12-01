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

#ifndef NEBULA_NET_BASE_SERVICE_BASE_H_
#define NEBULA_NET_BASE_SERVICE_BASE_H_

#include <wangle/concurrent/IOThreadPoolExecutor.h>

#include "nebula/net/base/service_config.h"
// #include "nebula/base/factory_object.h"
#include "nebula/base/func_factory_manager.h"

namespace folly {
class IOBuf;
}

namespace nebula {
  
// 通过服务类型和服务名创建ServiceBase
// 服务类型为:
//   tcp_server
//   tcp_client
//   http_server
// 服务名为每个应用自定义:
//   比如zhazhad的服务名zhazha_server
//   通过配置创建ServiceBase
//   如果未设置服务名，则使用服务类型值为默认服务名：
//    tcp_server <-> tcp_server
//    tcp_client <-> tcp_client
//    udp_server <-> udp_server
//    udp_client <-> udp_client
//    http_server <-> http_server
//    http_client <-> http_client

// 数据转发策略
enum class DispatchStrategy : int {
  kDefault = 0,           // 默认，使用kRandom
  kRandom = 1,            // 随机
  kRoundRobin = 2,        // 轮询
  kConsistencyHash = 3,   // 一致性hash
  kBroadCast = 4,         // 广播
};

enum class ServiceModuleType : int {
    INVALID = 0,
    TCP_SERVER = 1,
    TCP_CLIENT_GROUP = 2,
    TCP_CLIENT_POOL = 3,
    TCP_CLIENT = 4,
    UDP_SERVER = 5,
    UDP_CLIENT = 6,
    HTTP_SERVER = 7,
    RPC_SERVER = 8,
    RPC_CLIENT = 9,
    MAX_SIZE = 10
};
  
class ServiceBase;

// 服务基础类
// 注意：
//   一个ServiceBase并不和一个ServiceType对应
//   和ServiceModuleType对应
class ServiceBase {
public:
  ServiceBase(const ServiceConfig& config)
    : config_(config) {}
  
  virtual ~ServiceBase() = default;
  
  // 模块名
  // ServiceBase有一些中间类型，特别是client
  // 有n个连接到同一服务不同主机上的tcp_client，我们会将这些连接分组到tcp_client_group
  // 同一主机上的同一服务我们也可能发起多个连接，我们会将这些连接分组到tcp_client_pool
  virtual ServiceModuleType GetModuleType() const = 0;
  
  // 服务配置信息
  inline const ServiceConfig& GetServiceConfig() const {
    return config_;
  }
  inline const std::string& GetServiceName() const {
    return config_.name;
  }
  inline const std::string& GetServiceType() const {
    return config_.type;
  }
  inline const std::string& GetServiceProto() const {
    return config_.proto;
  }
  
  virtual bool Start() { return false; }
  virtual bool Pause() { return false; }
  virtual bool Stop() { return false; }
  
  // TcpClientGroup或TcpClientPool使用
  virtual bool AddChild(std::shared_ptr<ServiceBase> service) { return false; }
  
  // TODO(@benqi): 实现RemoveChild
  // virtual void RemoveChild(ServiceBase* service) {}
  
protected:
  ServiceConfig config_;
};
  
// using ServiceSelfRegisterTemplate = ServiceSelfRegisterFactoryManager::RegisterTemplate<>;

using ServiceBasePtr = std::shared_ptr<ServiceBase>;

/*
//////////////////////////////////////////////////////////////////////////////
class ServiceBaseFactory : public FactoryObject<ServiceBase> {
public:
  explicit ServiceBaseFactory(const std::string& name)
    : name_(name) {}
  
  virtual ~ServiceBaseFactory() = default;
  
  // 服务名: 由配置进行设置，比如zhazhad
  virtual const std::string& GetName() const {
    return name_;
  }

  // 协议: 由配置进行设置，比如zproto、echo等
  virtual const std::string& GetProto() const {
    return proto_;
  }

  virtual ServiceBasePtr CreateInstance(const ServiceConfig& config) const = 0;
  
protected:
  ServiceBasePtr CreateInstance() const override {
    return nullptr;
  }
  
  std::string name_;
  std::string proto_;
};

using ServiceBaseFactoryPtr = std::shared_ptr<ServiceBaseFactory>;
*/
  
using NewServiceBaseFunc = std::function<ServiceBasePtr(const ServiceConfig&, const std::shared_ptr<wangle::IOThreadPoolExecutor>&)>;
  
using ServiceSelfRegisterFactoryManager = FuncFactoryManager<NewServiceBaseFunc, std::pair<std::string, std::string>>;
using ServiceSelfRegisterTemplate = ServiceSelfRegisterFactoryManager::RegisterTemplate;


}

  
#endif
