#ifndef STACK_IF_H
#define STACK_IF_H

#include "systemc.h"

// Write side of the channel: push a character, or clear the stack.
class stack_write_if : virtual public sc_interface
{
public:
  virtual bool nb_write(char c) = 0;
  virtual void reset()          = 0;
};

// Read side of the channel: pop a character.
class stack_read_if : virtual public sc_interface
{
public:
  virtual bool nb_read(char& c) = 0;
};

#endif // STACK_IF_H
