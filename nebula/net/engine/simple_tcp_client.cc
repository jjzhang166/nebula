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

#include "nebula/net/engine/simple_tcp_client.h"

#include <glog/logging.h>

#include <folly/Random.h>
#include <folly/SocketAddress.h>

namespace nebula {
  
//////////////////////////////////////////////////////////////////////////////
void SimpleTcpClient::Connect(const std::string& ip,
                              uint16_t port,
                              SimpleConnPipelineFactoryPtr factory) {
  if (!factory) {
    LOG(ERROR) << "SimpleTcpClient - ConnectByFactory, factory is null, ip = " << ip
    << ", port = " << port;
    return;
  }
  
  client_.pipelineFactory(factory);
  folly::SocketAddress address(ip.c_str(), port);
  client_.connect(address).then([this, ip, port](SimpleConnPipeline* pipeline) {
    //        LOG(INFO) << "SimpleTcpClient - Connect sucess: ip = " << ip << ", port = " << port;
    pipeline->setPipelineManager(this);
    this->connected_ = true;
  }).onError([this, ip, port](const std::exception& ex) {
    // TODO(@benqi), 是否要通知
    
    if (callback_) {
      callback_->OnConnectionError();
    }
    
    if (connect_error_callback_) {
      connect_error_callback_->OnClientConnectionError(this);
    }
    
    LOG(ERROR) << "SimpleTcpClient - Error connecting to :" << ip << ":" << port << ", exception: " << ex.what();
  });
}

void SimpleTcpClient::deletePipeline(wangle::PipelineBase* pipeline) {
  //    LOG(INFO) << "deletePipeline ";
  CHECK(client_.getPipeline() == pipeline);
  connected_ = false;
}

void SimpleTcpClient::refreshTimeout() {
  // LOG(INFO) << "refreshTimeout ";
}

}
