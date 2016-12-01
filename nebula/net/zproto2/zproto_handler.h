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

#ifndef NUBULA_NET_ZPROTO_ZPROTO_HANDLER_H_
#define NUBULA_NET_ZPROTO_ZPROTO_HANDLER_H_

#include <string>

#include <wangle/channel/AsyncSocketHandler.h>
#include <wangle/channel/Pipeline.h>

#include "nebula/net/server/tcp_service_base.h"
#include "nebula/net/zproto/zproto_level_data.h"

class TcpServiceBase;

// 使用AttachData存储连接附加数据
// 比如用户名等
struct ConnAttachData {
  virtual ~ConnAttachData() = default;
};

typedef std::shared_ptr<ConnAttachData> ConnAttachDataPtr;

//class ZProtoBase {
//public:
//  virtual ~ZProtoBase() = default;
//};

class ZProtoHandler : public wangle::HandlerAdapter<std::shared_ptr<BasicSyncMessage>, std::unique_ptr<folly::IOBuf>> {
public:
  enum ConnState {
    kNone = 0,
    kConnected,                     // 连接建立
    kWait_TL_req_pq,                // 等TL_req_pq
    kWait_TL_req_DH_params,         // 等req_DH_params
    kWait_TL_set_client_DH_params,  // 等TL_set_client_DH_params
    kRpcRunning,                    // 可以running
    kClosed,
    kError
  };
  
  explicit ZProtoHandler(nebula::ServiceBase* service)
    : service_(dynamic_cast<nebula::TcpServiceBase*>(service)),
      conn_id_(0),
  conn_state_(kNone) {
  }
  
  //////////////////////////////////////////////////////////////////////////
  inline nebula::TcpServiceBase* GetServiceBase() {
    return service_;
  }
  
  inline ConnAttachData* GetAttachData() {
    return attach_data_.get();
  }
  
  //////////////////////////////////////////////////////////////////////////
  template <class T>
  inline T* CastByGetAttachData() {
    return nullptr;
  }
  
  template <class T>
  inline typename std::enable_if<std::is_base_of<ConnAttachData, T>::value>::type*
  CastByGetAttachData() {
    return dynamic_cast<T*>(attach_data_.get());
  }
  
  inline void SetAttachData(const std::shared_ptr<ConnAttachData>& v) {
    attach_data_ = v;
  }
  
  inline uint64_t GetConnID() const {
    return conn_id_;
  }
  
  //////////////////////////////////////////////////////////////////////////
  // 重载 HandlerAdapter<std::unique_ptr<IOBuf>>
  void read(Context* ctx, std::shared_ptr<BasicSyncMessage> msg) override;
  folly::Future<folly::Unit> write(Context* ctx, std::unique_ptr<folly::IOBuf> out) override;
  
  void readEOF(Context* ctx) override;
  void readException(Context* ctx, folly::exception_wrapper e) override;
  
  void transportActive(Context* ctx) override;
  folly::Future<folly::Unit> close(Context* ctx) override;
  
  inline const std::string& GetRemoteAddress() const {
    return remote_address_;
  }
  
protected:
//  void On_TL_req_pq(Context* ctx, std::shared_ptr<MTProtoBase> mtproto);
//  void On_TL_req_DH_params(Context* ctx, std::shared_ptr<MTProtoBase> mtproto);
//  void On_TL_set_client_DH_params(Context* ctx, std::shared_ptr<MTProtoBase> mtproto);
//  
  // 全局的
  nebula::TcpServiceBase* service_{nullptr};
  uint64_t conn_id_;
  std::shared_ptr<ConnAttachData> attach_data_;
  int conn_state_ {kNone};
  
  std::string remote_address_;
};

#endif
