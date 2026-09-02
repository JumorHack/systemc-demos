#ifndef COMMON_H
#define COMMON_H

// Needed for the simple_target_socket
#define SC_INCLUDE_DYNAMIC_PROCESSES

#include <cstdio>
#include <fstream>

#include "systemc"
using namespace sc_core;
using namespace sc_dt;
using namespace std;

#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
#include "tlm_utils/peq_with_cb_and_phase.h"

// Enables the allocate/free counters and per-phase trace lines
#define DEBUG

// All trace output goes here -- the AT protocol chatter for 1000 transactions
// is far too much for stdout.
inline ofstream fout("foo.txt");

// **************************************************************************************
// User-defined memory manager, which maintains a pool of transactions
// **************************************************************************************

class mm: public tlm::tlm_mm_interface
{
  typedef tlm::tlm_generic_payload gp_t;

public:
  mm() : free_list(0), empties(0)
  #ifdef DEBUG
  , count(0)
  #endif
  {}

  gp_t* allocate();
  void  free(gp_t* trans);

private:
  struct access
  {
    gp_t* trans;
    access* next;
    access* prev;
  };

  access* free_list;
  access* empties;

  #ifdef DEBUG
  int     count;
  #endif
};

inline mm::gp_t* mm::allocate()
{
  #ifdef DEBUG
    fout << "----------------------------- Called allocate(), #trans = " << ++count << endl;
  #endif
  gp_t* ptr;
  if (free_list)
  {
    ptr = free_list->trans;
    empties = free_list;
    free_list = free_list->next;
  }
  else
  {
    ptr = new gp_t(this);
  }
  return ptr;
}

inline void mm::free(gp_t* trans)
{
  #ifdef DEBUG
    fout << "----------------------------- Called free(), #trans = " << --count << endl;
  #endif
  if (!empties)
  {
    empties = new access;
    empties->next = free_list;
    empties->prev = 0;
    if (free_list)
      free_list->prev = empties;
  }
  free_list = empties;
  free_list->trans = trans;
  empties = free_list->prev;
}

// Generate a random delay (with power-law distribution) to aid testing and stress the protocol
inline int rand_ps()
{
  int n = rand() % 100;
  n = n * n * n;
  return n / 100;
}

#endif // COMMON_H
