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

#ifndef NUBULA_NET_BASE_ATTACH_DATA_HELPER_H_
#define NUBULA_NET_BASE_ATTACH_DATA_HELPER_H_

#include <memory>

namespace nebula {
  
// 主要用于handler里存储附加连接数据，比如用户名等
// 可以使用boost::any
class AttachDataHelper {
public:
  struct AttachData {
    virtual ~AttachData() = default;
  };

  inline AttachData* attach_data() {
    return attach_data_.get();
  }
  
  //////////////////////////////////////////////////////////////////////////
  template <class T>
  inline T* cast() {
    return nullptr;
  }
  
  template <class T>
  inline typename std::enable_if<std::is_base_of<AttachData, T>::value>::type*
  cast() {
    return dynamic_cast<T*>(attach_data_.get());
  }
  
  inline void set_attach_data(const std::shared_ptr<AttachData>& v) {
    attach_data_ = v;
  }
  
protected:
  AttachDataPtr attach_data_;
};

using AttachDataPtr = std::shared_ptr<AttachDataHelper::AttachData>;

}

#endif // NUBULA_NET_BASE_ATTACH_DATA_HELPER_H_

