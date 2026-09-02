# demo_hier_channel — a hierarchical channel

Shows how a **module can also be a channel** in SystemC. `stack` derives from
`sc_module` *and* from two interfaces, so `sc_port<IF>` on other modules can bind
straight to it — no separate primitive channel required.

## Files

| File | Contents |
|------|----------|
| [`stack_if.h`](stack_if.h) | `stack_write_if` (`nb_write`, `reset`) and `stack_read_if` (`nb_read`), both `virtual public sc_interface`. |
| [`stack.h`](stack.h) | `class stack : public sc_module, public stack_write_if, public stack_read_if` — a 20-entry LIFO. Also overrides `register_port()` to log every binding. |
| [`producer.h`](producer.h) | `sc_port<stack_write_if> out`. Pushes one character of a test string per `ClkFast` edge. |
| [`consumer.h`](consumer.h) | `sc_port<stack_read_if> in`. Pops one character per `ClkSlow` edge. |
| [`main.cpp`](main.cpp) | Two clocks, one `stack`, one `producer`, one `consumer`; `P1.out(Stack1)` and `C1.in(Stack1)` bind both ports to the same channel. |

## Why it's a "hierarchical" channel

A *primitive* channel (`sc_signal`, `sc_fifo`, …) derives only from `sc_interface`
and cannot contain processes or sub-modules. A *hierarchical* channel is a full
`sc_module` that additionally implements one or more interfaces — so it can have
its own ports, processes and structure while still being the thing a port binds to.

```
 producer                       consumer
 ┌─────────┐   sc_port           ┌─────────┐
 │  out  ●─┼──────────┐   ┌──────┼─● in     │
 └─────────┘          ▼   ▼      └─────────┘
                  ┌─────────────┐
                  │   stack     │  sc_module + stack_write_if + stack_read_if
                  │  (20 × char) │
                  └─────────────┘
```

The kernel calls `stack::register_port()` once per binding during elaboration —
that's the first two lines of output.

## Build & run

```sh
cmake -S . -B build -G Ninja && cmake --build build   # from repo root
./build/demo_hier_channel/stack_tb
```

## Expected output (head)

```
binding    C1.port_0 to interface: 13stack_read_if
binding    P1.port_0 to interface: 14stack_write_if
W H at 0 s
R H at 50 ns
W a at 100 ns
R a at 100 ns
...
```

`13stack_read_if` is the length-prefixed mangled interface name.

## Note on the clock values

`main.cpp` uses `ClkFast` = **100 ns** period and `ClkSlow` = **50 ns** period, so
the consumer actually drains faster than the producer fills. The stack never builds
up depth, so the LIFO "reversal" is not visible — each char pushed is popped on the
next read. To see the reversal, make the producer clock faster than the consumer's,
e.g.:

```cpp
sc_clock ClkFast("ClkFast", 20, SC_NS);   // producer
sc_clock ClkSlow("ClkSlow", 50, SC_NS);   // consumer
```

## Bugs fixed vs. the original snippet

- Member functions were defined inside the class body with a `stack::` qualifier
  (`bool stack::nb_write(...)`) — illegal C++ (`extra qualification`). Qualifier removed.
- `char* TestString = "..."` — a string literal cannot bind to a non-`const char*`
  in C++11+. Changed to `const char*`.
- Added include guards; added the missing `consumer.h` and `stack_if.h`.
