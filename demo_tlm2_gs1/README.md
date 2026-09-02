# demo_tlm2_gs1 — TLM-2.0 getting started (example 1)

The smallest useful TLM-2.0 model: **one initiator bound directly to one target**
(a simple memory), no bus in between, using the **loosely-timed** coding style —
blocking transport (`b_transport`) with the generic payload and a single
timing-annotation argument.

From the Doulos *TLM-2.0 Getting Started* tutorial (example 1), split into one
file per component following
[SingularityKChen/SystemC-Training](https://github.com/SingularityKChen/SystemC-Training).

## Files

| File | Contents |
|------|----------|
| [`initiator.h`](initiator.h) | `struct Initiator` — the socket, the `thread_process` that builds and sends transactions. |
| [`target.h`](target.h) | `struct Memory` — the socket, the `b_transport` callback, `int mem[256]`. Carries `#define SC_INCLUDE_DYNAMIC_PROCESSES`. |
| [`top.h`](top.h) | `SC_MODULE(Top)` — instantiates both and binds the sockets. |
| [`testbench.cpp`](testbench.cpp) | Doulos Apache-2.0 header + `sc_main`. |

## What it models

```
   Initiator                              Memory (target)
 ┌───────────┐   simple_initiator_socket   ┌──────────────┐
 │  thread   │  ───────── bind ─────────▶  │ b_transport()│  int mem[256]
 │  process  │      generic payload        │  callback    │
 └───────────┘                             └──────────────┘
```

- **Sockets** — `tlm_utils::simple_initiator_socket` / `simple_target_socket`
  (32-bit, base protocol). The target registers a plain method with
  `socket.register_b_transport(this, &Memory::b_transport)`; no `tlm_fw/bw_if`
  boilerplate.
- **Generic payload** (`tlm::tlm_generic_payload`) — command, address, data
  pointer, data length, streaming width, byte-enable pointer, DMI hint, response
  status. One payload object is **reused** across all calls.
- **`b_transport(trans, delay)`** — a blocking call. The `delay` argument carries
  timing *annotation*: the target may add to it instead of calling `wait()`. Here
  the memory ignores it, so it stays 10 ns; the initiator then does `wait(delay)`
  itself ("realising" the annotated delay).
- **Obligations** — the initiator must check `is_response_error()`; the target
  must range-check the address, reject unsupported features (byte enables,
  streaming, bursts > word), perform the read/write, and set
  `TLM_OK_RESPONSE`.

## Build & run

```sh
cmake -S . -B build -G Ninja && cmake --build build   # from repo root
./build/demo_tlm2_gs1/tlm2_gs1
```

## Expected output

A random (but fixed-seed, so reproducible) mix of 16 reads/writes over addresses
`0x20 … 0x5c`, one every 10 ns:

```
trans = { W, 20 } , data = ff000020 at time 0 s delay = 10 ns
trans = { W, 24 } , data = ff000024 at time 10 ns delay = 10 ns
...
trans = { R, 2c } , data = aa000055 at time 30 ns delay = 10 ns
...
trans = { W, 5c } , data = ff00005c at time 150 ns delay = 10 ns
```

Writes store `0xFF0000nn`; reads return the target's random init data
`0xAA0000nn`.

## Notes

- **`#define SC_INCLUDE_DYNAMIC_PROCESSES`** before `#include "systemc"` is
  mandatory — `simple_target_socket` spawns helper processes internally. It lives
  in [`target.h`](target.h), the file that pulls in `simple_target_socket.h`.
- Uses the namespaced headers (`#include "systemc"` + `using namespace sc_core;`
  …), unlike the non-TLM demos which use `systemc.h`. Both styles link against the
  same `SystemC::systemc` target.
- The `trans` payload and the `Top` sub-modules are `new`ed and never `delete`d —
  a small leak, kept as-is to match the reference example.
- This is *LT / example 1*. [`../demo_tlm2_gs2`](../demo_tlm2_gs2/) adds DMI, the
  debug transport interface, and proper response-status codes.
