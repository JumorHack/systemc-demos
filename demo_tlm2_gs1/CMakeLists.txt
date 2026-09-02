# demo_tlm2_gs1 -- TLM-2.0 "getting started" example 1.
# One initiator bound directly to one target (memory), loosely-timed style:
# blocking transport (b_transport) with the generic payload and a single
# timing-annotation argument. Split into initiator.h / target.h / top.h /
# testbench.cpp, following github.com/SingularityKChen/SystemC-Training.

# Allow this demo to be configured standalone, not only from the top-level project.
if(NOT TARGET SystemC::systemc)
  cmake_minimum_required(VERSION 3.16)
  project(demo_tlm2_gs1 CXX)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  list(APPEND CMAKE_PREFIX_PATH /opt/systemc)
  find_package(SystemCLanguage CONFIG REQUIRED)
endif()

# TLM-2.0 ships as headers inside the SystemC include dir, so linking
# SystemC::systemc is all that is needed.
add_executable(tlm2_gs1
    testbench.cpp
    initiator.h
    target.h
    top.h)

target_link_libraries(tlm2_gs1 PRIVATE SystemC::systemc)
