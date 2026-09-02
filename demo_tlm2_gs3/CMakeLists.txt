# demo_tlm2_gs3 -- TLM-2.0 "getting started" example 3.
# Adds a Router<N> interconnect between the initiator and N targets:
#   - forward path: b_transport / get_direct_mem_ptr / transport_dbg decode the
#     address (bits [9:8] pick a target), mask it to the target's local space,
#     and forward
#   - backward path: tagged invalidate_direct_mem_ptr recomposes the address
#     back into system space
# Split into initiator.h / router.h / target.h / top.h / testbench.cpp,
# following github.com/SingularityKChen/SystemC-Training.

# Allow this demo to be configured standalone, not only from the top-level project.
if(NOT TARGET SystemC::systemc)
  cmake_minimum_required(VERSION 3.16)
  project(demo_tlm2_gs3 CXX)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  list(APPEND CMAKE_PREFIX_PATH /opt/systemc)
  find_package(SystemCLanguage CONFIG REQUIRED)
endif()

add_executable(tlm2_gs3
    testbench.cpp
    initiator.h
    router.h
    target.h
    top.h)

target_link_libraries(tlm2_gs3 PRIVATE SystemC::systemc)

# Uncomment to build the error-injection variant:
# target_compile_definitions(tlm2_gs3 PRIVATE INJECT_ERROR)
