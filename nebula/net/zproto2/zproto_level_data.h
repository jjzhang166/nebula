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

#ifndef NUBULA_NET_ZPROTO_LEVEL_DATA_H_
#define NUBULA_NET_ZPROTO_LEVEL_DATA_H_

#include <list>

#include "nebula/base/io_buf_util.h"
#include "nebula/base/self_register_factory_manager.h"

// using
/*
/////////////////////////////////////////////////////////////////////////////////////
// Frame: FrameRawData(folly::IOBuf->Frame)

// Package: PackageRawData(PackageRawData -> Package)
struct PackageRawData {
  uint8 frame_header { 0x00 };
  std::unique_ptr<folly::IOBuf> body;
};
using PackageRawDataPtr = std::shared_ptr<PackageRawData>;

// BasicSyncRawData
struct BasicSyncRawData {
  uint8 frame_header { 0x00 };
  
  // unique identifier that is constant thru all application lifetime
  int64_t auth_id;
  // random identifier of current session
  int64_t session_id;
  // message header
  int32 message_header {0x01};
  
  // BasicSync头
  int32 sync_header; // REQ/RSP/PUSH/ACK/
  int64_t message_id;
  
  std::unique_ptr<folly::IOBuf> body;
};
using BasicSyncRawDataPtr = std::shared_ptr<BasicSyncRawData>;
*/

//struct FrameHeader {
//  uint32_t index;
//  uint8_t  header;
//  uint32_t package_len;
//  uint32_t crc32;
//};

struct FrameHeader {
  uint8_t frame_type;
};

//////////////////////////////////////////////////////////////////////
// Connection Level
struct Frame {
  enum FrameType {
    PROTO = 0,
    PING = 1,
    PONG = 2,
    DROP = 3,
    REDIRECT = 4,
    ACK = 6,
    HANDSHAKE = 0xFF,
    HANDSHAKE_RESPONSE = 0xFE,
  };

  const static uint32_t kHeaderLength = 9;
  const static uint32_t kTailLength = 4;

  inline uint32_t CalcFrameLength() const {
    return kHeaderLength + kTailLength + body_length;
  }

  bool Decode(std::unique_ptr<folly::IOBuf> frame_data);
  
  std::string ToString() const;
  
  // Index of package starting from zero.
  // If packageIndex is broken connection need to be dropped.
  int32_t package_index;
  // Type of message
  uint8_t header;
  // Package payload length
  int32_t body_length {0};
  // Package payload
  std::unique_ptr<folly::IOBuf> body;
  
  // CRC32 of body
  int32_t crc32;
};

struct FrameMessage {
  virtual ~FrameMessage() = default;
  
  virtual uint8_t GetFrameType() const = 0;
  virtual bool Decode(Frame& frame) { return true; }
  // virtual bool SerializeToIOBuf(folly::IOBuf* buf) const = 0;
};

// HEADER_PROTO = 0;
struct ProtoRawData : FrameMessage {
  static const uint8_t HEADER = Frame::PROTO;
  
  uint8_t GetFrameType() const override {
    return HEADER;
  }

  // Impl from FrameMessage
  bool Decode(Frame& frame) override {
    message_data.swap(frame.body);
    return true;
  }
  
  std::unique_ptr<folly::IOBuf> message_data;
};


/*
// header = 1
// Ping message can be sent from both sides
struct Ping : public FrameMessage {
  static const uint8_t HEADER = Frame::HEADER_PING;

  uint8_t GetFrameType() const override {
    return HEADER;
  }
  
  bool Decode(std::unique_ptr<folly::IOBuf> body) override {
    random_bytes = std::move(body);

    return true;
  }
  
  std::unique_ptr<folly::IOBuf> random_bytes;
};

// header = 2
// Pong message need to be sent immediately after receving Ping message
struct Pong : public FrameMessage {
  static const uint8_t HEADER = Frame::HEADER_PONG;

  uint8_t GetFrameType() const override {
    return HEADER;
  }
  
  bool Decode(std::unique_ptr<folly::IOBuf> body) override {
    random_bytes = std::move(body);
    
    return true;
  }
  
  // Same bytes as in Ping package
  std::unique_ptr<folly::IOBuf> random_bytes;
};

// header = 3
// Notification about connection drop
struct Drop4Frame : public FrameMessage {
  static const uint8_t HEADER = Frame::HEADER_DROP;

  uint8_t GetFrameType() const override {
    return HEADER;
  }
  
  bool Decode(std::unique_ptr<folly::IOBuf> body) override {
    try {
      folly::io::Cursor c(body.get());
      
      message_id = c.readBE<int64_t>();
      error_code = c.readBE<uint8_t>();
      int l = c.readBE<int>();
      error_message = c.readFixedString(l);
    } catch(...) {
      // TODO(@wubenqi): error's log
      return false;
    }
    return true;
  }
  
  // Same bytes as in Ping package
  int64_t message_id;
  uint8_t error_code;
  std::string error_message;
};

// header = 4
// Sent by server when we need to temporary redirect to another server
struct Redirect : public FrameMessage {
  static const uint8_t HEADER = Frame::HEADER_REDIRECT;

  uint8_t GetFrameType() const override {
    return HEADER;
  }
  
  bool Decode(std::unique_ptr<folly::IOBuf> body) override {
    try {
      folly::io::Cursor c(body.get());

      int l = c.readBE<int32_t>();
      host = c.readFixedString(l);
      port = c.readBE<int>();
      timeout = c.readBE<int>();
    } catch(...) {
      // TODO(@wubenqi): error's log
      return false;
    }
    return true;
  }

  std::string host;
  int port;
  // Redirection timeout
  int timeout;
};

// header = 6
// Proto package is received by destination peer. Used for determening of connection state
struct Ack : public FrameMessage {
  static const uint8_t HEADER = Frame::HEADER_ACK;

  uint8_t GetFrameType() const override {
    return HEADER;
  }
  
  bool Decode(std::unique_ptr<folly::IOBuf> body) override {
    try {
      folly::io::Cursor c(body.get());
      
      received_package_index = c.readBE<int>();
    } catch(...) {
      // TODO(@wubenqi): error's log
      return false;
    }
    return true;
  }
  
  int received_package_index;
};

// header == 0xFF
struct Handshake : public FrameMessage {
  static const uint8_t HEADER = Frame::HEADER_HANDSHAKE;

  uint8_t GetFrameType() const override {
    return HEADER;
  }
  
  bool Decode(std::unique_ptr<folly::IOBuf> body) override {
    try {
      folly::io::Cursor c(body.get());
      
      proto_revision = c.readBE<uint8_t>();
      api_major_version = c.readBE<uint8_t>();
      api_minor_version = c.readBE<uint8_t>();
      c.pull(random_bytes, 32);
    } catch(...) {
      // TODO(@wubenqi): error's log
      return false;
    }
    return true;
  }

  // Current MTProto revision
  // For Rev 2 need to eq 1
  uint8_t proto_revision;
  // API Major and Minor version
  uint8_t api_major_version;
  uint8_t api_minor_version;
  
  // Some Random Bytes (suggested size is 32 bytes)
  uint8_t random_bytes[32];
};

// header == 0xFE
struct HandshakeResponse : public FrameMessage {
  static const uint8_t HEADER = Frame::HEADER_HANDSHAKE_RESPONSE;

  uint8_t GetFrameType() const override {
    return HEADER;
  }
  
  bool Decode(std::unique_ptr<folly::IOBuf> body) override {
    try {
      folly::io::Cursor c(body.get());
      
      proto_revision = c.readBE<uint8_t>();
      api_major_version = c.readBE<uint8_t>();
      api_minor_version = c.readBE<uint8_t>();
      c.pull(sha1, 32);
    } catch(...) {
      // TODO(@wubenqi): error's log
      return false;
    }
    return true;
  }
  
  // return same versions as request, 0 - version is not supported
  uint8_t proto_revision;
  uint8_t api_major_version;
  uint8_t api_minor_version;
  
  // SHA256 of randomBytes from request
  uint8_t sha1[32];
};
*/

//////////////////////////////////////////////////////////////////////
struct PackageHeader {
  // FrameHeader frame_header;
  // uint8_t frame_type {Frame::HEADER_PROTO};
  // unique identifier that is constant thru all application lifetime
  int64_t auth_id;
  // random identifier of current session
  int64_t session_id;
  // message header
  // uint8_t package_type;
};

// Transport Level
struct Package {
  enum PackageType {
    PLAIN_TEXT_MESSAGE = 1,
    ENCRYPTED_MESSAGE = 2,
    DROP = 3,
    AUTHIDINVALID = 4,
    REQUEST_AUTH_ID = 0xF0,
    RESPONSE_AUTH_ID = 0xF1,
    REQUEST_START_AUTH = 0xE0,
    RESPONSE_START_AUTH = 0xE1,
    REQUEST_GET_SERVER_KEY = 0xE2,
    RESPONSE_GET_SERVER_KEY = 0xE3,
    REQUEST_DH = 0xE6,
    RESPONSE_DO_DH = 0xE7,
  };

  const static uint32_t kHeaderLength = 17; //
  // const static uint32_t kTailLength = 4;

  bool Decode(ProtoRawData& proto_raw_data) {
    // frame_header = proto_raw_data.GetFrameType();
    message.swap(proto_raw_data.message_data);
    try {
      folly::io::Cursor c(message.get());
      
      package_header.auth_id = c.readBE<int64_t>();
      package_header.session_id = c.readBE<int64_t>();
      package_type = c.readBE<uint8_t>();
      
      // c.skip(sizeof(int32_t));
      nebula::io_buf_util::TrimStart(message.get(), kHeaderLength);
      // message->trimStart(20);
      
      // message.swap(package_data.body);
      // message = std::move(body);
    } catch(...) {
      // TODO(@wubenqi): error's log
      return false;
    }
    // message
    
    return true;
  }
  
  PackageHeader package_header;
  uint8_t package_type;

  // message contents
  // message: PlainTextMessage/EncryptedMessage/Drop/AuthIdInvalid
  std::unique_ptr<folly::IOBuf> message;
};

struct PackageMessage {
  virtual ~PackageMessage() = default;
  
  void set_auth_id(int64_t v) {
    package_header.auth_id = v;
  }

  void set_session_id(int64_t v) {
    package_header.session_id = v;
  }

  // Imp from FrameMessage
  virtual uint8_t GetFrameType() const {
    return Frame::PROTO;
  }
  
  virtual uint8_t GetPackageType() const = 0;
  
  virtual bool Decode(Package& package) {
    package_header = package.package_header;
    
    // auth_id = package.auth_id;
    // session_id = package.session_id;
    return true;
  }
//  uint8_t GetFrameType() const override {
//    return HEADER;
//  }

  PackageHeader package_header;
  // uint8_t package_type;

  // unique identifier that is constant thru all application lifetime
  // int64_t auth_id;
  // random identifier of current session
  // int64_t session_id;
  // message header
  // int32_t message_header;
};

// Plain Text message Container
struct PlainTextMessage : public PackageMessage {
  // HEADER = 0x01
  static const uint8_t HEADER = Package::PLAIN_TEXT_MESSAGE;
  
  uint8_t GetPackageType() const override {
    return HEADER;
  }

  bool Decode(Package& package) override {
    PackageMessage::Decode(package);
    message.swap(package.message);
    try {
      folly::io::Cursor c(message.get());
      message_id = c.read<int64_t>();
      nebula::io_buf_util::TrimStart(message.get(), 8);

      // package.message->skip(sizeof(int32_t));
      // message->trimStart8;
    } catch(...) {
      // TODO(@wubenqi): error's log
      return false;
    }
    return true;
  }

  // message identifier
  int64_t message_id;
  // message body
  std::unique_ptr<folly::IOBuf> message;
};

/*
// Encrypted message container
struct EncryptedMessage : public PackageMessage {
  static const uint8_t HEADER = Package::HEADER_ENCRYPTED_MESSAGE;
  
  int32_t GetMessageHeader() const override {
    return HEADER;
  }

  bool Decode(std::unique_ptr<folly::IOBuf> body) override {
    try {
      folly::io::Cursor c(body.get());
      seq_number = c.read<int64_t>();
      
    } catch(...) {
      // TODO(@wubenqi): error's log
      return false;
    }
    return true;
  }

  // HEADER = 0x02
  // Sequence number starting from zero for each direction
  int64_t seq_number;
  // First encryption level
  std::unique_ptr<folly::IOBuf> encrypted_package;
};
  
// Drop Container
struct Drop4Package : public PackageMessage {
  static const uint8_t HEADER = Package::HEADER_DROP;
  
  int32_t GetMessageHeader() const override {
    return HEADER;
  }

  bool Decode(std::unique_ptr<folly::IOBuf> body) override {
    try {
      folly::io::Cursor c(body.get());
      message_id = c.read<int64_t>();

      int l = c.readBE<int32_t>();
      error_tag = c.readFixedString(l);

    } catch(...) {
      // TODO(@wubenqi): error's log
      return false;
    }
    return true;
  }

  // HEADER = 0x03
  // Message Id of message that causes Drop. May be zero if not available
  int64_t message_id;
  // Error Tag
  std::string error_tag;
};

struct RequestAuthId : public PackageMessage {
  // HEADER = 0xF0
  static const uint8_t HEADER = Package::HEADER_REQUEST_AUTH_ID;
  
  int32_t GetMessageHeader() const override {
    return HEADER;
  }

  bool Decode(std::unique_ptr<folly::IOBuf> body) override {
    return true;
  }
};
  
struct ResponseAuthId : public PackageMessage {
  static const uint8_t HEADER = Package::HEADER_RESPONSE_AUTH_ID;
  
  int32_t GetMessageHeader() const override {
    return HEADER;
  }

  bool Decode(std::unique_ptr<folly::IOBuf> body) override {
    try {
      folly::io::Cursor c(body.get());
      auth_id = c.readBE<int64_t>();
    } catch(...) {
      // TODO(@wubenqi): error's log
      return false;
    }
    return true;
  }

  // HEADER = 0xF1
  int64_t auth_id;
};

///////////////////////////////////////////////////////////////////////////////////
// Authentication Key

// Before start Client MUST send RequestStartAuth message:
struct RequestStartAuth : public PackageMessage {
  // HEADER = 0xE0
  static const uint8_t HEADER = Package::HEADER_RESPONSE_START_AUTH;
  
  int32_t GetMessageHeader() const override {
    return HEADER;
  }
  
  bool Decode(std::unique_ptr<folly::IOBuf> body) override {
    return true;
  }
  
  int64_t random_id;
};

// Server MUST return list of truncated to 8 bytes of SHA-256 of available keyss
struct ResponseStartAuth : public PackageMessage {
  // HEADER = 0xE1
  static const uint8_t HEADER = Package::HEADER_RESPONSE_START_AUTH;

  int32_t GetMessageHeader() const override {
    return HEADER;
  }
  
  bool Decode(std::unique_ptr<folly::IOBuf> body) override {
    return true;
  }

  int64_t random_id;
  std::list<int64_t> available_keys;
  std::string server_nonce;
};
  
// Client downloads required key. Client can skip downloading keys if it have built-in keys installed.
struct RequestGetServerKey : public PackageMessage {
  // HEADER = 0xE2
  static const uint8_t HEADER = Package::HEADER_REQUEST_GET_SERVER_KEY;
  
  int32_t GetMessageHeader() const override {
    return HEADER;
  }
  
  bool Decode(std::unique_ptr<folly::IOBuf> body) override {
    return true;
  }
  
  int64_t key_id;
};

// Server return raw key data. Client MUST to check received key by comparing FULL hash that is hardcoded inside application. Again, DON'T compare truncated hashes - this is insecure.
struct ResponseGetServerKey : public PackageMessage {
  // HEADER = 0xE2
  static const uint8_t HEADER = Package::HEADER_RESPONSE_GET_SERVER_KEY;
  
  int32_t GetMessageHeader() const override {
    return HEADER;
  }
  
  bool Decode(std::unique_ptr<folly::IOBuf> body) override {
    return true;
  }
  
  int64_t key_id;
  std::string key;
};

// Performing Diffie Hellman
struct RequestDH : public PackageMessage {
  // HEADER = 0xE6
  static const uint8_t HEADER = Package::HEADER_REQUEST_DH;
  
  int32_t GetMessageHeader() const override {
    return HEADER;
  }
  
  bool Decode(std::unique_ptr<folly::IOBuf> body) override {
    return true;
  }
  
  int64_t random_id;
  
  // Used keyId
  int64_t key_id;
  
  // Client's 32 securely generated bytes
  std::string client_nonce;
  // Client's key used for encryption
  std::string client_key;
};

/ *
  Calculations
  
  pre_master_secret := <result_of_dh>
  master_secret := PRF_COMBINED(pre_master_secret, "master secret", clientNonce + ServerNonce, 128)
  verify := PRF_COMBINED(master_secret, "client finished", clientNonce + ServerNonce, 256)
  verify_sign := Ed25519(verification, server_private_signing_key)
  
  where PRF_COMBINED:
    PRF(COMBINE(SHA256, STREEBOG256)), where:
      COMBINE(str, HASH1, HASH2) = HASH1(str + HASH2(str))
 * /
// master_secret is result encryption key. First 128 bytes is US encryption keys and last 128 bytes is Russian encryption keys.
// HEADER_RESPONSE_DO_DH
struct ResponseDoDH : public PackageMessage {
  // HEADER = 0xE7
  static const uint8_t HEADER = Package::HEADER_RESPONSE_DO_DH;
  
  int GetMessageHeader() const override {
    return HEADER;
  }
  
  bool Decode(std::unique_ptr<folly::IOBuf> body) override {
    return true;
  }

  int64_t random_id;
  std::string verify;
  std::string verify_sign;
};
*/

struct ZProtoHeader {
  // uint8_t zproto_type;
  int64_t message_id;
  // ZProtoRpcResponse字段有用
  int64_t req_message_id;
};


struct ZProto {
  enum ZProtoType {
    PROTO_RPC_REQUEST = 0x03,
    PROTO_RPC_RESPONSE = 0x04,
    PROTO_PUSH = 0x05,
    MESSAGE_ACK = 0x06,
    UNSENT_MESSAGE = 0x07,
    UNSENT_RESPONSE = 0x08,
    REQUEST_RESEND = 0x09,
    NEW_SESSION = 0x0C,
    SESSION_HELLO = 0x0F,
    SESSION_LOST = 0x10,
    CONTAINER = 0x0A,
  };

  const static uint32_t kHeaderLength = 9; //

  bool Decode(PlainTextMessage& plain_text_message) {
    package_header = plain_text_message.package_header;
    zoroto_header.message_id = plain_text_message.message_id;
    // auth_id = plain_text_message.auth_id;
    // session_id = plain_text_message.session_id;
    // message_id = plain_text_message.GetMessageHeader();
    message_data.swap(plain_text_message.message);
    
    try {
      folly::io::Cursor c(message_data.get());
      zproto_type = c.readBE<uint8_t>();
      // zoroto_header.message_id = c.readBE<int64_t>();
      nebula::io_buf_util::TrimStart(message_data.get(), 1);
    } catch(...) {
      // TODO(@wubenqi): error's log
      return false;
    }
    return true;
  }

  
  // int64_t auth_id;
  // random identifier of current session
  // int64_t session_id;

  PackageHeader package_header;
  ZProtoHeader zoroto_header;
  
  // int64_t message_id;
  uint8_t zproto_type;
  
  std::unique_ptr<folly::IOBuf> message_data;
};

//////////////////////////////////////////////////////////////////////
struct ZProtoMessage {
  virtual ~ZProtoMessage() = default;
  
  // Frame类型
  virtual uint8_t GetFrameType() const {
    return Frame::PROTO;
  }
  
  // Package类型
  virtual uint8_t GetPackageType() const {
    return Package::PLAIN_TEXT_MESSAGE;
  }
  
  virtual uint8_t GetZProtoType() const = 0;
  
  virtual bool Decode(ZProto& zproto) {
    package_header = zproto.package_header;
    zproto_header = zproto.zoroto_header;
    return true;
  }
  
  PackageHeader package_header;
  ZProtoHeader zproto_header;
  
  // Request body
  std::unique_ptr<folly::IOBuf> payload;
};

struct ProtoRpcRequest : public ZProtoMessage {
  // HEADER = 0x03
  static const uint8_t HEADER = ZProto::PROTO_RPC_REQUEST;
  
  uint8_t GetZProtoType() const override {
    return HEADER;
  }

  bool Decode(ZProto& zproto) override {
    ZProtoMessage::Decode(zproto);
    payload.swap(zproto.message_data);
    return true;
  }
};

struct ProtoRpcResponse : public ZProtoMessage {
  // HEADER = 0x04
  static const uint8_t HEADER = ZProto::PROTO_RPC_RESPONSE;
  
  uint8_t GetZProtoType() const override {
    return HEADER;
  }
  
  bool Decode(ZProto& zproto) override {
    ZProtoMessage::Decode(zproto);
    payload.swap(zproto.message_data);
    try {
      folly::io::Cursor c(payload.get());
      req_message_id = c.readBE<int64_t>();
      nebula::io_buf_util::TrimStart(payload.get(), 8);
    } catch(...) {
      // TODO(@wubenqi): error's log
      return false;
    }
    return true;
  }

  // messageId from Message that contains ProtoRpcRequest
  int64_t req_message_id;
};

struct ProtoPush : public ZProtoMessage {
  // HEADER = 0x05
  static const uint8_t HEADER = ZProto::PROTO_PUSH;

  uint8_t GetZProtoType() const override {
    return HEADER;
  }
  
  bool Decode(ZProto& zproto) override {
    ZProtoMessage::Decode(zproto);
    payload.swap(zproto.message_data);
    return true;
  }
};

/*
struct MessageAck : public ZProtoMessage {
  // HEADER = 0x06
  static const uint8_t HEADER = 0x06;
  
  int GetZProtoHeader() const override {
    return HEADER;
  }

  bool Decode(std::unique_ptr<folly::IOBuf> body) override {
    try {
      folly::io::Cursor c(body.get());
      int size = c.readBE<int>();
      for (int i=0; i<size; ++i) {
        message_ids.push_back(c.readBE<int64_t>());
      }
    } catch(...) {
      // TODO(@wubenqi): error's log
      return false;
    }
    return true;
  }
  
  // HEADER = 0x06
  // Message Identificators for confirmation
  std::vector<int64_t> message_ids;
};

// Notification about unsent message (usually ProtoRpcRequest or ProtoPush)
struct UnsentMessage : public ZProtoMessage {
  // HEADER = 0x07
  static const uint8_t HEADER = 0x07;
  
  int GetZProtoHeader() const override {
    return HEADER;
  }

  bool Decode(std::unique_ptr<folly::IOBuf> body) override {
    try {
      folly::io::Cursor c(body.get());
      
      message_id = c.readBE<int64_t>();
      len = c.readBE<int32_t>();
    } catch(...) {
      // TODO(@wubenqi): error's log
      return false;
    }
    return true;
  }

  // HEADER = 0x07
  // Sent Message Id
  int64_t message_id;
  // Size of message in bytes
  int32_t len;
};

// Notification about unsent ProtoRpcResponse
struct UnsentResponse : public ZProtoMessage {
  // HEADER = 0x08
  static const uint8_t HEADER = 0x08;
  
  int GetZProtoHeader() const override {
    return HEADER;
  }

  bool Decode(std::unique_ptr<folly::IOBuf> body) override {
    try {
      folly::io::Cursor c(body.get());
      
      message_id = c.readBE<int64_t>();
      request_message_id = c.readBE<int64_t>();
      len = c.readBE<int32_t>();
    } catch(...) {
      // TODO(@wubenqi): error's log
      return false;
    }
    return true;
  }

  // HEADER = 0x08
  // Sent Message Id
  int64_t message_id;
  // Request Message Id
  int64_t request_message_id;
  // Size of message in bytes
  int32_t len;
};

// Requesting resending of message
struct RequestResend : public ZProtoMessage {
  // HEADER = 0x09
  static const uint8_t HEADER = 0x09;

  int GetZProtoHeader() const override {
    return HEADER;
  }
  
  bool Decode(std::unique_ptr<folly::IOBuf> body) override {
    try {
      folly::io::Cursor c(body.get());
      
      message_id = c.readBE<int64_t>();
    } catch(...) {
      // TODO(@wubenqi): error's log
      return false;
    }
    return true;
  }

  // HEADER = 0x09
  // Message Id for resend
  int64_t message_id; // : long
};

struct NewSession : public ZProtoMessage {
  // HEADER = 0x0C
  static const uint8_t HEADER = 0x0C;
  
  int GetZProtoHeader() const override {
    return HEADER;
  }
  
  bool Decode(std::unique_ptr<folly::IOBuf> body) override {
    try {
      folly::io::Cursor c(body.get());
      
      session_id = c.readBE<int64_t>();
      message_id = c.readBE<int64_t>();
    } catch(...) {
      // TODO(@wubenqi): error's log
      return false;
    }
    return true;
  }

  // HEADER = 0x0C
  // Created Session Id
  int64_t session_id;
  // Message Id of Message that created session
  int64_t message_id;
};

struct SessionHello : public ZProtoMessage {
  // HEADER = 0x0F
  static const uint8_t HEADER = 0x0F;
  
  int GetZProtoHeader() const override {
    return HEADER;
  }
  
  bool Decode(std::unique_ptr<folly::IOBuf> body) override {
    return true;
  }
};

struct SessionLost : public ZProtoMessage {
  // HEADER = 0x10
  static const uint8_t HEADER = 0x10;
  
  int GetZProtoHeader() const override {
    return HEADER;
  }
  
  bool Decode(std::unique_ptr<folly::IOBuf> body) override {
    return true;
  }
};

struct Container : public ZProtoMessage {
  // HEADER = 0x0A
  static const uint8_t HEADER = 0x0A;
  
  int GetZProtoHeader() const override {
    return HEADER;
  }
  
  bool Decode(std::unique_ptr<folly::IOBuf> body) override {
    return true;
  }

  // Messages count
  int32_t count;
  
  // Messages in container
  // list<ZProtoMessage> data;
};
*/

//struct BasciSyncHeader {
//  uint8_t basic_sync_type;
//};

struct BasicSync {
  enum BasicSyncType {
    RPC_REQUEST = 0x01,
    RPC_OK = 0x01,
    RPC_ERROR = 0x02,
    RPC_FLOOD_WAIT = 0x03,
    RPC_INTERNAL_ERROR = 0x04,
    PUSH = 0x01,
  };
  
  const static uint32_t kHeaderLength = 1; //

  bool Decode(ZProtoMessage& zproto) {
    package_header = zproto.package_header;
    zproto_header = zproto.zproto_header;
    
    message_data.swap(zproto.payload);
    
//    auth_id = zproto.auth_id;
//    session_id = zproto.session_id;
//    message_id = zproto.message_id;
//    zproto_header = zproto.zproto_header;
//    if (zproto.zproto_header == 1) {
//      zproto.req_message_id = zproto.req_message_id;
//    }
//    
//    if (zproto_header == HEADER_PROTO_RPC_REQUEST) {
//      auto& req = dynamic_cast<ProtoRpcRequest>(zproto);
//      message_data.swap(req.message_data);
//    } else if (zproto_header == HEADER_PROTO_RPC_RESPONSE) {
//      auto& rsp = dynamic_cast<ProtoRpcResponse>(zproto);
//      req_message_id = rsp.req_message_id;
//      message_data.swap(req.message_data);
//    } else if (zproto_header == HEADER_PROTO_PUSH) {
//      auto& push = dynamic_cast<ProtoPush>(zproto);
//      message_data.swap(push.message_data);
//    }
    try {
      folly::io::Cursor c(message_data.get());
      basic_sync_type = c.readBE<uint8_t>();
      nebula::io_buf_util::TrimStart(message_data.get(), kHeaderLength);
      // body->trimStart(1);
    } catch(...) {
      // TODO(@wubenqi): error's log
      return false;
    }
    return true;
  }
  
  PackageHeader package_header;
  ZProtoHeader zproto_header;
  
  uint8_t basic_sync_type;
  
  std::unique_ptr<folly::IOBuf> message_data;
};

//////////////////////////////////////////////////////////////////////
struct BasicSyncMessage {
  // HEADER = 0x07
  // static const uint8_t HEADER = 0x07;
  virtual ~BasicSyncMessage() = default;
  
  // Frame类型
  virtual uint8_t GetFrameType() const {
    return Frame::PROTO;
  }
  
  // Package类型
  virtual uint8_t GetPackageType() const {
    return Package::PLAIN_TEXT_MESSAGE;
  }
  
  virtual uint8_t GetZProtoType() const = 0;

  virtual uint8_t GetBasicSyncType() const = 0;

  virtual uint8_t CalcPackageHeaderSize() const {
    return  Package::kHeaderLength +
            ZProto::kHeaderLength +
            BasicSync::kHeaderLength;
  }

  /////////////////////////////////////////////////////////////////////////////
  virtual bool Decode(BasicSync& sync) {
    package_header = sync.package_header;
    zproto_header = sync.zproto_header;
    return true;
  }
  
  bool SerializeToIOBuf(std::unique_ptr<folly::IOBuf>& io_buf) const;

  virtual int32_t CalcBasicSyncMessageSize() const = 0;
  virtual void Encode(IOBufWriter& iobw) const = 0;
  
  
  PackageHeader package_header;
  ZProtoHeader zproto_header;
};

struct RpcRequest : public BasicSyncMessage {
  // HEADER = 0x01
  static const uint8_t HEADER = BasicSync::RPC_REQUEST;
  
  uint8_t GetZProtoType() const override {
    return ZProto::PROTO_RPC_REQUEST;
  }

  uint8_t GetBasicSyncType() const override {
    return HEADER;
  }
  
  bool Decode(BasicSync& sync) override {
    BasicSyncMessage::Decode(sync);
    body.swap(sync.message_data);
    try {
      folly::io::Cursor c(body.get());
      method_id = c.readBE<int32_t>();
      nebula::io_buf_util::TrimStart(body.get(), 4);
    } catch(...) {
      // TODO(@wubenqi): error's log
      return false;
    }

    return true;
  }

  int32_t CalcBasicSyncMessageSize() const override {
    return sizeof(method_id) + static_cast<uint32_t>(body->computeChainDataLength());
  }
  
  void Encode(IOBufWriter& iobw) const override {
    iobw.writeBE(method_id);
    folly::io::Cursor c(body.get());
    iobw.push(c, static_cast<uint32_t>(body->computeChainDataLength()));
  }
  
  // ID of API Method Request
  int32_t method_id; //: int
  // Encoded Request
  // body: bytes
  mutable std::unique_ptr<folly::IOBuf> body;
};

// Successful RPC
struct RpcOk : public BasicSyncMessage {
  // HEADER = 0x01
  static const uint8_t HEADER = BasicSync::RPC_OK;
  
  uint8_t GetZProtoType() const override {
    return ZProto::PROTO_RPC_RESPONSE;
  }
  
  uint8_t GetBasicSyncType() const override {
    return HEADER;
  }
  
  bool Decode(BasicSync& sync) override {
    BasicSyncMessage::Decode(sync);
    body.swap(sync.message_data);
    try {
      folly::io::Cursor c(body.get());
      method_response_id = c.readBE<int32_t>();
      nebula::io_buf_util::TrimStart(body.get(), 4);
    } catch(...) {
      // TODO(@wubenqi): error's log
      return false;
    }
    return true;
  }

  int32_t CalcBasicSyncMessageSize() const override {
    return sizeof(method_response_id) + static_cast<uint32_t>(body->computeChainDataLength());
  }
  
  void Encode(IOBufWriter& iobw) const override {
    iobw.writeBE(method_response_id);
    folly::io::Cursor c(body.get());
    iobw.push(c, static_cast<uint32_t>(body->computeChainDataLength()));
  }

  // ID of API Method Response
  int32_t method_response_id; // : int
  // Encoded response
  // body: bytes
  mutable std::unique_ptr<folly::IOBuf> body;
};

// RPC Error
struct RpcError : public BasicSyncMessage {
  // HEADER = 0x02
  static const uint8_t HEADER = BasicSync::RPC_ERROR;
  
  uint8_t GetZProtoType() const override {
    return ZProto::PROTO_RPC_RESPONSE;
  }
  
  uint8_t GetBasicSyncType() const override {
    return HEADER;
  }
  
  bool Decode(BasicSync& sync) override {
    BasicSyncMessage::Decode(sync);
    try {
      folly::io::Cursor c(sync.message_data.get());
      error_code = c.readBE<int32_t>();
      
      auto len = c.readBE<int32_t>();
      error_tag = c.readFixedString(len);
      
      len = c.readBE<int32_t>();
      user_message = c.readFixedString(len);
      
      can_try_again = c.readBE<uint8_t>();
      // == 0 ? false : true;
      
      len = c.readBE<int32_t>();
      error_data = c.readFixedString(len);
    } catch(...) {
      // TODO(@wubenqi): error's log
      return false;
    }
    return true;
  }
  
  int32_t CalcBasicSyncMessageSize() const override {
    return sizeof(error_code) +
            4 + error_tag.length() +
            4 + user_message.length() +
            sizeof(uint8_t) +
            4 + user_message.length();
  }
  
  void Encode(IOBufWriter& iobw) const override {
    iobw.writeBE(error_code);

    iobw.writeBE((int32_t)error_tag.length());
    iobw.push((const uint8_t*)error_tag.data(), error_data.length());

    iobw.writeBE((int32_t)user_message.length());
    iobw.push((const uint8_t*)user_message.data(), user_message.length());

    uint8_t try2 = can_try_again ? 1 : 0;
    iobw.writeBE(try2);
    
    iobw.writeBE((int32_t)error_data.length());
    iobw.push((const uint8_t*)error_data.data(), error_data.length());
  }

  // Error Code like HTTP Error code
  int32_t error_code;
  // Error Tag like "ACCESS_DENIED"
  std::string error_tag;

  // User visible error
  std::string user_message;

  // Can user try again
  bool can_try_again;

  // Some additional data of error
  std::string error_data;
};

// RPC Flood Control.
// Client need to repeat request after delay
struct RpcFloodWait : public BasicSyncMessage {
  // HEADER = 0x03
  static const uint8_t HEADER = BasicSync::RPC_FLOOD_WAIT;
  
  uint8_t GetZProtoType() const override {
    return ZProto::PROTO_RPC_RESPONSE;
  }
  
  uint8_t GetBasicSyncType() const override {
    return HEADER;
  }
  
  bool Decode(BasicSync& sync) override {
    BasicSyncMessage::Decode(sync);
    try {
      folly::io::Cursor c(sync.message_data.get());
      delay = c.readBE<int32_t>();
    } catch(...) {
      // TODO(@wubenqi): error's log
      return false;
    }
    return true;
  }

  int32_t CalcBasicSyncMessageSize() const override {
    return sizeof(delay);
  }
  
  void Encode(IOBufWriter& iobw) const override {
    iobw.writeBE(delay);
  }

  // Repeat delay on seconds
  int32_t delay;
};

// Internal Server Error
// Client may try to resend message
struct RpcInternalError : public BasicSyncMessage {
  // HEADER = 0x04
  static const uint8_t HEADER = BasicSync::RPC_INTERNAL_ERROR;
  
  uint8_t GetZProtoType() const override {
    return ZProto::PROTO_RPC_RESPONSE;
  }
  
  uint8_t GetBasicSyncType() const override {
    return HEADER;
  }
  
  bool Decode(BasicSync& sync) override {
    BasicSyncMessage::Decode(sync);
    try {
      folly::io::Cursor c(sync.message_data.get());
      
      can_try_again = c.readBE<uint8_t>();
      try_again_delay = c.readBE<int32_t>();
    } catch(...) {
      // TODO(@wubenqi): error's log
      return false;
    }
    return true;
  }

  int32_t CalcBasicSyncMessageSize() const override {
    return sizeof(uint8_t) + sizeof(try_again_delay);
  }
  
  void Encode(IOBufWriter& iobw) const override {
    uint8_t try2 = can_try_again ? 1 : 0;
    iobw.writeBE(try2);
    
    iobw.writeBE(try_again_delay);
  }

  bool can_try_again; //: bool
  int32_t try_again_delay; //: int
};

struct Push : public BasicSyncMessage {
  static const uint8_t HEADER = BasicSync::PUSH;
  
  uint8_t GetZProtoType() const override {
    return ZProto::PROTO_PUSH;
  }
  
  uint8_t GetBasicSyncType() const override {
    return HEADER;
  }
  
  bool Decode(BasicSync& sync) override {
    BasicSyncMessage::Decode(sync);
    body.swap(sync.message_data);

    try {
      folly::io::Cursor c(sync.message_data.get());
      
      update_id = c.readBE<int32_t>();
      nebula::io_buf_util::TrimStart(body.get(), 4);

    } catch(...) {
      // TODO(@wubenqi): error's log
      return false;
    }
    return true;
  }

  int32_t CalcBasicSyncMessageSize() const override {
    return sizeof(update_id) + static_cast<int32_t>(body->computeChainDataLength());
  }
  
  void Encode(IOBufWriter& iobw) const override {
    iobw.writeBE(update_id);
    folly::io::Cursor c(body.get());
    iobw.push(c, static_cast<int32_t>(body->computeChainDataLength()));
  }

  // Push Entity Id
  int32_t update_id; //: int
  // Encoded Push body
  mutable std::unique_ptr<folly::IOBuf> body;
};

//////////////////////////////////////////////////////////////////////////////////////////////////
using FrameFactory = nebula::SelfRegisterFactoryManager<FrameMessage, uint8_t>;
using PackageFactory = nebula::SelfRegisterFactoryManager<PackageMessage, int>;
using ZProtoFactory = nebula::SelfRegisterFactoryManager<ZProtoMessage, int>;
using BasicSyncFactory = nebula::SelfRegisterFactoryManager<BasicSyncMessage, int>;

#endif
