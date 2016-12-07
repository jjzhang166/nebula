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

#ifndef NEBULA_NET_ENGINE_TCP_CLIENT_GROUP_H_
#define NEBULA_NET_ENGINE_TCP_CLIENT_GROUP_H_

#include "nebula/net/engine/tcp_client.h"

namespace nebula {
  
//////////////////////////////////////////////////////////////////////////////
// 功能：
//  1. 消息分发均匀分布，每个连接可分布在不同的线程
//  2. 可运行时新增和减少连接
// 实现思路
//  1. 每连接创建时通过IOThreadPoolExecutor分发到不同的线程里
//  2. tcp_client_group维护一个连接列表，此列表只新增不删除，一旦删除只是做个删除标记
//     因为从列表里移除，则除了要同步连接列表以外，还要同步tcp_client内容，锁粒度太大
// TODO(@benqi):
//  tcp_client添加状态
  
  
class TcpClientGroupBase : public TcpServiceBase {
public:
  typedef std::pair<uint64_t, std::weak_ptr<wangle::PipelineBase>> OnlineTcpClient;
  typedef std::vector<OnlineTcpClient> OnlineTcpClientList;

  TcpClientGroupBase(const ServiceConfig& config, const IOThreadPoolExecutorPtr& io_group)
    : TcpServiceBase(config, io_group) {}

  virtual ~TcpClientGroupBase() = default;

  // Impl from TcpConnEventCallback
  // 内网经常断线的可能性不大，故让tcp_client_group维护一个已经连接列表
  uint64_t OnNewConnection(wangle::PipelineBase* pipeline) override;
  
  // EventBase线程里执行
  bool OnConnectionClosed(uint64_t conn_id) override;
  
  // 获取client
  bool GetOnlineClientByRandom(OnlineTcpClient* client) const;
  
  // TODO(@benqi): 暂时未实现
  bool GetOnlineClientByConsistencyHash(OnlineTcpClient* client) const;
  
  bool GetOnlineClients(OnlineTcpClientList* clients) const;
  
protected:
  mutable std::mutex online_mutex_;
  OnlineTcpClientList online_clients_;
  
  TcpConnEventCallback* group_event_callback_{nullptr};
};
  
template<typename Pipeline = DefaultPipeline>
class TcpClientGroup : public TcpClientGroupBase {
public:
  typedef std::vector<std::shared_ptr<TcpClient<Pipeline>>> TcpClientList;
  // typedef std::vector<std::pair<uint64_t, std::weak_ptr<wangle::PipelineBase>> OnlineTcpClientList;

  TcpClientGroup(const ServiceConfig& config, const IOThreadPoolExecutorPtr& io_group)
      : TcpClientGroupBase(config, io_group) {}
  
  virtual ~TcpClientGroup() = default;
  
  // Impl from TcpServiceBase
  ServiceModuleType GetModuleType() const override {
      return ServiceModuleType::TCP_CLIENT_GROUP;
  }

  bool AddChild(std::shared_ptr<ServiceBase> child) override {
    // TODO(@benqi)
    //   检查是否是TcpClient
    //   检查类型名和服务名是否一样
    
    std::static_pointer_cast<TcpClient<Pipeline>>(child)->set_group_event_callback(this);
    
    std::lock_guard<std::mutex> g(mutex_);
    clients_.push_back(std::static_pointer_cast<TcpClient<Pipeline>>(child));
    
    return true;
  }

  //////////////////////////////////////////////////////////////////////////
  bool Start() override {
    std::lock_guard<std::mutex> g(mutex_);
    // TODO(@benqi):
    // 检查状态，未删除的才能Start
    for (auto& c : clients_) {
      c->Start();
    }
    return true;
  }
  
  bool Pause() override {
    return true;
  }
  
  bool Stop() override {
    LOG(INFO) << "TcpClientGroup - Stop service: " << GetServiceConfig().ToString();
    
    std::lock_guard<std::mutex> g(mutex_);
    for (auto& c : clients_) {
      c->Stop();
    }
    
    return true;
  }
  

protected:
  TcpClientList GetOnlineClients() {
    TcpClientList clients;
    
    std::lock_guard<std::mutex> g(mutex_);
    for (auto& c : clients_) {
      clients.push_back(c);
    }
    
    return clients;
  }
  
  // 线程同步
  std::mutex mutex_;
  TcpClientList clients_;
};

}

#endif
