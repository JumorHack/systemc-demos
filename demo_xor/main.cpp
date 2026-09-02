#include "systemc.h"
#include "stim.h"
#include "exor2.h"
#include "mon.h"

int sc_main(int argc, char* argv[])
{
  sc_signal<bool> ASig, BSig, FSig;
  sc_clock TestClk("TestClock", 10, SC_NS, 0.5);

  stim Stim1("Stimulus");
  Stim1.A(ASig);
  Stim1.B(BSig);
  Stim1.Clk(TestClk);

  exor2 DUT("exor2");
  DUT.A(ASig);
  DUT.B(BSig);
  DUT.F(FSig);

  mon Monitor1("Monitor");
  Monitor1.A(ASig);
  Monitor1.B(BSig);
  Monitor1.F(FSig);
  Monitor1.Clk(TestClk);

  // Optional VCD waveform dump -> exor2.vcd
  sc_trace_file* tf = sc_create_vcd_trace_file("exor2");
  sc_trace(tf, ASig, "A");
  sc_trace(tf, BSig, "B");
  sc_trace(tf, FSig, "F");

  sc_start();               // runs until stim.h calls sc_stop()

  sc_close_vcd_trace_file(tf);
  return 0;
}
