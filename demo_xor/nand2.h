#ifndef NAND2_H
#define NAND2_H

#include "systemc.h"

SC_MODULE(nand2)            // 2-input NAND gate
{
  sc_in<bool>  A, B;       // input signal ports
  sc_out<bool> F;          // output signal port

  void do_nand2()          // combinational behaviour
  {
    F.write( !(A.read() && B.read()) );
  }

  SC_CTOR(nand2)           // constructor for nand2
  {
    std::cout << "Constructing nand2 " << name() << endl;
    SC_METHOD(do_nand2);   // register do_nand2 with the kernel
    sensitive << A << B;   // sensitivity list
  }
};

#endif // NAND2_H
