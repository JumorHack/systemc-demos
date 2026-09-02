#ifndef FIFO_IF_H
#define FIFO_IF_H

#include "systemc.h"

// Write side: blocking write, plus a query for free space.
class fifo_out_if : virtual public sc_interface
{
public:
  virtual void write(char) = 0;       // blocking write
  virtual int  num_free() const = 0;  // free entries

protected:
  fifo_out_if() = default;

private:
  fifo_out_if(const fifo_out_if&) = delete;             // non-copyable
  fifo_out_if& operator=(const fifo_out_if&) = delete;
};

// Read side: blocking read (two forms), plus a query for pending data.
class fifo_in_if : virtual public sc_interface
{
public:
  virtual void read(char&) = 0;            // blocking read
  virtual char read()      = 0;            // blocking read, shortcut form
  virtual int  num_available() const = 0;  // readable entries

protected:
  fifo_in_if() = default;

private:
  fifo_in_if(const fifo_in_if&) = delete;             // non-copyable
  fifo_in_if& operator=(const fifo_in_if&) = delete;
};

#endif // FIFO_IF_H
