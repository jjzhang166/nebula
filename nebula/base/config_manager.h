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

#ifndef NEBULA_BASE_CONFIG_MANAGER_H_
#define NEBULA_BASE_CONFIG_MANAGER_H_

#include <string>
#include <map>

#include <folly/FBString.h>
#include <folly/Singleton.h>

#include "nebula/base/configurable.h"
#include "nebula/base/configuration.h"

// 配置文件管理器
// 1. 首先注册配置项
// 2. 然后加载配置文件
// TODO(@benqi)，检查配置文件是否修改
// 3. 检查配置文件是否修改，并通知各配置项

namespace folly {
class EventBase;
} // folly

namespace nebula {
  
class ConfigManager {
public:
    ~ConfigManager() = default;

    // 单件接口
    static ConfigManager* GetInstance();
    
    // 注意：
    //  item的生命周期
    void Register(const folly::fbstring& item_name, Configurable* item, bool recv_updated = false);
    void UnRegister(const folly::fbstring& item_name);
    
    bool Initialize(const std::string& config_file);
    
    void StartObservingConfigFile(folly::EventBase* evb);
  
private:
    friend class folly::Singleton<ConfigManager>;

    ConfigManager() = default;

    bool OnConfigFileUpdated();
    bool OnConfigDataUpdated(const folly::fbstring& config_data, bool is_first);
    
    std::string config_file_;
    
    struct ConfigurableWithUpdated {
        Configurable* configurable{nullptr};
        bool recv_updated{false};              // 是否要接收通知
    };
    
    typedef std::map<folly::fbstring, ConfigurableWithUpdated> ConfigItemMap;
    ConfigItemMap config_items_;
    
    bool is_watched_{false};
};

}

#endif
