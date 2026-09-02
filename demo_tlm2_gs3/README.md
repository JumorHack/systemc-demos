# demo_tlm2_gs3 — TLM-2.0 getting started (example 3)

Adds an **interconnect component** — a `Router<N>` — between the initiator and
*N* targets. The router decodes the transaction address to pick a target, masks
the address into that target's local space, forwards the call, and does the
reverse translation on anything coming back.

From the Doulos *TLM-2.0 Getting Started* tutorial (example 3), split per
component following
[SingularityKChen/SystemC-Training](https://github.com/SingularityKChen/SystemC-Training).

## Files

| File | Contents |
|------|----------|
| [`initiator.h`](initiator.h) | `Initiator`: sweeps addresses `0xC0 … 0x13C`, so the run crosses a target boundary. DMI check now tests the transaction address against the *system-space* DMI region. Dumps two targets via `transport_dbg`. |
| [`router.h`](router.h) | `template<unsigned N_TARGETS> struct Router`: one `simple_target_socket`, `N` **tagged** `simple_initiator_socket_tagged`. Forward + backward paths, address translation both ways. |
| [`target.h`](target.h) | `Memory` (as in gs2) plus a `static mem_nr` that stamps each instance's data with its number, so the dump shows which physical memory a word came from. |
| [`top.h`](top.h) | 1 `Initiator`, 1 `Router<4>`, 4 `Memory`; binds initiator→router, router[i]→memory[i]. |
| [`testbench.cpp`](testbench.cpp) | Doulos Apache-2.0 header + `sc_main`. |

## Topology

```
                         ┌───────────────── Router<4> ─────────────────┐
   Initiator             │  target_socket        initiator_socket[0] ──┼──▶ memory_0  (sys 0x000–0x0FF)
 ┌───────────┐  b_transport  │  decode addr → target_nr = (A>>8)&3     │
 │  thread   │ ───────────▶  │  mask   addr → A & 0xFF                 ├──▶ memory_1  (sys 0x100–0x1FF)
 │  process  │ ◀───────────  │  forward to initiator_socket[target_nr] ├──▶ memory_2  (sys 0x200–0x2FF)
 └───────────┘  invalidate   │  tagged bw callback recomposes address  ├──▶ memory_3  (sys 0x300–0x3FF)
                (per target)  └────────────────────────────────────────┘
```

Address map: bits **[9:8]** select the target, bits **[7:0]** are the local
offset. Each target owns 256 bytes of the system map.

## What the router does

| Path | Method | Translation |
|------|--------|-------------|
| forward | `b_transport` | `set_address(A & 0xFF)`, forward to `initiator_socket[(A>>8)&3]` |
| forward | `get_direct_mem_ptr` | mask address, forward; then **recompose** the granted `tlm_dmi` start/end back to system space so the initiator's DMI test works on raw addresses |
| forward | `transport_dbg` | mask address, forward, return byte count |
| backward | `invalidate_direct_mem_ptr(id, s, e)` | the **tag `id`** says which target called; recompose `[s,e]` into system space and pass up through `target_socket` |

Tagged sockets (`simple_initiator_socket_tagged`) are what make the backward path
work: with plain sockets the router could not tell which of its 4 downstream
targets fired the invalidate.

## Build & run

```sh
cmake -S . -B build -G Ninja && cmake --build build   # from repo root
./build/demo_tlm2_gs3/tlm2_gs3
```

## Expected output (abridged)

```
trans = { R, c0 } , data = aa000002 at time 10 ns      <- slow path: acquire DMI for target 0
DMI   = { R, c4 } , data = aa000035 at time 20 ns      <- fast path within target 0 (sys 0x000–0x0FF)
...
DMI   = { R, fc } , data = aa000048 at time 160 ns
trans = { R, 100 } , data = aa100093 at time 160 ns    <- crossed into target 1: DMI region no longer covers A, slow path again
DMI   = { R, 104 } , data = aa1000b3 at time 170 ns    <- fast path within target 1 (sys 0x100–0x1FF)
...
mem[80] = aa000048        <- transport_dbg dump of target 0 (A=0x80, 256 B)
...
mem[100] = aa100093       <- transport_dbg dump of target 1 (A=0x100, 128 B)
```

The `aa0…` vs `aa1…` prefix on the two dumps proves they hit **different physical
memories** — the `mem_nr` stamp — while the initiator only ever used system
addresses.

## Changes vs. the reference snippet

- `sprintf` → `snprintf(…, sizeof …, …)` in `initiator.h`, `router.h`, `top.h`
  (`sprintf` is deprecated by the platform toolchain).
- `delete[]` the debug-dump buffer.
- `compose_address` shifts a `sc_dt::uint64` (not a 32-bit `unsigned`) before the OR.
- Unused callback parameters cast to `void`; self-contained
  `<cassert>`/`<cstdio>`/`<cstring>`; include guards.
- Dropped a stray leading `#` on `testbench.cpp`'s first line.
