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

#ifndef NUBULA_NET_ZPROTO_TRANSPORT_HANDLER_H_
#define NUBULA_NET_ZPROTO_TRANSPORT_HANDLER_H_

#include <wangle/channel/Handler.h>

#include "nebula/net/zproto/zproto_level_data.h"

// Transport Level处理器
class TransportHandler : public wangle::Handler<std::shared_ptr<ProtoRawData>, std::shared_ptr<ZProtoMessage>,
              std::unique_ptr<folly::IOBuf>, std::unique_ptr<folly::IOBuf>> {
public:
  TransportHandler() = default;
  
  void read(Context* ctx, std::shared_ptr<ProtoRawData> msg) override;
  folly::Future<folly::Unit> write(Context* ctx, std::unique_ptr<folly::IOBuf> msg) override {
    return ctx->fireWrite(std::move(msg));
  }

  ////////////////////////////////////////////////////////////////////////////
  void OnPlainTextMessage(Context* ctx, std::shared_ptr<PackageMessage> message_data);
  void OnEncryptedMessage(Context* ctx, std::shared_ptr<PackageMessage> message_data);
  void OnDrop4Package(Context* ctx, std::shared_ptr<PackageMessage> message_data);
  void OnRequestAuthId(Context* ctx, std::shared_ptr<PackageMessage> message_data);
  void OnRequestStartAuth(Context* ctx, std::shared_ptr<PackageMessage> message_data);
  void OnRequestGetServerKey(Context* ctx, std::shared_ptr<PackageMessage> message_data);
  void OnRequestDH(Context* ctx, std::shared_ptr<PackageMessage> message_data);
  void OnAuthIdInvalid(Context* ctx, std::shared_ptr<PackageMessage> message_data);
  
  ////////////////////////////////////////////////////////////////////////////
  void OnProtoRpcRequest(Context* ctx, std::shared_ptr<ZProtoMessage> message_data);
  void OnMessageAck(Context* ctx, std::shared_ptr<ZProtoMessage> message_data);
  void OnUnsentMessage(Context* ctx, std::shared_ptr<ZProtoMessage> message_data);
  void OnUnsentResponse(Context* ctx, std::shared_ptr<ZProtoMessage> message_data);
  void OnRequestResend(Context* ctx, std::shared_ptr<ZProtoMessage> message_data);
  void OnNewSession(Context* ctx, std::shared_ptr<ZProtoMessage> message_data);
  void OnSessionHello(Context* ctx, std::shared_ptr<ZProtoMessage> message_data);
  void OnSessionLost(Context* ctx, std::shared_ptr<ZProtoMessage> message_data);
};

#endif
