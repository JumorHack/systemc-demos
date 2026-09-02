#ifndef MON_H
#define MON_H

#include <iomanip>
#include "systemc.h"

SC_MODULE(mon)             // prints the DUT signals on every falling clock edge
{
  sc_in<bool> A, B, F;
  sc_in<bool> Clk;

  void Monitor()
  {
    cout << std::setw(10) << "Time"
         << std::setw(2)  << "A"
         << std::setw(2)  << "B"
         << std::setw(2)  << "F" << endl;

    while (true)
    {
      wait();             // next Clk.neg(): the inputs applied on the previous
                          // posedge have propagated through the gate network
      cout << std::setw(10) << sc_time_stamp()
           << std::setw(2)  << A.read()
           << std::setw(2)  << B.read()
           << std::setw(2)  << F.read() << endl;
    }
  }

  SC_CTOR(mon)
  {
    SC_THREAD(Monitor);
    sensitive << Clk.neg();   // sample mid-cycle, after the gate network settles
  }
};

#endif // MON_H
