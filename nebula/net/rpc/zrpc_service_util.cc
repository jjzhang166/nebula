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

#include "nebula/net/rpc/zrpc_service_util.h"

#include <folly/MoveWrapper.h>

#include "nebula/base/id_util.h"
#include "nebula/base/map_util.h"

#include "nebula/net/base/nebula_pipeline.h"
#include "nebula/net/thread_local_conn_manager.h"
#include "nebula/net/engine/tcp_client_group.h"
#include "nebula/net/net_engine_manager.h"

#include "nebula/net/rpc/zrpc_client_handler.h"

std::map<int, ZRpcUtil::ServiceFunc> ZRpcUtil::g_rpc_services;
// static ProtoRpcResponsePtr kEmptyResponse;

folly::Future<ProtoRpcResponsePtr> ZRpcUtil::DoClientCall(const std::string& service_name, RpcRequestPtr request) {
  CHECK(request);
  
  // TODO(@benqi): 移入tcp_client_group_util.h里
  auto net_engine = nebula::NetEngineManager::GetInstance();
  // auto& conn_manager = nebula::GetConnManagerByThreadLocal();
  
  auto service = net_engine->Lookup(service_name);
  if (!service) {
    LOG(ERROR) << "Write - invalid error, not find service_name: " << service_name;
    return folly::makeFuture(std::make_shared<RpcInternalError>(request->message_id()));
  }
  
  auto group = std::static_pointer_cast<nebula::TcpClientGroupBase>(service);
  nebula::TcpClientGroupBase::OnlineTcpClient client;
  if (!group->GetOnlineClientByRandom(&client)) {
    LOG(ERROR) << "Write - invalid error, not online client's service_name: " << service_name;
    return folly::makeFuture(std::make_shared<RpcInternalError>(request->message_id()));
  }
  
  auto pipeline = client.second.lock();
  if (!pipeline) {
    LOG(ERROR) << "Write - invalid error, not online client's service_name: " << service_name;
    return folly::makeFuture(std::make_shared<RpcInternalError>(request->message_id()));
  }
  
  auto handler = dynamic_cast<ZRpcClientPipeline*>(pipeline.get())->getHandler<ZRpcClientHandler>();
  
  // TODO(@benqi): 线程同步
  return handler->ServiceCall(request);
}

void ZRpcUtil::Register(int method_id, ServiceFunc f) {
  if (ContainsKey(g_rpc_services, method_id)) {
    LOG(ERROR) << "Register - duplicate entry for method_id: " << method_id;
  } else {
    g_rpc_services.emplace(method_id, f);
  }
}

ProtoRpcResponsePtr ZRpcUtil::DoServiceCall(RpcRequestPtr request) {
  CHECK(request);
  
  auto it = g_rpc_services.find(request->method_id);
  if (it != g_rpc_services.end()) {
    auto r = (it->second)(request);
    r->set_message_id(GetNextIDBySnowflake());
    return r;
    //(it->second)(request);
  } else {
    LOG(ERROR) << "ServiceCall - Not register request: " << request->ToString();
    return std::make_shared<RpcInternalError>(request->message_id());
  }
}

