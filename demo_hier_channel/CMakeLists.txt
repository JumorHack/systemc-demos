# demo_hier_channel -- SystemC hierarchical channel.
# `stack` is an sc_module that also implements stack_write_if / stack_read_if;
# producer and consumer bind to it through sc_port<IF>.

# Allow this demo to be configured standalone, not only from the top-level project.
if(NOT TARGET SystemC::systemc)
  cmake_minimum_required(VERSION 3.16)
  project(demo_hier_channel CXX)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  list(APPEND CMAKE_PREFIX_PATH /opt/systemc)
  find_package(SystemCLanguage CONFIG REQUIRED)
endif()

add_executable(stack_tb
    main.cpp
    stack_if.h
    stack.h
    producer.h
    consumer.h)

target_link_libraries(stack_tb PRIVATE SystemC::systemc)
