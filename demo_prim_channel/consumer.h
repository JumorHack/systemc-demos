#ifndef CONSUMER_H
#define CONSUMER_H

#include "systemc.h"
#include "fifo_if.h"

// Pops one character per slow-clock edge. in->read() is blocking, so if the
// FIFO is empty this thread suspends inside read() until the producer writes.
class consumer : public sc_module
{
public:
  sc_port<fifo_in_if> in;
  sc_in<bool>         Clock;

  void do_reads()
  {
    while (true)
    {
      wait();                          // slow clock edge
      char c = in->read();             // shortcut form; blocks if the FIFO is empty
      cout << "        R '" << c << "'  avail=" << in->num_available()
           << "  at " << sc_time_stamp() << endl;
    }
  }

  SC_CTOR(consumer)
  {
    SC_THREAD(do_reads);
    sensitive << Clock.pos();
  }
};

#endif // CONSUMER_H
