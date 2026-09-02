# demo_tlm2_gs6 -- TLM-2.0 "getting started" example 6.
# Multi-sockets in an interconnect: multi_passthrough_initiator_socket /
# multi_passthrough_target_socket replace the tagged-socket arrays of gs5.
# Combines the approximately-timed initiator + target of gs4 with a bus:
# one targ_socket and one init_socket, each bound 4 times, 4 initiators and
# 4 targets, forward + backward nb_transport through the bus.
# Split into common.h / initiator.h / bus.h / target.h / top.h / testbench.cpp,
# in the style of github.com/SingularityKChen/SystemC-Training.

# Allow this demo to be configured standalone, not only from the top-level project.
if(NOT TARGET SystemC::systemc)
  cmake_minimum_required(VERSION 3.16)
  project(demo_tlm2_gs6 CXX)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  list(APPEND CMAKE_PREFIX_PATH /opt/systemc)
  find_package(SystemCLanguage CONFIG REQUIRED)
endif()

add_executable(tlm2_gs6
    testbench.cpp
    common.h
    initiator.h
    bus.h
    target.h
    top.h)

target_link_libraries(tlm2_gs6 PRIVATE SystemC::systemc)
