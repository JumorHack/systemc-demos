# demo_tlm2_gs2 -- TLM-2.0 "getting started" example 2.
# Same direct initiator/target topology as gs1, but adds:
#   - the Direct Memory Interface (get_direct_mem_ptr) and the DMI hint,
#     with periodic invalidate_direct_mem_ptr on the backward path
#   - the debug transport interface (transport_dbg)
#   - proper per-error response-status codes
# Split into initiator.h / target.h / top.h / testbench.cpp, following
# github.com/SingularityKChen/SystemC-Training.

# Allow this demo to be configured standalone, not only from the top-level project.
if(NOT TARGET SystemC::systemc)
  cmake_minimum_required(VERSION 3.16)
  project(demo_tlm2_gs2 CXX)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  list(APPEND CMAKE_PREFIX_PATH /opt/systemc)
  find_package(SystemCLanguage CONFIG REQUIRED)
endif()

add_executable(tlm2_gs2
    testbench.cpp
    initiator.h
    target.h
    top.h)

target_link_libraries(tlm2_gs2 PRIVATE SystemC::systemc)

# Uncomment to build the error-injection variant:
# target_compile_definitions(tlm2_gs2 PRIVATE INJECT_ERROR)
