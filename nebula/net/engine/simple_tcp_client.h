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
 *
 */

#ifndef NEBULA_NET_ENGINE_SIMPLE_TCP_CLIENT_H_
#define NEBULA_NET_ENGINE_SIMPLE_TCP_CLIENT_H_

#include "nebula/net/engine/simple_conn_handler.h"
#include "nebula/net/base/client_bootstrap2.h"

namespace nebula {
  
class SimpleTcpClient;
class ConnectErrorCallback {
public:
  virtual void OnClientConnectionError(SimpleTcpClient* tcp_client) {}
};


// 使用场景：测试客户端，代理等有生命周期、需要管理的Client连接
// TODO(@benqi)
//  如果连接断开以后，如何保证数据可靠
//  先不管
class SimpleTcpClient : public wangle::PipelineManager {
public:
  explicit SimpleTcpClient(folly::EventBase* base,
                           SocketEventCallback* callback = nullptr,
                           const std::string& name = "", ConnectErrorCallback* connect_error_callback = nullptr) :
  callback_(callback),
		connect_error_callback_(connect_error_callback),
  client_(base),
  name_(name) {
    
  }
  
  virtual ~SimpleTcpClient() {
    if (client_.getPipeline()) {
      client_.getPipeline()->setPipelineManager(nullptr);
    }
  }
  
  SimpleConnPipeline* GetPipeline() {
    return client_.getPipeline();
  }
  
  folly::EventBase* GetEventBase() {
    return client_.getEventBase();
  }
  
  // 注意，需要指定factory
  void Connect(const std::string& ip,
               uint16_t port,
               SimpleConnPipelineFactoryPtr factory);
  
  // PipelineManager implementation
  void deletePipeline(wangle::PipelineBase* pipeline) override;
  void refreshTimeout() override;
  
  bool connected() {
    return connected_;
  }
  
  void SetSocketEventCallback(SocketEventCallback* cb) {
    callback_ = cb;
  }
  
  void SetConnectErrorCallback(ConnectErrorCallback* cb) {
    connect_error_callback_ = cb;
  }
  
  void clearCallBack()  {
    callback_ = nullptr;
    connect_error_callback_ = nullptr;
  }
  
  const std::string& GetName() const {
    return name_;
  }
  
private:
  // void DoHeartBeat(bool is_send);
  // 指定该连接所属EventBase线程
  // folly::EventBase* base_ = nullptr;
  bool connected_ {false};
  SocketEventCallback* callback_{nullptr};
  ConnectErrorCallback* connect_error_callback_{nullptr};
  
  wangle::ClientBootstrap2<SimpleConnPipeline> client_;
  std::string name_;
};

}


#endif
