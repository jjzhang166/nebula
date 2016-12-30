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

#ifndef NEBULA_BASE_CONFIG_UTIL_H_
#define NEBULA_BASE_CONFIG_UTIL_H_

#include "nebula/base/configurable.h"

#include <iostream>
#include <vector>

namespace nebula {
  
struct SystemConfig : public Configurable {
    virtual ~SystemConfig() = default;

    bool SetConf(const std::string& conf_name, const Configuration& conf) override;

    void PrintDebug() const {
    }
};

}

#endif
