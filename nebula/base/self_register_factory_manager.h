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

#ifndef NEBULA_BASE_SELF_REGISTER_FACTORY_MANAGER_H_
#define NEBULA_BASE_SELF_REGISTER_FACTORY_MANAGER_H_

#include <string>
#include <unordered_map>

#include "nebula/base/logger/glog_util.h"

namespace nebula {
  
//////////////////////////////////////////////////////////////////////////
// TODO(@benqi): FuncFactoryManager能满足SelfRegisterFactoryManager要求，后续可移除
// C++11实现的
// 遵循了开闭原则，简洁而优雅的自动注册的对象工厂
// C++11确实能节省很多代码量
template<class OBJ, class TYPE=std::string>
class SelfRegisterFactoryManager {
public:
  ~SelfRegisterFactoryManager() {}
  
  // C++11的一个新特性：内部类可以通过外部类的实例访问外部类的私有成员
  // 所以RegisterTemplate可以直接访问SelfRegisterFactoryManager的私有变量factories_
  // 
  // 辅助自注册帮助类
  // 只有一个构造函数
  // 通过REGISTER_OBJ宏使用
  template<typename T>
  struct RegisterTemplate {
    RegisterTemplate(const TYPE& k) {
      auto& factories = SelfRegisterFactoryManager<OBJ, TYPE>::GetInstance().factories_;
      auto it = factories.find(k);
      if (it != factories.end()) {
        // TODO(@wubenqi): 是否需要抛出异常？
        LOG(ERROR) << "RegisterTemplate - duplicate entry for key: " << k;
      } else {
        // lambda表达式
        factories.emplace(k, [] { return new T(); });
      }
    }

    RegisterTemplate(const TYPE& k, std::function<T*()> new_func) {
      auto& factories = SelfRegisterFactoryManager<OBJ, TYPE>::GetInstance().factories_;
      auto it = factories.find(k);
      if (it != factories.end()) {
        // TODO(@wubenqi): 是否需要抛出异常？
        LOG(ERROR) << "RegisterTemplate - duplicate entry for key: " << k;
      } else {
        // lambda表达式
        factories.emplace(k, new_func);
      }
    }

//    template<typename... Args>
//    struct RegisterTemplate(const TYPE& k, Args... args) {
//      auto& factories = SelfRegisterFactoryManager<T, TYPE>::GetInstance().factories_;
//      auto it = factories.find(k);
//      if (it != factories.end()) {
//        // TODO(@wubenqi): 是否需要抛出异常？
//        LOG(ERROR) << "RegisterTemplate - duplicate entry for message_type: " << k;
//      } else {
//        // lambda表达式
//        factories.emplace(k, [&] { return new T(args...); });
//      }
//    }
  
    template<typename... Args>
    RegisterTemplate(const TYPE& k, std::function<T*(Args... args)> new_func, Args... args) {
      auto& factories = SelfRegisterFactoryManager<OBJ, TYPE>::GetInstance().factories_;
      auto it = factories.find(k);
      if (it != factories.end()) {
        // TODO(@wubenqi): 是否需要抛出异常？
        LOG(ERROR) << "RegisterTemplate - duplicate entry for key: " << k;
      } else {
        // lambda表达式
        factories.emplace(k, [&] { return new_func(args...); });
      }
    }
  };
  
  // 通过工厂创建裸指针
  static OBJ* CreateInstance(const TYPE& obj_type) {
    auto& factories = GetInstance().factories_;
    auto it = factories.find(obj_type);
    if (it == factories.end()) {
      LOG(ERROR) << "CreateInstance - not exist message_type: " << obj_type;
      return nullptr;
    } else {
      return it->second();
    }
  }
  
  // 智能指针
  static std::unique_ptr<OBJ> CreateUniqueInstance(const TYPE& obj_type) {
    return std::unique_ptr<OBJ>(CreateInstance(obj_type));
  }
  
  static std::shared_ptr<OBJ> CreateSharedInstance(const TYPE& obj_type) {
    return std::shared_ptr<OBJ>(CreateInstance(obj_type));
  }
  
private:
  SelfRegisterFactoryManager() = default;
  SelfRegisterFactoryManager(const SelfRegisterFactoryManager&) = delete;
  SelfRegisterFactoryManager(SelfRegisterFactoryManager&&) = delete;
  
  static SelfRegisterFactoryManager& GetInstance() {
    // 有人说：
    //  在C++11里这个方法还是线程安全的，
    //  因为C++11中静态局部变量的初始化是线程安全的。
    // 真的？查查C++11的手册
    static SelfRegisterFactoryManager g_obj_factorys;
    return g_obj_factorys;
  }
  
  std::unordered_map<TYPE, std::function<OBJ*()>> factories_;
};

// using MessageFactoryManager = SelfRegisterFactoryManager<BaseMessage, std::string>;
}

// 在派生类中调用宏注册自己
// 创建一个静态类
// 优点:
//  静态类的好处是不用在初始化时执行一个函数
// 缺点:
//  不符合google编程规范
//  更头痛的是这个宏还很难用，不能带命名空间，因为要通过(OBJ, TYPE, T, K)>拼出一个唯一的类名字

// K为字符串
// REGISTER_STRING_KEY_OBJECT(BaseMessage, Message1, message1);
#define REGISTER_STRING_KEY_OBJECT(OBJ, T, K) \
  static nebula::SelfRegisterFactoryManager<OBJ, std::string>::RegisterTemplate<T> g_reg_string_key_##K(#K)

// 数字形式
// REGISTER_STRING_KEY_OBJECT(BaseMessage, Message1, message1);
#define REGISTER_NUMBER_KEY_OBJECT(OBJ, TYPE, T, K) \
  static nebula::SelfRegisterFactoryManager<OBJ, TYPE>::RegisterTemplate<T> g_reg_number_key_##K(K)

#endif
