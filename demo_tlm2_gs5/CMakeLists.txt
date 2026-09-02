# demo_tlm2_gs5 -- TLM-2.0 "getting started" example 5.
# Two loosely-timed initiators, each with temporal decoupling + a quantum
# keeper, talking through a Bus<N_INIT, N_TARG> to four memories:
#   - tagged sockets on both sides of the bus so a single module presents
#     many fw/bw interfaces
#   - address decode/mask forward, address recompose backward
#   - DMI propagated both ways; invalidate broadcast to every initiator
#   - one initiator uses an explicit transaction-pool memory manager
# Split into common.h / initiator1.h / initiator2.h / bus.h / memory.h /
# top.h / testbench.cpp, in the style of
# github.com/SingularityKChen/SystemC-Training.

# Allow this demo to be configured standalone, not only from the top-level project.
if(NOT TARGET SystemC::systemc)
  cmake_minimum_required(VERSION 3.16)
  project(demo_tlm2_gs5 CXX)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  list(APPEND CMAKE_PREFIX_PATH /opt/systemc)
  find_package(SystemCLanguage CONFIG REQUIRED)
endif()

add_executable(tlm2_gs5
    testbench.cpp
    common.h
    initiator1.h
    initiator2.h
    bus.h
    memory.h
    top.h)

target_link_libraries(tlm2_gs5 PRIVATE SystemC::systemc)
