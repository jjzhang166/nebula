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

#ifndef NEBULA_NET_ENGINE_HTTP_CLIENT_H_
#define NEBULA_NET_ENGINE_HTTP_CLIENT_H_

#include <folly/io/async/EventBase.h>

#include "nebula/net/http/http_client_lib.h"

// 异步http_client
// 注意：
//   如果body非常大，比如大文件下载，建议不要用此接口，直接重载HttpRequestHandler
struct HttpClientReply {
  enum {
    OK = 0,             // 返回成功
    INVALID = -1,       // 输入参数错误（url解析出错，url主机名解析出错）
    CONNECT_ERROR = -2, // 网络连接错误
    HTTP_ERROR = -3,    // Http内部错误
    NONE = -4,          // 等待
  };

  int result{NONE}; // 返回值：为0成功，注意：为0时headers和body才会设置值
  
  std::unique_ptr<proxygen::HTTPMessage> headers;
  // 注意：IOBuf可能有多个组成
  std::unique_ptr<folly::IOBuf> body;
  
  void Move(HttpClientReply& other) {
    other.result = result;
    other.headers = std::move(headers);
    other.body = std::move(body);
  }
  
  std::string ToString() const;
};

using HttpClientReplyPtr = std::shared_ptr<HttpClientReply>;
typedef std::function<void(HttpClientReply&)> HttpClientReplyCallback;

// TODO(@benqi), check只能new出来使用
class HttpClient : public HTTPClientLibCallback {
public:
  static HttpClient* CreateHttpClient(folly::EventBase* evb, const HttpClientReplyCallback& cb) {
    return new HttpClient(evb, cb);
  }
  
  virtual ~HttpClient();
  
  void Fetch(proxygen::HTTPMethod method,
             const std::string& url,
             const std::string& post_data) {
    Fetch(method, url, "", post_data);
  }
  
  void Fetch(proxygen::HTTPMethod method,
             const std::string& url,
             const std::string& headers,
             const std::string& post_data);
  
  //    void Get(const std::string& url);
  //    void Post(const std::string& url, const std::string& post_data);
  
  // 引用计数
  void AddRef() {
    ++ref_count_;
  }
  
  void ReleaseRef() {
    // LOG(INFO) << "ReleaseRef - ref_count: " << ref_count_;
    
    if (ref_count_ <= 0) {
      LOG(ERROR) << "HttpClient - ReleaseRef error, ref_count_: " << ref_count_;
    } else {
      --ref_count_;
      if (ref_count_ == 0) {
        delete this;
      }
    }
  }
  
protected:
  HttpClient(folly::EventBase* evb, const HttpClientReplyCallback& cb);
  
  // Impl from HTTPClientLibCallback
  void OnHeadersComplete(std::unique_ptr<proxygen::HTTPMessage> msg) override;
  void OnBodyComplete(std::unique_ptr<folly::IOBuf> body) override;
  void OnConnectError(const folly::AsyncSocketException& ex) override;
  void OnError(const proxygen::HTTPException& error) override;
  
  folly::EventBase* evb_{nullptr};
  
  HttpClientReplyCallback cb_;
  HttpClientReply reply_;
  
  std::unique_ptr<proxygen::HTTPConnector> connector_;
  std::unique_ptr<HTTPClientLib> http_client_;
  folly::HHWheelTimer::UniquePtr timer_;
  
  int ref_count_{0};
  
  // int64_t fiber_id_{0};
};

// 异步
void HttpClientGet(folly::EventBase* evb,
                   const std::string& url,
                   HttpClientReplyCallback cb);

void HttpClientGet(folly::EventBase* evb,
                   const std::string& url,
                   const std::string& headers,
                   HttpClientReplyCallback cb);

void HttpClientPost(folly::EventBase* evb,
                    const std::string& url,
                    const std::string& post_data,
                    HttpClientReplyCallback cb);

void HttpClientPost(folly::EventBase* evb,
                    const std::string& url,
                    const std::string& headers,
                    const std::string& post_data,
                    HttpClientReplyCallback cb);

// 使用协程，同步调用
HttpClientReplyPtr HttpClientSyncGet(folly::EventBase* evb,
                                     const std::string& url);

HttpClientReplyPtr HttpClientSyncGet(folly::EventBase* evb,
                                     const std::string& url,
                                     const std::string& headers);

HttpClientReplyPtr HttpClientSyncPost(folly::EventBase* evb,
                                      const std::string& url,
                                      const std::string& post_data);

HttpClientReplyPtr HttpClientSyncPost(folly::EventBase* evb,
                                      const std::string& url,
                                      const std::string& headers,
                                      const std::string& post_data);


#endif
