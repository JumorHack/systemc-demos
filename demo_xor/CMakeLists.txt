# demo_xor -- a 2-input XOR built from four NAND gates, plus a clocked testbench
# (stimulus + monitor) and a VCD waveform dump.

# Allow this demo to be configured standalone, not only from the top-level project.
if(NOT TARGET SystemC::systemc)
  cmake_minimum_required(VERSION 3.16)
  project(demo_xor CXX)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  list(APPEND CMAKE_PREFIX_PATH /opt/systemc)
  find_package(SystemCLanguage CONFIG REQUIRED)
endif()

add_executable(exor2_tb
    main.cpp
    stim.h
    exor2.h
    nand2.h
    mon.h)

target_link_libraries(exor2_tb PRIVATE SystemC::systemc)
