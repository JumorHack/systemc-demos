#ifndef PRODUCER_H
#define PRODUCER_H

#include <string>
#include "systemc.h"
#include "fifo_if.h"

// Pushes one character per fast-clock edge. out->write() is blocking, so once
// the FIFO fills up this thread suspends inside write() (dynamic sensitivity)
// until the consumer frees a slot -- not on the clock.
class producer : public sc_module
{
public:
  sc_port<fifo_out_if> out;
  sc_in<bool>          Clock;

  void do_writes()
  {
    const std::string msg = "The quick brown fox jumps over the lazy dog";
    std::size_t i = 0;
    while (true)
    {
      wait();                          // fast clock edge
      out->write(msg[i]);              // blocks here if the FIFO is full
      cout << "W '" << msg[i] << "'  free=" << out->num_free()
           << "  at " << sc_time_stamp() << endl;
      i = (i + 1) % msg.size();
    }
  }

  SC_CTOR(producer)
  {
    SC_THREAD(do_writes);
    sensitive << Clock.pos();
  }
};

#endif // PRODUCER_H
