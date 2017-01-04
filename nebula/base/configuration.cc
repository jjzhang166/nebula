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

#include "nebula/base/configuration.h"

namespace nebula {
  
folly::dynamic Configuration::GetValue(const std::string& k) const {
  folly::dynamic rv = nullptr;
  
  // confs为一数组对象
  if (dynamic_conf_.isArray()) {
    for (auto& v2 : dynamic_conf_) {
      // confs
      if (!v2.isObject()) continue;
      if (0 == v2.count(k)) continue;
      rv = v2[k];
      break;
    }
  } else {
    auto tmp = dynamic_conf_.get_ptr(k);
    if (tmp) {
      rv = *tmp;
    }
  }
  
  return rv;
}

}
