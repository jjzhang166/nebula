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

#include "nebula/net/handler/zproto/zproto_event_callback.h"

#include "nebula/base/logger/glog_util.h"

#if 0

/////////////////////////////////////////////////////////////////////////////////////////
ZProtoEventCallback::NewConnectionFunc ZProtoEventCallback::g_new_connection;
ZProtoEventCallback::DataReceivedFunc ZProtoEventCallback::g_data_received;
ZProtoEventCallback::ConnectionClosedFunc ZProtoEventCallback::g_connection_closed;

/////////////////////////////////////////////////////////////////////////////////////////
bool ZProtoEventCallback::Initializer::g_inited = false;

int ZProtoEventCallback::OnNewConnection(nebula::TcpServiceBase* service, nebula::ZProtoPipeline* pipeline) {
  if (g_new_connection) {
    return g_new_connection(service, pipeline);
  } else {
    return -1;
  }
}

int ZProtoEventCallback::OnDataReceived(nebula::ZProtoPipeline* pipeline, std::shared_ptr<PackageMessage> message_data) {
  if (g_data_received) {
    return g_data_received(pipeline, message_data);
  } else {
    return -1;
  }
}

int ZProtoEventCallback::OnConnectionClosed(nebula::TcpServiceBase* service, nebula::ZProtoPipeline* pipeline) {
  if (g_connection_closed) {
    return g_connection_closed(service, pipeline);
  } else {
    return -1;
  }
}

ZProtoEventCallback::Initializer::Initializer(NewConnectionFunc new_connection,
                                              DataReceivedFunc data_received,
                                              ConnectionClosedFunc connection_closed) {
  if (!g_inited) {
    g_new_connection = new_connection;
    g_data_received = data_received;
    g_connection_closed = connection_closed;
    g_inited = true;
  } else {
    LOG(ERROR) << "Initializer - ZProtoConnEvent inited!!!";
  }
}

#endif



