#ifndef STIM_H
#define STIM_H

#include "systemc.h"

SC_MODULE(stim)            // drives the DUT inputs, one vector per clock
{
  sc_out<bool> A, B;
  sc_in<bool>  Clk;

  void StimGen()
  {
    A.write(false); B.write(false); wait();
    A.write(false); B.write(true);  wait();
    A.write(true);  B.write(false); wait();
    A.write(true);  B.write(true);  wait();
    sc_stop();             // stop the simulation
  }

  SC_CTOR(stim)
  {
    SC_THREAD(StimGen);
    sensitive << Clk.pos();
    dont_initialize();       // start on the first real clock edge, not at t=0
  }
};

#endif // STIM_H
