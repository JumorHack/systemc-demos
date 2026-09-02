#ifndef FIFO_H
#define FIFO_H

#include "systemc.h"
#include "fifo_if.h"

// A primitive channel: a drastically simplified, char-only, blocking version
// of sc_fifo. It derives from sc_prim_channel (the primitive-channel analogue
// of sc_module) and from the two interfaces, so ports bind straight to it.
//
// The evaluate/update split: write()/read() run in the evaluation phase and
// only call request_update(); the kernel then calls update() in the update
// phase, where the data_read_event / data_written_event notifications are
// posted to wake any process blocked in write()/read().
class fifo
: public sc_prim_channel,
  public fifo_out_if,
  public fifo_in_if
{
protected:
  int   size;            // buffer size
  char* buf;             // circular buffer
  int   freespace;       // free slots (updated eagerly by write/read)
  int   ri;              // read index
  int   wi;              // write index
  int   num_readable;    // slots readable at the start of this delta
  int   num_read;        // reads requested in this delta
  int   num_written;     // writes requested in this delta

  sc_event data_read_event;      // "a slot freed up"
  sc_event data_written_event;   // "a byte arrived"

public:
  explicit fifo(int size_ = 16)
  : sc_prim_channel(sc_gen_unique_name("myfifo"))
  {
    size = size_;
    buf  = new char[size];
    reset();
  }

  ~fifo() override
  {
    delete[] buf;
  }

  // ---- queries -----------------------------------------------------------
  int num_available() const override
  {
    return num_readable - num_read;
  }

  int num_free() const override
  {
    return size - num_readable - num_written;
  }

  // ---- blocking write --------------------------------------------------
  void write(char c) override
  {
    if (num_free() == 0)
      wait(data_read_event);       // dynamic sensitivity: block until a read frees a slot
    num_written++;
    buf[wi] = c;
    wi = (wi + 1) % size;
    freespace--;
    request_update();
  }

  // ---- blocking read -------------------------------------------------
  void read(char& c) override
  {
    if (num_available() == 0)
      wait(data_written_event);    // block until a write provides data
    num_read++;
    c = buf[ri];
    ri = (ri + 1) % size;
    freespace++;
    request_update();
  }

  char read() override             // shortcut: char c = port->read();
  {
    char c;
    read(c);
    return c;
  }

  void reset()
  {
    freespace    = size;
    ri           = 0;
    wi           = 0;
    num_readable = 0;
    num_read     = 0;
    num_written  = 0;
  }

  // ---- update phase --------------------------------------------------
  void update() override
  {
    if (num_read > 0)
      data_read_event.notify(SC_ZERO_TIME);     // delta notification
    if (num_written > 0)
      data_written_event.notify(SC_ZERO_TIME);

    num_readable = size - freespace;
    num_read     = 0;
    num_written  = 0;
  }
};

#endif // FIFO_H
