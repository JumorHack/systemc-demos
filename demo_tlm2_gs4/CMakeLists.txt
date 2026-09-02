# demo_tlm2_gs4 -- TLM-2.0 "getting started" example 4.
# Approximately-timed coding style: the non-blocking transport interface
# (nb_transport_fw / nb_transport_bw) with the 4-phase base protocol
# (BEGIN_REQ / END_REQ / BEGIN_RESP / END_RESP), payload event queues on both
# sides, a 2-deep pipelined target with the BEGIN_REQ / BEGIN_RESP exclusion
# rules, and an explicit memory manager with reference counting.
# Split into common.h / initiator.h / target.h / top.h / testbench.cpp,
# in the style of github.com/SingularityKChen/SystemC-Training.

# Allow this demo to be configured standalone, not only from the top-level project.
if(NOT TARGET SystemC::systemc)
  cmake_minimum_required(VERSION 3.16)
  project(demo_tlm2_gs4 CXX)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  list(APPEND CMAKE_PREFIX_PATH /opt/systemc)
  find_package(SystemCLanguage CONFIG REQUIRED)
endif()

add_executable(tlm2_gs4
    testbench.cpp
    common.h
    initiator.h
    target.h
    top.h)

target_link_libraries(tlm2_gs4 PRIVATE SystemC::systemc)
