cmake_minimum_required(VERSION 2.6)
project(testtask)

add_executable(testtask main.cpp server.cpp connectionmanager.cpp connectionprocessor.cpp proxy.cpp forwarder.cpp config.cpp)
target_link_libraries(testtask event log4cpp)

install(TARGETS testtask RUNTIME DESTINATION bin)
