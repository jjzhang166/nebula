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

#ifndef NEBULA_BASE_CLUSTER_NODE_MANAGER_H_
#define NEBULA_BASE_CLUSTER_NODE_MANAGER_H_

#include <string>

namespace nebula {
  
/**
 idc:     留下
 物理机:   test001/192.168.0.1
 集群:     线上,测试,灰度
 应用类别: 测试客户端,接入层,路由层,业务层,状态
 实例名:     frontd
 实例编号:   1
 集群唯一索引ID: 1
 */
  
/**
    // 配置，维护即将部署的所有物理节点和应用，程序里可以通过本机信息获取所有信息
    // 物理机节点信息
    {
        "idc" = "留下",
        "host" = "test001",
        "ip" = "192.168.0.1",
    }

    // 应用信息
    {
        "cluster" = "线上",
        "app_type" = "接入",
        "app_name" = "frontd",
    }
 
    // 部署实例
    {
        "app_idx" = 1,
        "cluster_idx" = 1,
    }
 */


// 集群节点命名
// 一个集群部署在一个idc中
// 每个idc有n多机器
//
struct ClusterNode {
    std::string idc;            // 机房名
    std::string host_name;      // 主机名
    std::string ip;             // IP地址，使用内网IP标识
    
    std::string cluster;        // 集群名（灰度、线上、测试等）
    std::string group;          // 组名
    std::string app;            // 应用名，比如：frontd
    std::string app_idx;        // 应用索引
    std::string cluster_idx;    // 整个集群里的唯一索引
};

// 机房.集群.组.服务名.编号
const std::string& GetRuntimeNodeID();
const char* GetRuntimeNodeID2();

}

#endif
