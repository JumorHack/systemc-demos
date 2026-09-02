cmake_minimum_required(VERSION 3.16)
project(sc_demo CXX)

# ---------------------------------------------------------------------------
# Shared configuration for every SystemC demo in this tree
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)          # SystemC 3.0.x needs at least C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Accellera SystemC (local install). Override with -DCMAKE_PREFIX_PATH=...
list(APPEND CMAKE_PREFIX_PATH /opt/systemc)
find_package(SystemCLanguage CONFIG REQUIRED)

# ---------------------------------------------------------------------------
# Auto-register every immediate subdirectory that ships its own CMakeLists.txt
# Drop a new demo folder next to this file and re-run cmake -- it gets picked up.
# ---------------------------------------------------------------------------
file(GLOB _entries CONFIGURE_DEPENDS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
     ${CMAKE_CURRENT_SOURCE_DIR}/*)
foreach(_entry ${_entries})
  if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${_entry}
     AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${_entry}/CMakeLists.txt)
    message(STATUS "sc_demo: adding demo '${_entry}'")
    add_subdirectory(${_entry})
  endif()
endforeach()
