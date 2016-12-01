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
  
// @benqi: 以前的设计
// 线程安全的保证
// 问题的提出:
//  某一类的服务器属于一个服务器组，比如线上IM的一组biz_server_buddy
//  共部署了8个实例
//  在logic_server_buddy会从8个实例里找出一个连接收发数据
//  这8个连接可能会断线或成功重连
// 引出下面的问题:
//  假如所属同一组的8个连接都落在不同的线程，则网络事件(连接建立或断开)的触发分属不同线程
//  从这个分组里随机找出可用连接的话，需要保护clients_
//  随之而来的是需要考虑每一步骤是否会需要同步，一旦考虑不好会引发各种各样问题。
// 采用一种比较简单的方式解决线程安全问题
//  在创建group时就给这个分组分配一个固定的线程，以后属于这个分组的所有网络事件都在这个线程里触发
//  其他线程给这个分组发送数据包时，通过runInEventBaseThread将数据包投递到该分组所属的线程
//  然后在这个分组线程里进行处理策略
//  如果所属分组线程已经没可用连接了，那也没办法，只好丢掉
//  即使是当前线上的单线程实现，分组里没可用连接，也只好丢掉。

// @benqi: 当前设计(需要性能测试)
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
  uint64_t OnNewConnection(wangle::PipelineBase* pipeline) override {
    auto conn_id = TcpServiceBase::OnNewConnection(pipeline); {
      std::weak_ptr<wangle::PipelineBase> cli(pipeline->shared_from_this());
      // TODO(@benqi): 读多写少，用读写锁
      std::lock_guard<std::mutex> g(online_mutex_);
      online_clients_.push_back(std::make_pair(conn_id, cli));
    }
    return conn_id;
  }
  
  // EventBase线程里执行
  bool OnConnectionClosed(uint64_t conn_id) override {
    {
      std::lock_guard<std::mutex> g(online_mutex_);
      for (auto it=online_clients_.begin(); it!=online_clients_.end(); ++it) {
        if (it->first == conn_id) {
          online_clients_.erase(it);
          break;
        }
      }
    }
    
    return TcpServiceBase::OnConnectionClosed(conn_id);
  }
  
  // 获取client
  bool GetOnlineClientByRandom(OnlineTcpClient* client) const {
    bool rv = true;
    
    folly::ThreadLocalPRNG rng; {
      std::lock_guard<std::mutex> g(online_mutex_);
      if (!online_clients_.empty()) {
        uint32_t idx = folly::Random::rand32(static_cast<uint32_t>(online_clients_.size()));
        *client = online_clients_[idx];
      } else {
        rv = false;
      }
    }
    
    return rv;
  }
  
  bool GetOnlineClientByConsistencyHash(OnlineTcpClient* client) const {
    bool rv = false;
    
    // std::lock_guard<std::mutex> g(mutex_);
    // uint32_t idx = folly::Random::rand32(static_cast<uint32_t>(online_clients_.size()));
    // dispatch_id = RendezvousHash(service_ip_available.begin(), service_ip_available.end()).get(key);
    // return online_clients_[0];
    
    return rv;
  }
  
  bool GetOnlineClients(OnlineTcpClientList* clients) const {
    bool rv = true; {
      std::lock_guard<std::mutex> g(online_mutex_);
      
      if (!online_clients_.empty()) {
        *clients = online_clients_;
      } else {
        rv = false;
      }
    }
    return rv;
  }
  
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

#if 0
  // TODO(@benqi)
  //  存在线程安全的问题
  //  如何保证？？？
  // 考虑如下解决方案：
  //  当创建分组时，保证这个分组里的所有连接都落在一个线程里
  //  投递数据给分组时，先将网络包发送到分组所属的线程
  //  然后在这个线程里按策略分发数据
  // 简单实现
  //  为一组主动发起的连接创建全分配到一个线程里
  //  所有主动发起的连接共用这个线程
  bool SendIOBuf(std::unique_ptr<folly::IOBuf> data, DispatchStrategy dispatch_strategy) {
    auto onlines = GetOnlineClients();
    if (onlines.empty()) {
      LOG(ERROR) << "SendIOBuf - Not active tcp_client: ";
      return false;
    }
    switch (dispatch_strategy) {
      case DispatchStrategy::kBroadCast:
        // 广播: 每个都发，保险起见，直接使用
        for (auto & c : onlines) {
          if (c->connected()) {
            c->SendIOBufThreadSafe(std::move(data->clone()));
          }
        }
        break;
      case DispatchStrategy::kDefault:
      case DispatchStrategy::kRandom:
      case DispatchStrategy::kRoundRobin:
      case DispatchStrategy::kConsistencyHash:
      default: {
        // TODO(@benqi)
        //  后续补上其他的分发策略
        //  当前只支持随机策略
        folly::ThreadLocalPRNG rng;
        uint32_t idx = folly::Random::rand32(static_cast<uint32_t>(onlines.size()));
        onlines[idx]->SendIOBufThreadSafe(std::move(data));
        
        LOG(INFO) << "SendIOBuf - dispatch_strategy, dispatch idx: " << idx;
      }
        break;
    }
    
    return true;
  }
  
  bool SendIOBuf(std::unique_ptr<folly::IOBuf> data,
                                 const std::function<void()>& c) {
    auto onlines = GetOnlineClients();
    if (onlines.empty()) {
      LOG(ERROR) << "SendIOBuf - Not active tcp_client: ";
      return false;
    }
    
    folly::ThreadLocalPRNG rng;
    uint32_t idx = folly::Random::rand32(static_cast<uint32_t>(onlines.size()));
    onlines[idx]->SendIOBufThreadSafe(std::move(data), c);
    // LOG(INFO) << "SendIOBuf - dispatch_strategy, dispatch idx: " << idx;
    return true;
    
  }
#endif
  
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
