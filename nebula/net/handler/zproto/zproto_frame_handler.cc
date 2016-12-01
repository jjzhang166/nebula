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

#include "nebula/net/handler/zproto/zproto_frame_handler.h"

#include <limits>

#include <folly/Likely.h>
#include <folly/Format.h>

#include "nebula/base/func_factory_manager.h"

///////////////////////////////////////////////////////////////////////////////////////
// 初始化
typedef void(ZProtoFrameHandler::*ExecHandler)(ZProtoFrameHandler::Context*, std::shared_ptr<FrameMessage>);
typedef nebula::FuncFactoryManager<ExecHandler, uint8_t> ExecHandlerFactory;
#define REGISTER_EXECUTE_HANDLER(T) \
  static ExecHandlerFactory::RegisterTemplate g_reg_tcp_transport_handler_##T(T::HEADER, &ZProtoFrameHandler::On##T)

// Frame
REGISTER_EXECUTE_HANDLER(ProtoRawData);
REGISTER_EXECUTE_HANDLER(Ping);
REGISTER_EXECUTE_HANDLER(Pong);
REGISTER_EXECUTE_HANDLER(Drop);
REGISTER_EXECUTE_HANDLER(Redirect);
REGISTER_EXECUTE_HANDLER(Ack);
REGISTER_EXECUTE_HANDLER(Handshake);
REGISTER_EXECUTE_HANDLER(HandshakeResponse);

void ZProtoFrameDecoder::read(Context* ctx, folly::IOBufQueue& q) {
  do {
    // 包不完整
    // 刚好一个完整包
    // 不止一个包
    int rv = decode(ctx, q, cached_frame_);
    if (rv < 0) {
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

int ZProtoFrameDecoder::decode(Context* ctx, folly::IOBufQueue& buf, Frame& result) {
  if (buf.empty()) {
    return -1;
  }
  
  auto buf_length = buf.chainLength();
  
  if (result.body_length == 0) {
    // 先解码到body_length
    if (buf_length >= Frame::HEADER_LEN) {
      folly::io::Cursor c(buf.front());
      
      // 包索引/类型／body
      result.magic_number = c.readBE<uint16_t>();
      if (result.magic_number != 0x5342) {
        LOG(ERROR) << "decode - Check magic_number invalid, 0x5342 != magic_number: " << result.magic_number;
        ctx->fireReadException(folly::make_exception_wrapper<std::runtime_error>("decode - Check magic_number invalid"));
        return -2;
      }
      result.frame_index = c.readBE<uint16_t>();
      // packageIndex is broken，要断开连接
      if (false == CheckPackageIndex(result.frame_index)) {
        LOG(ERROR) << "decode - Check package_index invalid, last_frame_index: "
                    << last_frame_index_ << ", recved frame: " << result.ToString();
        ctx->fireReadException(folly::make_exception_wrapper<std::runtime_error>("decode - Check package_index invalid"));
        return -3;
      }

      uint32_t tmp = c.readBE<uint32_t>();
      
      // TODO(@benqi): 检查frame_type
      result.frame_type = tmp >> 24;
      
      result.body_length = tmp & 0xffffff;
      // TODO(@benqi): 使用宏或配置文件
      if (result.body_length > MAX_FRAME_BODY_LEN) {
         LOG(ERROR) << "decode - Invalid body length(>1MB), recved frame: " << result.ToString();
        ctx->fireReadException(folly::make_exception_wrapper<std::runtime_error>("decode - Check body length invalid"));
        return -5;
      }

    } else {
      // TODO(@wubenqi): 考虑日志输出更详细的信息(conn_id/ip...)
      LOG(WARNING) << "decode - need 8 byte, but only recv len: " << buf_length;
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
  buf.trimStart(Frame::HEADER_LEN);
  auto d = buf.split(result.body_length);
  result.body = std::move(d);

  folly::io::Cursor c(buf.front());
  
  result.crc32 = c.readBE<int32_t>();
  buf.trimStart(sizeof(result.crc32));

  if (buf_length == result.CalcFrameLength()) {
    return 0;
  } else {
    return 1;
  }
}

bool ZProtoFrameDecoder::CheckPackageIndex(uint16_t frame_index) {
  //
  return frame_index-last_frame_index_ == 1 ||
        (last_frame_index_ == std::numeric_limits<uint16_t>::max() &&
         frame_index == 0);
}
//  header无法识别，忽略
bool ZProtoFrameDecoder::OnFrameHandler(Context* ctx, Frame& frame) {
  // TODO(@benqi): CRC32校验错误，要断开连接
  // int crc32 = CRC();
  // if (crc32 != frame.crc32) {
  // LOG(ERROR) << "OnFrameHandler - crc32 error, crc32: "
  //                << crc32 << ", recved frame: " << frame.ToString();
  // ctx->fireReadException(folly::make_exception_wrapper<std::runtime_error>("OnFrameHandler - crc32 check error"));
  //   return false;
  // }
  
  auto frame_message = FrameFactory::CreateSharedInstance(frame.frame_type);
  if (frame_message) {
    if (frame_message->Decode(frame) &&
        // 检查解压包的长度是否一致，避免格式一样，
        // 但数据长度不一样(FrameMessage里有string等字段，可能会有解压后长度与body长度不一致情况)
        frame_message->CalcFrameSize() == frame.CalcFrameLength()) {
      ctx->fireRead(frame_message);
    } else {
      LOG(ERROR) << "OnFrameHandler - Decode FrameMessage error " << frame.ToString();
      ctx->fireReadException(folly::make_exception_wrapper<std::runtime_error>("OnFrameHandler - Decode FrameMessage error"));
      return false;
    }
  }

  // 设置下一步数据
  if (UNLIKELY(frame.frame_index == std::numeric_limits<uint16_t>::max())) {
    last_frame_index_ = 0;
  } else {
    last_frame_index_ = frame.frame_index;
  }
  
  frame.body_length = 0;
  return true;
}

void ZProtoFrameHandler::read(Context* ctx, std::shared_ptr<FrameMessage> msg) {
  ExecHandlerFactory::Execute2<ZProtoFrameHandler>(this, msg->GetFrameType(), ctx, msg);
}

void ZProtoFrameHandler::OnProtoRawData(Context* ctx, std::shared_ptr<FrameMessage> message) {
  CAST_PROTO_MESSAGE(ProtoRawData, message_data);
  ctx->fireRead(message_data);
}

void ZProtoFrameHandler::OnPing(Context* ctx, std::shared_ptr<FrameMessage> message) {
  CAST_PROTO_MESSAGE(Ping, ping);
  // 直接返回
  Pong pong;
  pong.random_bytes.swap(ping->random_bytes);
  WriteFrameMessage(ctx, &pong);
}

void ZProtoFrameHandler::OnPong(Context* ctx, std::shared_ptr<FrameMessage> message) {
  CAST_PROTO_MESSAGE(Pong, pong);
  
  // TODO(@benqi): 调试环境开启
  LOG(INFO) << "OnPong - recv pong";
}

void ZProtoFrameHandler::OnDrop(Context* ctx, std::shared_ptr<FrameMessage> message) {
  CAST_PROTO_MESSAGE(Drop, drop);
  
  // TODO(@benqi): 调试环境开启
  LOG(INFO) << "OnDrop - recv drop";
  
  // 关闭连接
  // TODO(@benqi)
  ctx->fireReadException(folly::make_exception_wrapper<std::runtime_error>("OnDrop - recv drop packet, will close"));
}

void ZProtoFrameHandler::OnRedirect(Context* ctx, std::shared_ptr<FrameMessage> message) {
  CAST_PROTO_MESSAGE(Redirect, redirect);
  
  LOG(INFO) << "OnRedirect - recv redirect";
  
}

void ZProtoFrameHandler::OnAck(Context* ctx, std::shared_ptr<FrameMessage> message) {
  CAST_PROTO_MESSAGE(Ack, ack);

  // TODO(@benqi): 调试环境开启
  LOG(INFO) << "OnAck - recv ack";

  // TODO(@benqi): process ack，检测连接状态
}

void ZProtoFrameHandler::OnHandshake(Context* ctx, std::shared_ptr<FrameMessage> message) {
  CAST_PROTO_MESSAGE(Handshake, handshake);

  // TODO(@benqi): 调试环境开启
  LOG(INFO) << "OnHandshake - recv handshake";
  
  HandshakeResponse handshake_response;
  handshake_response.proto_revision = handshake->proto_revision;
  handshake_response.api_major_version = handshake->api_major_version;
  handshake_response.api_minor_version = handshake->api_minor_version;
  memcpy(handshake_response.sha1, handshake->random_bytes, 32);
  
  WriteFrameMessage(ctx, &handshake_response);
}

void ZProtoFrameHandler::OnHandshakeResponse(Context* ctx, std::shared_ptr<FrameMessage> message) {
  CAST_PROTO_MESSAGE(HandshakeResponse, handshake_response);
  
  // TODO(@benqi): 调试环境开启
  LOG(INFO) << "OnHandshakeResponse - recv handshake_response";
}

void ZProtoFrameHandler::WriteFrameMessage(Context *ctx, const FrameMessage* message) {
  std::unique_ptr<folly::IOBuf> io_buf;
  if (!message->SerializeToIOBuf(io_buf)) {
    LOG(ERROR) << "WriteFrameMessage - SerializeToIOBuf message error!!!";
    return;
  }
  
  write(ctx, std::move(io_buf));
}

folly::Future<folly::Unit> ZProtoFrameHandler::write(Context* ctx, std::unique_ptr<folly::IOBuf> msg) {
  uint16_t send_frame_index = ++last_send_frame_index_;
  WriteFrameIndex(send_frame_index, msg.get());
  
  return ctx->fireWrite(std::forward<std::unique_ptr<folly::IOBuf>>(msg));
}

