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

#ifndef NEBULA_BASE_CONFIGURATION_H_
#define NEBULA_BASE_CONFIGURATION_H_

#include <string>
#include <vector>

#include <folly/dynamic.h>

namespace nebula {
  
// 动态配置框架
class Configuration {
public:
    Configuration() = default;
    explicit Configuration(folly::dynamic dynamic_conf) :
        dynamic_conf_(dynamic_conf) {}
    
    virtual ~Configuration() = default;

    inline folly::dynamic GetDynamicConf() const {
        return dynamic_conf_;
    }
    inline void SetDynamicConf(folly::dynamic dynamic_conf) {
        dynamic_conf_ = dynamic_conf;
    }
    
    folly::dynamic GetValue(const std::string& k) const;

    // TODO(@bneqi):
    //   提供方便使用的接口

protected:
    folly::dynamic dynamic_conf_ = nullptr;
};

// services
}

#endif
