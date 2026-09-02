# demo_tlm2_gs5 — TLM-2.0 getting started (example 5)

The full loosely-timed picture: **two initiators**, each with **temporal
decoupling + a quantum keeper**, talking through a **`Bus<N_INIT, N_TARG>`** to
**four memories**. DMI is propagated both ways (with `invalidate` broadcast to
every initiator), and one initiator uses an explicit transaction-pool memory
manager.

From the Doulos *TLM-2.0 Getting Started* example 5, split per component in the
style of
[SingularityKChen/SystemC-Training](https://github.com/SingularityKChen/SystemC-Training).

## Files

| File | Contents |
|------|----------|
| [`common.h`](common.h) | shared includes, `RUN_LENGTH`, and `class mm` — the transaction-pool memory manager. |
| [`initiator1.h`](initiator1.h) | `Initiator1`: **writes** all 4 memories, uses **DMI** + **debug transport** (`dump()`), a quantum keeper, **no** explicit mm. |
| [`initiator2.h`](initiator2.h) | `Initiator2`: **reads** all 4 memories, uses an explicit **`mm` pool** + `acquire()`/`release()`, a quantum keeper, no DMI/debug. |
| [`bus.h`](bus.h) | `template<unsigned N_INITIATORS, unsigned N_TARGETS> struct Bus`: **tagged** target + initiator sockets, address decode/mask forward, recompose backward, DMI both ways, invalidate **broadcast**. |
| [`memory.h`](memory.h) | `Memory` (b_transport / DMI / debug), `delay += LATENCY` temporal decoupling, a static `mem_nr` stamp, and a one-shot `invalidation_process`. |
| [`top.h`](top.h) | `SC_MODULE(Top)` — 2 initiators, `Bus<2,4>`, 4 memories. |
| [`testbench.cpp`](testbench.cpp) | Doulos Apache-2.0 header + `sc_main`. |

## Topology

```
 Initiator1 (writes) ──▶ targ_socket[0] ┐
                                        │  Bus<2,4>            init_socket[0] ──▶ memory_0  sys 0x00–0x3F
 Initiator2 (reads)  ──▶ targ_socket[1] ┤  decode (A>>6)&3    init_socket[1] ──▶ memory_1  sys 0x40–0x7F
                                        │  mask   A & 0x3F    init_socket[2] ──▶ memory_2  sys 0x80–0xBF
                                        ┘  (tagged both ends) init_socket[3] ──▶ memory_3  sys 0xC0–0xFF
```

Address map: bits **[7:6]** select the memory, bits **[5:0]** are the local
offset. Each memory owns 64 bytes of the system map.

## Temporal decoupling + quantum keeper

Each initiator carries a `tlm_utils::tlm_quantumkeeper` with a **1 µs global
quantum**. Per iteration:

```cpp
delay = m_qk.get_local_time();       // how far this initiator is "ahead"
socket->b_transport(trans, delay);   // target does delay += LATENCY (10 ns)
m_qk.set(delay);
m_qk.inc( sc_time(100, SC_NS) );     // model extra processing
if (m_qk.need_sync()) m_qk.sync();   // once the 1 µs budget is spent -> real wait()
```

So many transactions execute at the **same `sc_time_stamp()`** while `delay`
climbs (10 → 120 → 230 → … ns); only when the 1 µs quantum is exhausted does the
initiator do a real `wait` and simulation time jump forward. Big speed-up, at
the cost of the two initiators seeing a slightly stale view of each other.

## The bus: tagged sockets

A single `Bus` module needs *N* target-side interfaces and *N* initiator-side
interfaces. `simple_target_socket_tagged` / `simple_initiator_socket_tagged` give
each callback an extra `int id`, so one set of methods (`b_transport`,
`nb_transport_fw/bw`, `get_direct_mem_ptr`, `transport_dbg`,
`invalidate_direct_mem_ptr`) serves every port.

| Path | What the bus does |
|------|-------------------|
| forward `b_transport` / `transport_dbg` / `get_direct_mem_ptr` | `decode_address` → target, `set_address(A & 0x3F)`, forward; restore the original address afterwards |
| forward DMI | additionally **recompose** the granted region into system space (`(target<<6) | offset`) |
| backward `nb_transport_bw` | look up the originating initiator in `m_id_map`, recompose address, forward up |
| backward `invalidate_direct_mem_ptr` | recompose the range, then **broadcast to all `N_INITIATORS`** |

## DMI + broadcast invalidate

- First write to a memory goes through `b_transport`; the target sets
  `dmi_allowed`, the initiator calls `get_direct_mem_ptr`, the bus recomposes the
  region → subsequent writes are `WRITE/DMI` (straight `memcpy`, no bus, no target).
- At t = 3 µs every `Memory::invalidation_process` fires once. The bus turns each
  target-local range into a system range and calls `invalidate_direct_mem_ptr` on
  **both** initiators — Initiator1 prints four `INVALIDATE DMI (...)` lines and
  falls back to `b_transport` to re-acquire.

## Build & run

```sh
cmake -S . -B build -G Ninja && cmake --build build   # from repo root
./build/demo_tlm2_gs5/tlm2_gs5
```

## Expected output (abridged)

```
Dump memories at time 0 s
mem[0] = aa0000a7      <- memory_0 (aa0…), memory_1 = aa1…, memory_2 = aa2…, memory_3 = aa3…
...
WRITE     addr = 0, data = 0 at 0 s delay = 10 ns          <- b_transport, DMI granted
WRITE/DMI addr = 4, data = 4 at 0 s delay = 120 ns         <- DMI fast path, local time climbing
...
WRITE/DMI addr = 24, data = 24 at 0 s delay = 1 us         <- quantum spent
READ     addr = 0, data = 0 at 1 us delay = 10 ns          <- Initiator2 catches up, reads back
...
INVALIDATE DMI (c0..ff) for Initiator1 at 3 us             <- broadcast, one range per memory
INVALIDATE DMI (40..7f) for Initiator1 at 3 us
INVALIDATE DMI (80..bf) for Initiator1 at 3 us
INVALIDATE DMI (0..3f) for Initiator1 at 3 us
WRITE     addr = 70, data = 70 at 3080 ns delay = 10 ns    <- back to b_transport to re-acquire
```

## Changes vs. the reference snippet

- `sprintf` → `snprintf(txt, sizeof txt, …)` in `bus.h` (×2) and `top.h`.
- `Memory::b_transport` bound check `adr > SIZE` → `adr >= SIZE` (latent
  off-by-one; never triggered by this 64-byte address map, fixed anyway).
- `compose_address` shifts a `sc_dt::uint64` before the OR; signed/unsigned
  `id < N_*` comparisons made explicit.
- `mm::allocate` / `mm::free` are `inline`; `Memory::mem_nr` is an `inline`
  variable; unused callback params cast to `void`; `delete trans` at the end of
  `Initiator1::thread_process`; include guards + self-contained includes.
