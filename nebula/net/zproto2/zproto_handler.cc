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

#include "nebula/net/zproto/zproto_handler.h"

#include "nebula/net/thread_local_conn_manager.h"
#include "nebula/net/zproto/zproto_event_callback.h"

///////////////////////////////////////////////////////////////////////////////////////////
// using
// using DataReceivedFunc = std::function<int(NebulaPipelinePtr, std::shared_ptr<ZProtoMessage>)>;

void ZProtoHandler::read(Context* ctx, std::shared_ptr<BasicSyncMessage> msg) {
  int rv = ZProtoEventCallback::OnDataReceived(std::static_pointer_cast<NebulaPipeline>(ctx->getPipelineShared()),
                                               msg);
  if (rv == -1) {
    // TODO(@wubenqi): 是否需要断开或其它处理
  }

  // FuncFactoryManager<int>
  
  
  // LOG(INFO) << "conn_id = " << conn_id_ << ", ZProtoHandler - read length: " << msg->length();
  // write(ctx, std::move(msg));
//  switch (msg->GetClassID()) {
//    case TL_req_pq::CLASS_ID :
//      // On_TL_req_pq(ctx, msg);
//      break;
//    case TL_req_DH_params::CLASS_ID:
//      // On_TL_req_DH_params(ctx, msg);
//      break;
//    case TL_set_client_DH_params::CLASS_ID:
//      // On_TL_set_client_DH_params(ctx, msg);
//      break;
//    default:
//      break;
//  }
}

//// PQ默认值:
////
//// static const uint8_t kDefaultPQ[8] = {0x17, 0xED, 0x48, 0x94, 0x1A, 0x08, 0xF9, 0x81};
//// static const uint8_t kDefaultP[4] = {0x49, 0x4C, 0x55, 0x3B};
//// static const uint8_t kDefaultQ[4] = {0x53, 0x91, 0x10, 0x73};
//
//void ZProtoHandler::On_TL_req_pq(Context* ctx, std::shared_ptr<MTProtoBase> mtproto) {
//  // MTProto<TL_req_pq>& req_pq = *(mtproto->Cast<TL_req_pq>());
//  // std::shared_ptr<MTProto<TL_resPQ>> resPQ = std::make_shared<MTProto<TL_resPQ>>();
//  // (*resPQ)->nonce = req_pq->nonce();
//}
//
//void ZProtoHandler::On_TL_req_DH_params(Context* ctx, std::shared_ptr<MTProtoBase> mtproto) {
//  
//}
//
//void ZProtoHandler::On_TL_set_client_DH_params(Context* ctx, std::shared_ptr<ZProtoBase> mtproto) {
//  
//}

folly::Future<folly::Unit> ZProtoHandler::write(Context* ctx, std::unique_ptr<folly::IOBuf> out) {
  // LOG(INFO) << "conn_id = " << conn_id_ << ", ZProtoHandler - write length: " << out->length();
  //所有发送数据字节数统计
  // return ctx->fireWrite(std::shared_ptr<BaseMTProtoMessage>(out));
  return ctx->fireWrite(std::forward<std::unique_ptr<folly::IOBuf>>(out));
}

void ZProtoHandler::readEOF(Context* ctx) {
  LOG(INFO) << "readEOF - conn_id = " << conn_id_ << ", ZProtoHandler - Connection closed by "
              << remote_address_
              << ", conn_info: " << service_->GetServiceConfig().ToString();
  close(ctx);
}

void ZProtoHandler::readException(Context* ctx, folly::exception_wrapper e) {
  LOG(ERROR) << "readException - conn_id = " << conn_id_
              << ", ZProtoHandler - Local error: " << exceptionStr(e)
              << ", by "  << remote_address_
              << ", conn_info: " << service_->GetServiceConfig().ToString();
  close(ctx);
}

void ZProtoHandler::transportActive(Context* ctx) {
  // 连接建立
  conn_state_ = kConnected;
  
  auto pipeline = std::static_pointer_cast<NebulaPipeline>(ctx->getPipelineShared());
  
  // 缓存连接地址
  remote_address_ = pipeline->getTransportInfo()->remoteAddr->getAddressStr();
  //->getTransportInfo()->remoteAddr->getAddressStr();
  
  conn_id_ = service_->OnNewConnection(pipeline.get());
  
  int rv = ZProtoEventCallback::OnNewConnection(service_, pipeline);
  if (rv == -1) {
    // TODO(@wubenqi): 是否需要断开或其它处理
  }
  
  LOG(INFO) << "transportActive - conn_id = " << conn_id_
              << ", ZProtoHandler - Connection connected by "
              << remote_address_
              << ", conn_info: " << service_->GetServiceConfig().ToString();
  
  // conn_state_ = kWait_TL_req_pq;
}

folly::Future<folly::Unit> ZProtoHandler::close(Context* ctx) {
  if (conn_state_ == kConnected) {
    
    // 有可能close了多次，但是只能回调一次
    // 通过conn_state_保证执行一次
    int rv = ZProtoEventCallback::OnConnectionClosed(service_,
                                                     std::static_pointer_cast<NebulaPipeline>(ctx->getPipelineShared()));
    if (rv == -1) {
      // TODO(@wubenqi): 是否需要断开或其它处理
    }
    
    service_->OnConnectionClosed(conn_id_);
    
    conn_state_ = kClosed;
    remote_address_.clear();
  }
  
  return ctx->fireClose();
}

