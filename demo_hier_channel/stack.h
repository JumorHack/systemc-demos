#ifndef STACK_H
#define STACK_H

#include "systemc.h"
#include "stack_if.h"

// A *hierarchical channel*: `stack` is an sc_module AND it implements the
// stack_write_if / stack_read_if interfaces. Because it derives from the
// interfaces, an sc_port<stack_write_if> / sc_port<stack_read_if> can bind
// straight to it -- no separate primitive channel needed.
class stack
: public sc_module,
  public stack_write_if,
  public stack_read_if
{
private:
  char data[20];
  int  top;                 // index of the next free slot (0..20)

public:
  stack(sc_module_name nm) : sc_module(nm), top(0)
  {
  }

  // ---- stack_write_if ----
  bool nb_write(char c)
  {
    if (top < 20)
    {
      data[top++] = c;
      return true;
    }
    return false;           // stack full
  }

  void reset()
  {
    top = 0;
  }

  // ---- stack_read_if ----
  bool nb_read(char& c)
  {
    if (top > 0)
    {
      c = data[--top];       // LIFO -> the output comes out reversed
      return true;
    }
    return false;           // stack empty
  }

  // Called by the kernel every time a port is bound to this channel.
  void register_port(sc_port_base& port_, const char* if_typename_)
  {
    cout << "binding    " << port_.name() << " to "
         << "interface: " << if_typename_ << endl;
  }
};

#endif // STACK_H
