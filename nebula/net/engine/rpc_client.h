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

#ifndef NEBULA_NET_ENGINE_RPC_CLIENT_H_
#define NEBULA_NET_ENGINE_RPC_CLIENT_H_

#include "nebula/net/engine/tcp_client.h"

namespace nebula {
  
template <typename Pipeline = DefaultPipeline>
class RpcClient : public TcpClient<Pipeline> {
public:
  RpcClient(const ServiceConfig& config, const IOThreadPoolExecutorPtr& io_group)
    : TcpClient<Pipeline>(config, io_group) {
  }

  virtual ~RpcClient() = default;
  
  // Impl from TcpServiceBase
  ServiceModuleType GetModuleType() const override {
    return ServiceModuleType::RPC_CLIENT;
  }
};
  
}

#endif


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

#ifndef NEBULA_NET_RPC_ZRPC_CLIENT_H_
#define NEBULA_NET_RPC_ZRPC_CLIENT_H_

#if 0
#include <gflags/gflags.h>

#include <wangle/service/Service.h>
#include <wangle/service/ExpiringFilter.h>
#include <wangle/service/ClientDispatcher.h>
#include <wangle/bootstrap/ClientBootstrap.h>
#include <wangle/channel/AsyncSocketHandler.h>
#include <wangle/codec/LengthFieldBasedFrameDecoder.h>
#include <wangle/codec/LengthFieldPrepender.h>
#include <wangle/channel/EventBaseHandler.h>

#include <wangle/example/rpc/ClientSerializeHandler.h>

using namespace folly;
using namespace wangle;

using thrift::test::Bonk;
using thrift::test::Xtruct;

DEFINE_int32(port, 8080, "test server port");
DEFINE_string(host, "::1", "test server address");


int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  
  /**
   * For specific protocols, all the following code would be wrapped
   * in a protocol-specific ServiceFactories.
   *
   * TODO: examples of ServiceFactoryFilters, for connection pooling, etc.
   */
  ClientBootstrap<SerializePipeline> client;
  client.group(std::make_shared<wangle::IOThreadPoolExecutor>(1));
  client.pipelineFactory(std::make_shared<RpcPipelineFactory>());
  auto pipeline = client.connect(SocketAddress(FLAGS_host, FLAGS_port)).get();
  // A serial dispatcher would assert if we tried to send more than one
  // request at a time
  // SerialClientDispatcher<SerializePipeline, Bonk> service;
  // Or we could use a pipelined dispatcher, but responses would always come
  // back in order
  // PipelinedClientDispatcher<SerializePipeline, Bonk> service;
  auto dispatcher = std::make_shared<BonkMultiplexClientDispatcher>();
  dispatcher->setPipeline(pipeline);
  
  // Set an idle timeout of 5s using a filter.
  ExpiringFilter<Bonk, Xtruct> service(dispatcher, std::chrono::seconds(5));
  
  try {
    while (true) {
      std::cout << "Input string and int" << std::endl;
      
      Bonk request;
      std::cin >> request.message;
      std::cin >> request.type;
      service(request).then([request](Xtruct response) {
        CHECK(request.type == response.i32_thing);
        std::cout << response.string_thing << std::endl;
      });
    }
  } catch (const std::exception& e) {
    std::cout << exceptionStr(e) << std::endl;
  }
  
  return 0;
}
#endif

#endif
