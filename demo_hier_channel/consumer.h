#ifndef CONSUMER_H
#define CONSUMER_H

#include "systemc.h"
#include "stack_if.h"

// Pops one character per slow-clock edge from whatever stack_read_if
// its `in` port is bound to.
class consumer : public sc_module
{
public:
  sc_port<stack_read_if> in;
  sc_in<bool>            Clock;

  void do_reads()
  {
    char c;
    while (true)
    {
      wait();                         // for clock edge
      if (in->nb_read(c))
        cout << "R " << c << " at "
             << sc_time_stamp() << endl;
    }
  }

  SC_CTOR(consumer)
  {
    SC_THREAD(do_reads);
    sensitive << Clock.pos();
  }
};

#endif // CONSUMER_H
