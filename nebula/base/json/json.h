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

#ifndef NEBULA_BASE_JSON_JSON_H_
#define NEBULA_BASE_JSON_JSON_H_

// #include <folly/dynamic.h>
// #include <folly/DynamicConverter.h>
// #if 0
// #define PATCH_BY_BENQI

#include <folly/json.h>
#include <folly/io/IOBuf.h>
#include "nebula/base/reflection_util.h"
#include "nebula/base/json/dynamic_converter.h"

// using namespace kapok;

// 使用go语言json接口
template<typename T>
T Unmarshal(const folly::dynamic& root) {
  return JsonConvertTo<T>(root);
}

// 会抛出异常...
template<typename T>
folly::dynamic Marshal(const T& o, const char* key = nullptr) {
  if (key == nullptr || key[0] == '\0' ) {
    return ToJsonDynamic(o);
  } else {
    return folly::dynamic::object(key, ToJsonDynamic(o));
  }
}

std::unique_ptr<folly::IOBuf> ToJsonIOBuf(const folly::dynamic& d) {
  auto&& json = folly::toJson(d);
  return folly::IOBuf::copyBuffer(json.c_str(), json.length());
}

#if 0
template<typename T, typename BeginObjec>
typename std::enable_if<is_tuple<T>::value>::type WriteObject(T const& t, BeginObjec) {
		// m_jsutil.StartArray();
		WriteTuple(t);
		// m_jsutil.EndArray();
}

template<std::size_t I = 0, typename Tuple>
typename std::enable_if<I == std::tuple_size<Tuple>::value>::type WriteTuple(const Tuple& t) {
}

template<std::size_t I = 0, typename Tuple>
typename std::enable_if<I < std::tuple_size<Tuple>::value>::type WriteTuple(const Tuple& t) {
		WriteObject(std::get<I>(t), std::false_type{});
		WriteTuple<I + 1>(t);
}

template<typename T, typename BeginObjec>
typename std::enable_if<is_user_class<T>::value>::type WriteObject(const T& t, BeginObjec) {
		// m_jsutil.StartObject();
		WriteTuple(((T&)t).Meta());
		// m_jsutil.EndObject();
}

//class Serializer {
//public:
//  template<typename T>
//  void Serialize(const T& t, const char* key = nullptr)
//  {
//		m_jsutil.Reset();
//		if (key == nullptr)
//      {
//      WriteObject(t, std::true_type{});
//      }
//    else
//      {
//      SerializeImpl(t, key);
//      }
//  }
//  
//private:
//  template<typename T>
//  void SerializeImpl(T const& t, const char* key)
//  {
//		m_jsutil.StartObject();
//		m_jsutil.WriteValue(key);
//		WriteObject(t, std::true_type{});
//		m_jsutil.EndObject();
//  }
//  
//};

// #endif
#endif

#endif
