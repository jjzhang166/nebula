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

#include "nebula/net/engine/http_client.h"

#include <folly/futures/Future.h>
#include <folly/MoveWrapper.h>

#include <folly/io/async/EventBase.h>
#include <folly/SocketAddress.h>
#include <folly/io/async/EventBaseManager.h>

#include <proxygen/lib/http/HTTPHeaders.h>
#include <proxygen/lib/http/HTTPConnector.h>

#include "nebula/base/logger/glog_util.h"

// 默认为10s
#define HTTP_FETCH_TIMEOUT 2

using namespace folly;
using namespace proxygen;

#define kDefaultConnectTimeout 1000

namespace {

// TODO(@benqi): 直接使用
#ifndef __MACH__
static __thread char g_resolve_buffer[64 * 1024];
#endif
  
static std::string S_GetHostByName(const std::string& hostname) {
  std::string ip;
  struct in_addr sin_addr;
  
  struct hostent hent;
  struct hostent* he = NULL;
  bzero(&hent, sizeof(hent));
  
#ifndef __MACH__
  int herrno = 0;
  int ret = gethostbyname_r(hostname.c_str(), &hent, g_resolve_buffer, sizeof(g_resolve_buffer), &he, &herrno);
#else
  he = gethostbyname(hostname.c_str());
  int ret = 0;
#endif
  if (ret == 0 && he != NULL && he->h_addr) {
    char host_ip[32] = {0};
    sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr);
    // *(struct in_addr*) he->h_addr_list[0];
    inet_ntop(AF_INET, &sin_addr, host_ip, sizeof(host_ip));
    ip = host_ip;
  } else {
    LOG(ERROR) << "S_GetHostByName - gethostbyname error: " << hostname;
  }
  
  return ip;
}

}

std::string HttpClientReply::ToString() const {
  std::ostringstream oss;
  oss << "result: " << result;
  if (body) {
    oss << body->clone()->moveToFbString();
  }
  return oss.str();
}

HttpClient::HttpClient(folly::EventBase* evb, const HttpClientReplyCallback& cb) :
evb_(evb),
cb_(cb) {
  
  if (evb_ == nullptr) {
    evb_ = folly::EventBaseManager::get()->getEventBase();
  }
}

HttpClient::~HttpClient() {
    // LOG(INFO) << "HttpClient::~HttpClient()";
}

void HttpClient::Fetch(proxygen::HTTPMethod method,
                       const std::string& url,
                       const std::string& headers,
                       const std::string& post_data) {
  URL url2(url);
  if (!url2.isValid()) {
    reply_.result = HttpClientReply::INVALID;
      if (cb_) {
          cb_(reply_);
      }
      return;
  }

  // TODO(@benqi): 不使用S_GetHostByName
  std::string str = S_GetHostByName(url2.getHost());
  if (str.empty()) {
      reply_.result = HttpClientReply::INVALID;
      if (cb_) {
          cb_(reply_);
      }
      return;
  }

  AddRef();
  
  SocketAddress addr(str, url2.getPort(), false);
  
  proxygen::HTTPHeaders headers2;
  if (!headers.empty()) {
      std::vector<folly::StringPiece> header_list;
      folly::split(";", headers, header_list);
      for (const auto& header_pair: header_list) {
          std::vector<folly::StringPiece> nv;
          folly::split(':', header_pair, nv);
          if (nv.size() > 0) {
              if (nv[0].empty()) {
                  continue;
              }
              StringPiece value("");
              if (nv.size() > 1) {
                  value = nv[1];
              } // trim anything else
              headers2.add(nv[0], value);
          }
      }

  }
  
  
  // copyBuffer();
  std::unique_ptr<folly::IOBuf> post_data2;
  if (method == proxygen::HTTPMethod::POST && !post_data.empty()) {
      post_data2 = std::move(folly::IOBuf::copyBuffer(post_data));
  }
  
  // folly::IOBuf::
  http_client_ = std::make_unique<HTTPClientLib>(evb_,
                                                    method,
                                                    url2,
                                                    headers2,
                                                    std::move(post_data2),
                                                    this);
  
  // Note: HHWheelTimer is a large object and should be created at most
  // once per thread
  timer_.reset(new HHWheelTimer(
                                evb_,
                                std::chrono::milliseconds(HHWheelTimer::DEFAULT_TICK_INTERVAL),
                                AsyncTimeout::InternalEnum::NORMAL,
                                std::chrono::milliseconds(5000)));
  
  connector_ = folly::make_unique<HTTPConnector>(http_client_.get(), timer_.get());
  
  static const AsyncSocket::OptionMap opts{{{SOL_SOCKET, SO_REUSEADDR}, 1}};
  
  connector_->connect(evb_, addr,
                      std::chrono::milliseconds(kDefaultConnectTimeout), opts);
}

void HttpClient::OnHeadersComplete(std::unique_ptr<proxygen::HTTPMessage> msg) {
  reply_.headers = std::move(msg);
}

void HttpClient::OnBodyComplete(std::unique_ptr<folly::IOBuf> body) {
  if (reply_.result == HttpClientReply::NONE) {
      reply_.result = HttpClientReply::OK;
      reply_.body = std::move(body);
  }

  if (cb_) {
      cb_(reply_);
  }
  
  ReleaseRef();
}

void HttpClient::OnConnectError(const folly::AsyncSocketException& ex) {
  reply_.result = HttpClientReply::CONNECT_ERROR;
  
  if (cb_) {
    cb_(reply_);
  }
  
  ReleaseRef();
}

void HttpClient::OnError(const proxygen::HTTPException& error) {
    reply_.result = HttpClientReply::HTTP_ERROR;
    
//    if (cb_) {
//        cb_(reply_);
//    }
//    
//    ReleaseRef();
}

void HttpClientGet(folly::EventBase* evb,
                   const std::string& url,
                   HttpClientReplyCallback cb) {
  auto http_client = HttpClient::CreateHttpClient(evb, cb);
  http_client->AddRef();
  http_client->Fetch(proxygen::HTTPMethod::GET, url, "");
  http_client->ReleaseRef();
}

void HttpClientGet(folly::EventBase* evb,
                   const std::string& url,
                   const std::string& headers,
                   HttpClientReplyCallback cb) {
  auto http_client = HttpClient::CreateHttpClient(evb, cb);
  http_client->AddRef();
  http_client->Fetch(proxygen::HTTPMethod::GET, url, headers);
  http_client->ReleaseRef();
}

void HttpClientPost(folly::EventBase* evb,
                    const std::string& url,
                    const std::string& post_data,
                    HttpClientReplyCallback cb) {
  auto http_client = HttpClient::CreateHttpClient(evb, cb);
  http_client->AddRef();
  http_client->Fetch(proxygen::HTTPMethod::POST, url, "", post_data);
  http_client->ReleaseRef();
}

void HttpClientPost(folly::EventBase* evb,
                    const std::string& url,
                    const std::string& headers,
                    const std::string& post_data,
                    HttpClientReplyCallback cb) {
  auto http_client = HttpClient::CreateHttpClient(evb, cb);
  http_client->AddRef();
  http_client->Fetch(proxygen::HTTPMethod::GET, url, headers, post_data);
  http_client->ReleaseRef();
}

#if 0
///////////////////////////////////////////////////////////////////////////////////////////
HttpClientReplyPtr HttpClientSyncFetch(folly::EventBase* evb,
                                       FiberDataManager* fiber_data_manger,
                                       proxygen::HTTPMethod method,
                                       const std::string& url,
                                       const std::string& headers,
                                       const std::string& post_data) {

  auto reply_data = std::make_shared<HttpClientReply>();
  if (!fiber_data_manger) {
    LOG(ERROR) << "HttpClientSyncFetch - fiber_data_manger is null, by fetch: " << url;
    return reply_data;
  }
  
  auto fiber_data = fiber_data_manger->AllocFiberData();
  if (fiber_data) {
    // 有可能已经超时，需要使用weak_ptr来检查是否已经被释放
    // TODO(@benqi): CreateHttpClient使用std::shared_ptr<HttpClientReplyData>
    std::weak_ptr<HttpClientReplyData> r1(reply_data);
    std::weak_ptr<FiberDataInfo> f1(fiber_data);
    auto http_client = HttpClient::CreateHttpClient(evb, [r1, f1, url] (HttpClientReplyData& reply) {
      auto r2 = r1.lock();
      auto f2 = f1.lock();
      if (r2 && f2) {
        reply.Move(*r2);
        if (reply.result != HttpReplyCode::INVALID) {
          f2->Post(true, impdu::CImPduBase::getEmptyCImPduBase());
          // f2->baton.post();
        }
      }
    });
    
    CallChainData chain_back = GetCallChainDataByThreadLocal();
    TRACE_FIBER() << "HttpClientSyncFetch - Ready fetch url: " << url
    << ", call_chain:" << chain_back.ToString()
    <<  ", " << fiber_data->ToString();
    
    http_client->AddRef();
    FiberDataManager::IncreaseStats(FiberDataManager::f_http);
    http_client->Fetch(method, url, headers, post_data);
    if (reply_data->result != HttpReplyCode::INVALID) {
      
      fiber_data->Wait(FIBER_HTTP_TIMEOUT);
      if (fiber_data->state == FiberDataInfo::kTimeout) {
        // TODO(@benqi):
        // 添加调用链追踪
        LOG(ERROR) << "HttpClientSyncFetch - fiber wait timeout:  "
        <<  fiber_data->ToString() << ", url: " << url;
      }
      // bool r = fiber_data->baton.timed_wait((std::chrono::seconds(FIBER_HTTP_TIMEOUT)));
      // if (r) {
      //     // LOG(INFO) << "";
      // } else {
      //    LOG(ERROR) << "HttpClientSyncFetch - fiber wait timeout:  "
      //                <<  fiber_data->ToString() << ", url: " << url;
      // }
    }
    fiber_data_manger->DeleteFiberData(fiber_data);
    http_client->ReleaseRef();
    
    TRACE_FIBER() << "HttpClientSyncFetch - end fetch url: " << url
    << ", call_chain:" << chain_back.ToString()
    <<  "," << fiber_data->ToString();
    
    GetCallChainDataByThreadLocal() = chain_back;
    
  } else {
    LOG(ERROR) << "HttpClientSyncFetch - fiber_data is null by fetch: " << url;
  }
  
  return reply_data;
}
#endif

HttpClientReplyPtr HttpClientSyncFetch(folly::EventBase* evb,
                                       proxygen::HTTPMethod method,
                                       const std::string& url,
                                       const std::string& headers,
                                       const std::string& post_data) {

  auto reply_data = std::make_shared<HttpClientReply>();
  auto http_client = HttpClient::CreateHttpClient(evb, [reply_data](HttpClientReply& reply) mutable {
    reply.Move(*reply_data);
  });
  
  http_client->AddRef();
  http_client->Fetch(proxygen::HTTPMethod::GET, url, headers, post_data);
  http_client->ReleaseRef();
  evb->loop();
  return reply_data;
  
//  auto reply_data = std::make_shared<HttpClientReplyData>();
//  // reply_data->result = HttpReplyCode::INVALID;
//  
//  // FiberDataManager* fiber_data_manger = Root::GetServiceRouterManager()->FindFiberData();
//  FiberDataManager& fiber_data_manger = GetFiberDataManagerByThreadLocal();
//  if (!fiber_data_manger.IsInited()) {
//    LOG(ERROR) << "HttpClientSyncFetch - fiber_data_manger is null by fetch: " << url;
//    return reply_data;
//  }
//  
//  return HttpClientSyncFetch(evb, &fiber_data_manger, method, url, headers, post_data);
}


HttpClientReplyPtr HttpClientSyncGet(folly::EventBase* evb,
                                     const std::string& url) {
  // return HttpClientSyncFetch(evb, proxygen::HTTPMethod::GET, url, "", "");
  return HttpClientSyncFetch(evb, proxygen::HTTPMethod::GET, url, "", "");
  
  // f.then([] (HttpClientReplyPtr reply) {
  //  std::cout << reply->ToString() << std::endl;
  // });
  
  // return std::make_shared<HttpClientReply>();
  
  // return f.get();
}

//HttpClientReplyPtr HttpClientSyncGet(folly::EventBase* evb,
//                                     const std::string& url,
//                                     const std::string& headers) {
//  return HttpClientSyncFetch(evb, proxygen::HTTPMethod::GET, url, headers, "");
//}
//
//HttpClientReplyPtr HttpClientSyncPost(folly::EventBase* evb,
//                                      const std::string& url,
//                                      const std::string& post_data) {
//  return HttpClientSyncFetch(evb, proxygen::HTTPMethod::GET, url, "", post_data);
//}
//
//HttpClientReplyPtr HttpClientSyncPost(folly::EventBase* evb,
//                                      const std::string& url,
//                                      const std::string& headers,
//                                      const std::string& post_data) {
//  return HttpClientSyncFetch(evb, proxygen::HTTPMethod::GET, url, headers, post_data);
//}

