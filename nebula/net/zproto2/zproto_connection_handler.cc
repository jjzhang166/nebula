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

#include "nebula/net/zproto/zproto_connection_handler.h"

#include <limits>

#include <folly/Likely.h>
#include <folly/Format.h>

#include "nebula/base/func_factory_manager.h"

///////////////////////////////////////////////////////////////////////////////////////
// 初始化
typedef void(TcpTransportHandler::*ExecHandler)(TcpTransportHandler::Context*, std::shared_ptr<FrameMessage>);
typedef nebula::FuncFactoryManager<ExecHandler, uint8_t> ExecHandlerFactory;
#define REGISTER_EXECUTE_HANDLER(T) \
  static ExecHandlerFactory::RegisterTemplate g_reg_tcp_transport_handler_##T(T::HEADER, &TcpTransportHandler::On##T)

// Frame
REGISTER_EXECUTE_HANDLER(ProtoRawData);
//REGISTER_EXECUTE_HANDLER(Ping);
//REGISTER_EXECUTE_HANDLER(Pong);
//REGISTER_EXECUTE_HANDLER(Drop4Frame);
//REGISTER_EXECUTE_HANDLER(Redirect);
//REGISTER_EXECUTE_HANDLER(Ack);
//REGISTER_EXECUTE_HANDLER(Handshake);
//REGISTER_EXECUTE_HANDLER(HandshakeResponse);

void TcpTransportDecoder::read(Context* ctx, folly::IOBufQueue& q) {
  do {
    // 包不完整
    // 刚好一个完整包
    // 不止一个包
    int rv = decode(ctx, q, cached_frame_);
    if (rv == -1) {
      // 非完整frame
      break;
    } else {
      if (!OnFrameHandler(ctx, cached_frame_)) {
        break;
      }
    }
    
    // 刚好一个完整的frame
    if (rv == 0) {
      break;
    }
    
    // rv > 1
    // 还有frame，继续
  } while (1);
}

int TcpTransportDecoder::decode(Context* ctx, folly::IOBufQueue& buf, Frame& result) {
  if (buf.empty()) {
    return -1;
  }
  
  auto buf_length = buf.chainLength();
  
  if (result.body_length == 0) {
    // 先解码到body_length
    if (buf_length >= 9) {
      folly::io::Cursor c(buf.front());
      
      // 包索引/类型／body
      result.package_index = c.readBE<int32_t>();
      result.header = c.readBE<uint8_t>();
      result.body_length = c.readBE<int32_t>();
    } else {
      // TODO(@wubenqi): 考虑日志输出更详细的信息(conn_id/ip...)
      LOG(WARNING) << "decode - need 9 byte, but only recv len: " << buf_length;
      return -1;
    }
  }
  
  // 检查是否一个完整frame
  if (buf_length < result.CalcFrameLength()) {
    LOG(WARNING) << "decode - need " << result.CalcFrameLength()
                  << " byte, but only recv len: " << buf_length;
 
    return -1;
  }
  
  // nebula::io_buf_util::TrimStart(&buf, 9);
  buf.trimStart(9);
  auto d = buf.split(result.body_length);
  result.body = std::move(d);

  folly::io::Cursor c(buf.front());
  
  result.crc32 = c.readBE<int32_t>();
  buf.trimStart(4);

  if (buf_length - result.CalcFrameLength() == 0) {
    return 0;
  } else {
    return 1;
  }
}

bool TcpTransportDecoder::CheckPackageIndex(int package_index) {
  if (LIKELY(package_index-last_package_index_ == 1)) {
    // last_package_index_ = package_index;
    return true;
  } else {
    if (last_package_index_ == std::numeric_limits<int>::max()) {
      if (package_index == 0) {
        // last_package_index = 0;
        return true;
      } else {
        return false;
      }
    } else {
      return false;
    }
  }
}
//  header无法识别，忽略
bool TcpTransportDecoder::OnFrameHandler(Context* ctx, Frame& frame) {
  // packageIndex is broken，要断开连接
  if (false == CheckPackageIndex(frame.package_index)) {
    LOG(ERROR) << "OnFrameHandler - Check package_index invalid, last_package_index: "
                << last_package_index_ << ", recved frame: " << frame.ToString();
    ctx->fireReadException(folly::make_exception_wrapper<std::runtime_error>("OnFrameHandler - Check package_index invalid"));
    return false;
  }
  
  // TODO(@wubenqi): CRC32校验错误，要断开连接
  // int crc32 = CRC();
  // if (crc32 != frame.crc32) {
  // LOG(ERROR) << "OnFrameHandler - crc32 error, crc32: "
  //                << crc32 << ", recved frame: " << frame.ToString();
  // ctx->fireReadException(folly::make_exception_wrapper<std::runtime_error>("OnFrameHandler - crc32 check error"));
  //   return false;
  // }
  
  auto frame_data = FrameFactory::CreateSharedInstance(frame.header);
  if (frame_data) {
    if (frame_data->Decode(frame)) {
      ctx->fireRead(frame_data);
    } else {
      ctx->fireReadException(folly::make_exception_wrapper<std::runtime_error>("OnFrameHandler - Decode package error"));
      return false;
    }
  }

  // 设置下一步数据
  if (UNLIKELY(frame.package_index == std::numeric_limits<int>::max())) {
    last_package_index_ = 0;
  } else {
    last_package_index_ = frame.package_index;
  }
  
  frame.body_length = 0;
  return true;
}

void TcpTransportHandler::read(Context* ctx, std::shared_ptr<FrameMessage> msg) {
  ExecHandlerFactory::Execute2<TcpTransportHandler>(this, msg->GetFrameType(), ctx, msg);
}

void TcpTransportHandler::OnProtoRawData(Context* ctx, std::shared_ptr<FrameMessage> frame_data) {
  auto message_data = std::static_pointer_cast<ProtoRawData>(frame_data);
  ctx->fireRead(message_data);
}

void TcpTransportHandler::OnPing(Context* ctx, std::shared_ptr<FrameMessage> frame_data) {
  // 直接返回
}

void TcpTransportHandler::OnPong(Context* ctx, std::shared_ptr<FrameMessage> frame_data) {
  //
}

void TcpTransportHandler::OnDrop4Frame(Context* ctx, std::shared_ptr<FrameMessage> frame_data) {
  // close连接
}

void TcpTransportHandler::OnRedirect(Context* ctx, std::shared_ptr<FrameMessage> frame_data) {
  //
}

void TcpTransportHandler::OnAck(Context* ctx, std::shared_ptr<FrameMessage> frame_data) {
  //
}

void TcpTransportHandler::OnHandshake(Context* ctx, std::shared_ptr<FrameMessage> frame_data) {
  //
}

void TcpTransportHandler::OnHandshakeResponse(Context* ctx, std::shared_ptr<FrameMessage> frame_data) {
  //
}

