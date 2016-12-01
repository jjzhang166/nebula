#  Copyright (c) 2016, https://github.com/nebula-im/nebula
#  All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

include_directories(/usr/local/include ../..)

add_executable (zproto_test zproto_test.cc)
target_link_libraries (zproto_test nebula-net nebula-base)

add_executable (zproto_client_test zproto_client_test.cc)
target_link_libraries (zproto_client_test nebula-net nebula-base)

add_executable (zproto_server_test zproto_server_test.cc)
target_link_libraries (zproto_server_test nebula-net nebula-base)

