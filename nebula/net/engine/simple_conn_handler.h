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

#ifndef NEBULA_NET_ENGINE_SIMPLE_CONN_HANDLER_H_
#define NEBULA_NET_ENGINE_SIMPLE_CONN_HANDLER_H_

#include <string>
#include <wangle/channel/AsyncSocketHandler.h>

namespace nebula {

using SimpleConnPipeline = wangle::Pipeline<folly::IOBufQueue&, std::unique_ptr<folly::IOBuf>>;

class SocketEventCallback {
public:
  virtual ~SocketEventCallback() = default;
  
  virtual void OnNewConnection(SimpleConnPipeline* pipeline) = 0;
  virtual void OnConnectionClosed(SimpleConnPipeline* pipeline) = 0;
  virtual void OnConnectionError() {}
  virtual int  OnDataArrived(SimpleConnPipeline* pipeline, std::unique_ptr<folly::IOBuf> buf) = 0;
};

class SimpleConnHandler : public wangle::HandlerAdapter<std::unique_ptr<folly::IOBuf>> {
public:
  SimpleConnHandler(SocketEventCallback* callback, const std::string& name = "");
  ~SimpleConnHandler() override;
  
  // Impl from SimpleConnHandler
  void read(Context* ctx, std::unique_ptr<folly::IOBuf> msg) override;
  folly::Future<folly::Unit> write(Context* ctx, std::unique_ptr<folly::IOBuf> out) override;
  void readEOF(Context* ctx) override;
  void readException(Context* ctx, folly::exception_wrapper e) override;
  void transportActive(Context* ctx) override;
  void transportInactive(Context* ctx) override;

  folly::Future<folly::Unit> close(Context* ctx) override;
  
  const std::string& GetName() const {
    return name_;
  }
  
  void ClearCalllback() {
    callback_ = nullptr;
  }
private:
  SocketEventCallback* callback_{nullptr};
  std::string name_;
  
  int state_ {0};
};

}

using SimpleConnPipelineFactory = wangle::PipelineFactory<nebula::SimpleConnPipeline>;
using SimpleConnPipelineFactoryPtr = std::shared_ptr<SimpleConnPipelineFactory>;


#endif // NEBULA_NET_ENGINE_SIMPLE_CONN_HANDLER_H_

