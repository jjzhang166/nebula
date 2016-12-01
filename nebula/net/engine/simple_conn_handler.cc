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

#include "nebula/net/engine/simple_conn_handler.h"

#include <wangle/channel/EventBaseHandler.h>

namespace nebula {
  
///////////////////////////////////////////////////////////////////////////////////////////
SimpleConnHandler::SimpleConnHandler(SocketEventCallback* callback, const std::string& name)
  : callback_(callback),
    name_(name) {
  //    LOG(INFO) << "SimpleConnHandler::SimpleConnHandler(), Allocate: " << name;
}

SimpleConnHandler::~SimpleConnHandler() {
  //    LOG(INFO) << "SimpleConnHandler::SimpleConnHandler(), Destroy: " << name_;
}

void SimpleConnHandler::read(Context* ctx, std::unique_ptr<folly::IOBuf> msg) {
  //    LOG(INFO) << "SimpleConnHandler - read data: len = " << msg->length() << ", name = " << name_ << ", this: " << std::hex << this ;
  
  if (!msg) {
    // 为什么msg会为null？？？
    LOG(ERROR) << "read - recv a invalid msg_data, msg is null!!! by peer "
    << ctx->getPipeline()->getTransportInfo()->remoteAddr->getAddressStr()
    << ", name = " << name_;
    return;
  }
  int rv = 0;
  if (callback_) {
    rv = callback_->OnDataArrived(reinterpret_cast<SimpleConnPipeline*>(ctx->getPipeline()), std::move(msg));
  }
  if (rv == -1) {
    LOG(ERROR) << "read - callback_->OnDataArrived error, recv len " << msg->length()
    << ", by peer " << ctx->getPipeline()->getTransportInfo()->remoteAddr->getAddressStr();
    // 直接关闭
    close(ctx);
  }
  
  //LOG(INFO) << "SimpleConnHandler - read data end!";
}

folly::Future<folly::Unit> SimpleConnHandler::write(Context* ctx, std::unique_ptr<folly::IOBuf> out) {
//     LOG(INFO) << "conn_id = " << conn_id_ << ", IMConnHandler - write length: " << out->length();
  return ctx->fireWrite(std::forward<std::unique_ptr<folly::IOBuf>>(out));
}

void SimpleConnHandler::readEOF(Context* ctx) {
  LOG(INFO) << "readEOF - Connection closed by "
              << ctx->getPipeline()->getTransportInfo()->remoteAddr->getAddressStr()
              << ", name = " << name_;
  
  close(ctx);
}

void SimpleConnHandler::readException(Context* ctx, folly::exception_wrapper e) {
  LOG(ERROR) << "readException - Local error: " << exceptionStr(e)
              << ", by "  << ctx->getPipeline()->getTransportInfo()->remoteAddr->getAddressStr();
  
  close(ctx);
}

void SimpleConnHandler::transportActive(Context* ctx) {
  state_ = 1;
  LOG(INFO) << "transportActive - Connection connected by " << ctx->getPipeline()->getTransportInfo()->remoteAddr->getAddressStr()
              << ", name = " << name_;
  
  if (callback_) {
    callback_->OnNewConnection(reinterpret_cast<SimpleConnPipeline*>(ctx->getPipeline()));
  }
}
  
void SimpleConnHandler::transportInactive(Context* ctx) {
  if (state_ != 1) {
    return;
  }

  LOG(INFO) << "SimpleConnHandler - Connection closed by "
            << ctx->getPipeline()->getTransportInfo()->remoteAddr->getAddressStr()
            << ", name: " << name_;
  
  //    LOG(INFO) <<"close connection, t his:" <<std::hex << this;
  if (callback_) {
    callback_->OnConnectionClosed(reinterpret_cast<SimpleConnPipeline*>(ctx->getPipeline()));
  }
  
  state_ = 0;
}

folly::Future<folly::Unit> SimpleConnHandler::close(Context* ctx) {
  return ctx->fireClose();
}
  
}


