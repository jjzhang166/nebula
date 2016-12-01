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

#include "nebula/net/zproto/zproto_transport_handler.h"

#include "nebula/base/func_factory_manager.h"

///////////////////////////////////////////////////////////////////////////////////////
// 初始化
// Package
typedef void(TransportHandler::*ExecPackageHandler)(TransportHandler::Context*, std::shared_ptr<PackageMessage>);
typedef nebula::FuncFactoryManager<ExecPackageHandler, int> ExecPackageHandlerFactory;
#define REGISTER_EXECUTE_PACKAGE_HANDLER(T) \
  static ExecPackageHandlerFactory::RegisterTemplate g_reg_transport_package_handler_##T(T::HEADER, &TransportHandler::On##T)

REGISTER_EXECUTE_PACKAGE_HANDLER(PlainTextMessage);
//REGISTER_EXECUTE_PACKAGE_HANDLER(EncryptedMessage);
//REGISTER_EXECUTE_PACKAGE_HANDLER(Drop4Package);
//REGISTER_EXECUTE_PACKAGE_HANDLER(RequestAuthId);
//REGISTER_EXECUTE_PACKAGE_HANDLER(RequestStartAuth);
//REGISTER_EXECUTE_PACKAGE_HANDLER(RequestGetServerKey);
//REGISTER_EXECUTE_PACKAGE_HANDLER(RequestDH);
// REGISTER_EXECUTE_HANDLER(AuthIdInvalid);

// ZProto
typedef void(TransportHandler::*ExecZProtoHandler)(TransportHandler::Context*, std::shared_ptr<ZProtoMessage>);
typedef nebula::FuncFactoryManager<ExecZProtoHandler, int> ExecZProtoHandlerFactory;
#define REGISTER_EXECUTE_ZPROTO_HANDLER(T) \
  static ExecZProtoHandlerFactory::RegisterTemplate g_reg_transport_zproto_handler_##T(T::HEADER, &TransportHandler::On##T)

REGISTER_EXECUTE_ZPROTO_HANDLER(ProtoRpcRequest);
//REGISTER_EXECUTE_ZPROTO_HANDLER(MessageAck);
//REGISTER_EXECUTE_ZPROTO_HANDLER(UnsentMessage);
//REGISTER_EXECUTE_ZPROTO_HANDLER(UnsentResponse);
//REGISTER_EXECUTE_ZPROTO_HANDLER(RequestResend);
//REGISTER_EXECUTE_ZPROTO_HANDLER(NewSession);
//REGISTER_EXECUTE_ZPROTO_HANDLER(SessionHello);
//REGISTER_EXECUTE_ZPROTO_HANDLER(SessionLost);

////////////////////////////////////////////////////////////////////////////////////////////////////////
void TransportHandler::read(Context* ctx, std::shared_ptr<ProtoRawData> msg) {
  Package package;
  if (!package.Decode(*msg)) {
    LOG(ERROR) << "";
    
    ctx->fireReadException(folly::make_exception_wrapper<std::runtime_error>("TransportHandler - Decode package error"));
    return;
  }
  
  // if (package.message_header)
  auto message_data = PackageFactory::CreateSharedInstance(package.package_type);
  message_data->Decode(package);
  ExecPackageHandlerFactory::Execute2<TransportHandler>(this, package.package_type, ctx, message_data);
}

void TransportHandler::OnPlainTextMessage(Context* ctx, std::shared_ptr<PackageMessage> message_data) {
  auto plain_text = std::static_pointer_cast<PlainTextMessage>(message_data);
  
  ZProto zproto;
  zproto.Decode(*plain_text);
  
  auto zproto_message = ZProtoFactory::CreateSharedInstance(zproto.zproto_type);
  ExecZProtoHandlerFactory::Execute2<TransportHandler>(this, zproto.zproto_type, ctx, zproto_message);
  // ctx->fireRead(message_data);
}

void TransportHandler::OnEncryptedMessage(Context* ctx, std::shared_ptr<PackageMessage> message_data) {
  
}

void TransportHandler::OnDrop4Package(Context* ctx, std::shared_ptr<PackageMessage> message_data) {
  
}

void TransportHandler::OnRequestAuthId(Context* ctx, std::shared_ptr<PackageMessage> message_data) {
  
}

void TransportHandler::OnRequestStartAuth(Context* ctx, std::shared_ptr<PackageMessage> message_data) {
  
}

void TransportHandler::OnRequestGetServerKey(Context* ctx, std::shared_ptr<PackageMessage> message_data) {
  
}

void TransportHandler::OnRequestDH(Context* ctx, std::shared_ptr<PackageMessage> message_data) {
  
}

void TransportHandler::OnAuthIdInvalid(Context* ctx, std::shared_ptr<PackageMessage> message_data) {
  
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
void TransportHandler::OnProtoRpcRequest(Context* ctx, std::shared_ptr<ZProtoMessage> message_data) {
  return ctx->fireRead(message_data);
}

void TransportHandler::OnMessageAck(Context* ctx, std::shared_ptr<ZProtoMessage> message_data) {
  
}

void TransportHandler::OnUnsentMessage(Context* ctx, std::shared_ptr<ZProtoMessage> message_data) {
  
}

void TransportHandler::OnUnsentResponse(Context* ctx, std::shared_ptr<ZProtoMessage> message_data) {
  
}

void TransportHandler::OnRequestResend(Context* ctx, std::shared_ptr<ZProtoMessage> message_data) {
  
}

void TransportHandler::OnNewSession(Context* ctx, std::shared_ptr<ZProtoMessage> message_data) {
  
}

void TransportHandler::OnSessionHello(Context* ctx, std::shared_ptr<ZProtoMessage> message_data) {
  
}

void TransportHandler::OnSessionLost(Context* ctx, std::shared_ptr<ZProtoMessage> message_data) {
  
}

