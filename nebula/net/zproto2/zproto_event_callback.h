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

#ifndef NEBULA_NET_ZPROTO_ZPROTO_EVENT_CALLBACK_H_
#define NEBULA_NET_ZPROTO_ZPROTO_EVENT_CALLBACK_H_

#include "nebula/net/base/nebula_pipeline.h"
#include "nebula/net/server/tcp_service_base.h"
#include "nebula/net/zproto/zproto_level_data.h"

/////////////////////////////////////////////////////////////////////////////////////////
struct ZProtoEventCallback {
  using NewConnectionFunc = std::function<int(nebula::TcpServiceBase*, NebulaPipelinePtr)>;
  using DataReceivedFunc = std::function<int(NebulaPipelinePtr, std::shared_ptr<BasicSyncMessage>)>;
  using ConnectionClosedFunc = std::function<int(nebula::TcpServiceBase*, NebulaPipelinePtr)>;
  
  static void Initializer(NewConnectionFunc new_connection,
                          DataReceivedFunc data_received,
                          ConnectionClosedFunc connection_closed) {
    g_new_connection = new_connection;
    g_data_received = data_received;
    g_connection_closed = connection_closed;
  }
  
  ///////////////////////////////////////////////////////////////////////////////////////
  static int OnNewConnection(nebula::TcpServiceBase* service, NebulaPipelinePtr pipeline);
  static int OnDataReceived(NebulaPipelinePtr pipeline, std::shared_ptr<BasicSyncMessage> message_data);
  static int OnConnectionClosed(nebula::TcpServiceBase* service, NebulaPipelinePtr pipeline);

  ///////////////////////////////////////////////////////////////////////////////////////
  static NewConnectionFunc g_new_connection;
  static DataReceivedFunc g_data_received;
  static ConnectionClosedFunc g_connection_closed;
  
  ///////////////////////////////////////////////////////////////////////////////////////
  struct Initializer {
    Initializer(NewConnectionFunc new_connection,
                DataReceivedFunc data_received,
                ConnectionClosedFunc connection_closed);
    static bool g_inited;
  };
};

#endif
