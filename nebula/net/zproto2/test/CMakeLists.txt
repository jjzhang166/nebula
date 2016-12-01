include_directories(/usr/local/include ../..)

add_executable (zproto_test zproto_test.cc)
target_link_libraries (zproto_test nebula)

add_executable (zproto_client_test zproto_client_test.cc)
target_link_libraries (zproto_client_test nebula)

add_executable (zproto_server_test zproto_server_test.cc)
target_link_libraries (zproto_server_test nebula)

