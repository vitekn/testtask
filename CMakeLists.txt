cmake_minimum_required(VERSION 2.6)
project(testtask)

set (build_dir ${CMAKE_SOURCE_DIR}/build)
include_directories(./include)

SET (EXECUTABLE_OUTPUT_PATH ${build_dir})
add_executable(testtask main.cpp server.cpp connectionmanager.cpp connectionprocessor.cpp proxy.cpp forwarder.cpp config.cpp proxymanager.cpp controller.cpp jsonparser.cpp controlchannel.cpp)
target_link_libraries(testtask event log4cpp jsoncpp ${CMAKE_SOURCE_DIR}/libs/libamqpcpp.a pthread dl)

install(TARGETS testtask DESTINATION /output)
