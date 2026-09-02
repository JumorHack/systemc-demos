# demo_tlm2_gs2 — TLM-2.0 getting started (example 2)

Same direct initiator → target (memory) topology as
[`../demo_tlm2_gs1`](../demo_tlm2_gs1/), but now the model shows the three things
a realistic loosely-timed model needs beyond plain `b_transport`:

1. **Direct Memory Interface (DMI)** + the DMI hint
2. **Debug transport interface** (`transport_dbg`)
3. **Proper response-status codes**

From the Doulos *TLM-2.0 Getting Started* tutorial (example 2), split per
component following
[SingularityKChen/SystemC-Training](https://github.com/SingularityKChen/SystemC-Training).

## Files

| File | Contents |
|------|----------|
| [`initiator.h`](initiator.h) | `Initiator`: tries DMI first, falls back to `b_transport`; registers `invalidate_direct_mem_ptr`; dumps memory at the end via `transport_dbg`. |
| [`target.h`](target.h) | `Memory`: `b_transport` with per-error status, `get_direct_mem_ptr`, `transport_dbg`, plus a thread that periodically invalidates the DMI region. |
| [`top.h`](top.h) | `SC_MODULE(Top)` — instantiate both, bind sockets. |
| [`testbench.cpp`](testbench.cpp) | Doulos Apache-2.0 header + `sc_main`. |

## 1. DMI — bypassing the transport call

```
   Initiator                                   Memory
 ┌───────────┐                              ┌──────────────┐
 │           │  ── b_transport ──────────▶  │ (slow path)  │
 │  thread   │  ◀─ trans.set_dmi_allowed(true)              │
 │  process  │                              │              │
 │           │  ── get_direct_mem_ptr ───▶  │ returns raw  │
 │           │  ◀───── tlm_dmi ────────────  │ &mem[0] + latencies
 │           │                              │              │
 │  memcpy() directly into mem[], then      │              │
 │  wait(latency)  ── no call into target   │              │
 │           │  ◀─ invalidate_direct_mem_ptr(range) ── (backward path)
 └───────────┘                              └──────────────┘
```

- On a normal `b_transport`, the target sets `trans.set_dmi_allowed(true)`.
- The initiator then calls `socket->get_direct_mem_ptr(trans, dmi_data)`; the
  target fills a `tlm::tlm_dmi` with a raw pointer to `mem[0]`, the address
  range, and read/write latencies.
- While `dmi_ptr_valid`, the initiator `memcpy`s straight into `mem[]` and only
  does `wait(latency)` — **the target's `b_transport` is never entered**. Much
  faster.
- `Memory::invalidation_process` calls `socket->invalidate_direct_mem_ptr(...)`
  every `8 × LATENCY` (four times). That runs the initiator's backward-path
  callback, which clears `dmi_ptr_valid`, so the next access falls back to
  `b_transport` and re-acquires DMI.

## 2. Debug transport — zero-time inspection

After the loop the initiator issues one `socket->transport_dbg(trans)` for 128
bytes. `Memory::transport_dbg` `memcpy`s without any `wait()`, so **no simulation
time passes**. Used for memory dumps, debuggers, loaders.

## 3. Response status

`Memory::b_transport` now returns a specific code instead of one blanket error:

| Condition | Status |
|-----------|--------|
| address ≥ SIZE | `TLM_ADDRESS_ERROR_RESPONSE` |
| byte enables present | `TLM_BYTE_ENABLE_ERROR_RESPONSE` |
| length > 4 or streaming width < length | `TLM_BURST_ERROR_RESPONSE` |
| otherwise | `TLM_OK_RESPONSE` |

The initiator prints `trans->get_response_string()` on error. Build with
`INJECT_ERROR` (see CMakeLists) to force the target to receive a bad
`streaming_width` for addresses > 0x90 and watch it report
`TLM_BURST_ERROR_RESPONSE`.

Note that here the **target** calls `wait(delay)` and then resets
`delay = SC_ZERO_TIME` — it consumes the annotated time itself ("b_transport may
block"), unlike gs1 where the initiator realised the delay.

## Build & run

```sh
cmake -S . -B build -G Ninja && cmake --build build   # from repo root
./build/demo_tlm2_gs2/tlm2_gs2
```

## Expected output (abridged)

```
trans = { W, 0 } , data = ff000000 at time 10 ns delay = 0 s     <- slow path, DMI granted
DMI   = { W, 4 } , data = ff000004 at time 20 ns                  <- fast path
DMI   = { W, 8 } , data = ff000008 at time 30 ns
...
DMI   = { R, 1c } , data = aa0000fe at time 80 ns
trans = { W, 20 } , data = ff000020 at time 80 ns delay = 0 s     <- DMI invalidated, slow path again
DMI   = { W, 24 } , data = ff000024 at time 90 ns
...
mem[0] = ff000000                                                 <- transport_dbg dump, time does not advance
mem[4] = ff000004
...
```

`trans = { … }` lines appear only right after each DMI invalidation (t = 10, 80,
160, 240 ns); everything else takes the DMI fast path.

## Changes vs. the reference snippet

- `sprintf` → `snprintf(txt, sizeof txt, …)` (bounded; `sprintf` is deprecated by
  the platform toolchain).
- The `unsigned char* data` used for the debug dump is `delete[]`d.
- `invalidate_direct_mem_ptr` / `get_direct_mem_ptr` cast their unused parameters
  to `void` to silence warnings.
- Added `<cassert>` / `<cstdio>` / `<cstring>` so the headers are self-contained.
- Include guards, and `#define SC_INCLUDE_DYNAMIC_PROCESSES` kept in `target.h`.
