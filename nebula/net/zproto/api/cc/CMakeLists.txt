include_directories(. .. ../../nebula)

set (SRC_LIST
 auth.pb.cc
 auth.pb.h
)

add_library(api_proto STATIC ${SRC_LIST})
target_link_libraries(api_proto protobuf)

