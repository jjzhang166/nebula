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

#include "nebula/net/zproto/zproto_level_data.h"

#include <folly/Format.h>

#include "nebula/base/self_register_factory_manager.h"

using namespace nebula;

// 模板不是类型, typedef只能给类型取别名。
// using关键字可以给模板取别名
template <class T>
using FrameRegister = SelfRegisterFactoryManager<FrameMessage, uint8_t>::RegisterTemplate<T>;

#define REGISTER_FRAME(T) \
  const uint8_t T::HEADER; \
  static FrameRegister<T> g_reg_frame_##T(T::HEADER)
#define REGISTER_PACKAGE(T) \
  static PackageFactoryManager::RegisterTemplate<T> g_reg_package_##T(T)
#define REGISTER_BASIC_SYNC(T) \
  static BasicSyncFactoryManager::RegisterTemplate<T> g_reg_basic_sync_##T(T)

// Frame
REGISTER_FRAME(ProtoRawData);
//REGISTER_FRAME(Ping);
//REGISTER_FRAME(Pong);
//REGISTER_FRAME(Drop4Frame);
//REGISTER_FRAME(Redirect);
//REGISTER_FRAME(Ack);
//REGISTER_FRAME(Handshake);
//REGISTER_FRAME(HandshakeResponse);

// Package
//REGISTER_PACKAGE(ProtoRpcRequest)
//REGISTER_PACKAGE(ProtoRpcResponse)
//REGISTER_PACKAGE(ProtoPush)
//REGISTER_PACKAGE(MessageAck)
//REGISTER_PACKAGE(UnsentMessage)
//REGISTER_PACKAGE(UnsentResponse)
//REGISTER_PACKAGE(NewSession)
//REGISTER_PACKAGE(SessionHello)
//REGISTER_PACKAGE(SessionLost)
//REGISTER_PACKAGE(Container)

// const uint8_t ProtoRawData::HEADER; // = Frame::HEADER_PROTO;

std::string Frame::ToString() const {
  return folly::sformat("{{package_index:{}, header:{}, body_length:{}, crc32:{}}}",
                        package_index,
                        header,
                        body_length,
                        crc32);
}



// FrameRegister<ProtoRawData> g_reg_frame_1(ProtoRawData::HEADER);

/*
// Package
REGISTER_PACKAGE(PlainTextMessage, 1);
REGISTER_PACKAGE(EncryptedMessage, 2);

REGISTER_PACKAGE(Drop4Package, 3);
REGISTER_PACKAGE(RequestAuthId, 0xF0);
REGISTER_PACKAGE(ResponseAuthId, 0xF1);
REGISTER_PACKAGE(Handshake, 0xFF);
REGISTER_PACKAGE(HandshakeResponse, 0xFE);
*/

bool Frame::Decode(std::unique_ptr<folly::IOBuf> frame_data) {
  folly::io::Cursor c(frame_data.get());
  
  try {
    package_index = c.readBE<int32_t>();
    header = c.readBE<uint8_t>();
    
    body_length = c.readBE<int32_t>();
    c.skip(body_length);
    crc32 = c.readBE<int32_t>();
    
    // frame_data->trimStart(9);
    body.swap(frame_data);
    body->trimStart(9);
    nebula::io_buf_util::TrimStart(body.get(), 9);
    nebula::io_buf_util::TrimEnd(body.get(), 4);
  } catch (...) {
    // TODO(@wubenqi): error's log
    return false;
  }
  
  return true;
  
/*
  auto buf_length = buf.chainLength();
  
  if (result.body_length == 0) {
    // 先解码到body_length
    if (buf_length >= 9) {
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
  
  buf.trimStart(9);
  auto d = buf.split(result.body_length);
  result.body = std::move(d);
  
  folly::io::Cursor c(buf.front());
  result.crc32 = c.readBE<int32_t>();
  buf.trimStart(4);
 */
}


bool BasicSyncMessage::SerializeToIOBuf(std::unique_ptr<folly::IOBuf>& io_buf) const {
  bool rv = true;
  auto io_buf2 = folly::IOBuf::create(2000);
  // io_buf2->trimStart(4);
  IOBufWriter iobw(io_buf2.get(), 2000);
  try {
    // PackageSize
    // auto body_len = CalcBodySize() + CalcPackageHeaderSize();

    // auto body_len = body ? static_cast<uint32_t>(body->computeChainDataLength()) : 0;
    uint32_t buf_len = CalcBasicSyncMessageSize() + CalcPackageHeaderSize();
    
    //cuint32_t
    // Frame
    iobw.writeBE((int)0);
    iobw.writeBE(GetFrameType());
    iobw.writeBE(buf_len);
    
    iobw.writeBE(package_header.auth_id);
    iobw.writeBE(package_header.session_id);
    iobw.writeBE(GetPackageType());
    
    // iobw.writeBE(zproto_header);
    iobw.writeBE(GetZProtoType());
    iobw.writeBE(zproto_header.message_id);
    
    iobw.writeBE(GetBasicSyncType());
    
    Encode(iobw);
    
    // crc32
    iobw.writeBE(uint32_t(0));

    // 返回值
    io_buf = std::move(io_buf2);
    
  } catch(const std::exception& e) {
    LOG(ERROR) << "SerializeToIOBuf - catch a threwn exception: " << folly::exceptionStr(e);
    rv = false;
  } catch (...) {
    LOG(ERROR) << "SerializeToIOBuf - catch a unknown threwn exception";
    rv = false;
  }
  return rv;
}
