#include "systemc.h"
#include "producer.h"
#include "consumer.h"
#include "fifo.h"

int sc_main(int argc, char* argv[])
{
  sc_clock ClkFast("ClkFast", 1,   SC_NS);   // producer: one write attempt / ns
  sc_clock ClkSlow("ClkSlow", 500, SC_NS);   // consumer: one read / 500 ns

  fifo fifo1;                 // no name needed -- sc_gen_unique_name() inside

  producer P1("P1");
  P1.out(fifo1);             // sc_port<fifo_out_if> -> primitive channel
  P1.Clock(ClkFast);

  consumer C1("C1");
  C1.in(fifo1);              // sc_port<fifo_in_if>  -> same channel
  C1.Clock(ClkSlow);

  sc_start(5000, SC_NS);

  return 0;
}
