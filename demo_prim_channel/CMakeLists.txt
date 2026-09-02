# demo_prim_channel -- SystemC primitive channel.
# `fifo` is an sc_prim_channel implementing fifo_out_if / fifo_in_if, a
# simplified blocking FIFO that shows the evaluate/update split, request_update()
# / update(), and blocking via dynamic sensitivity (wait(sc_event&)).

# Allow this demo to be configured standalone, not only from the top-level project.
if(NOT TARGET SystemC::systemc)
  cmake_minimum_required(VERSION 3.16)
  project(demo_prim_channel CXX)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  list(APPEND CMAKE_PREFIX_PATH /opt/systemc)
  find_package(SystemCLanguage CONFIG REQUIRED)
endif()

add_executable(fifo_tb
    main.cpp
    fifo_if.h
    fifo.h
    producer.h
    consumer.h)

target_link_libraries(fifo_tb PRIVATE SystemC::systemc)
