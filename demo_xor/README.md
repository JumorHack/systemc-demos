# demo_xor — XOR from NAND gates

A classic SystemC structural-modelling example: a 2-input XOR gate (`exor2`) built
from **four NAND gates**, verified by a small clocked testbench (stimulus + monitor)
and dumped to a VCD waveform.

## Modules

| File | Module | Role |
|------|--------|------|
| [`nand2.h`](nand2.h) | `nand2` | Leaf cell. `SC_METHOD` sensitive to `A`, `B`; drives `F = !(A && B)`. |
| [`exor2.h`](exor2.h) | `exor2` | **DUT.** Four `nand2` instances wired with internal signals `S1..S3`. No behaviour of its own — pure structure. |
| [`stim.h`](stim.h) | `stim` | `SC_THREAD` on `Clk.pos()`. Applies `00 → 01 → 10 → 11`, then `sc_stop()`. |
| [`mon.h`](mon.h) | `mon` | `SC_THREAD` on `Clk.neg()`. Prints `Time A B F` once per clock. |
| [`main.cpp`](main.cpp) | `sc_main` | Builds signals + a 10 ns clock, wires the three modules, adds a VCD trace, runs. |

## The netlist

```
S1 = A  NAND B
S2 = A  NAND S1
S3 = S1 NAND B
F  = S2 NAND S3      ≡   A XOR B
```

```
        TestClk (10 ns)
          │
  ┌───────┼──────────────────┐
  ▼       ▼                  ▼
 stim ──A,B──▶  exor2 (DUT) ──F──▶ mon
```

## Build & run

From the repo root:

```sh
cmake -S . -B build -G Ninja && cmake --build build
./build/demo_xor/exor2_tb
```

or standalone:

```sh
cd demo_xor
cmake -S . -B build -G Ninja && cmake --build build
./build/exor2_tb
```

## Expected output

```
      Time A B F
      5 ns 0 0 0
     15 ns 0 1 1
     25 ns 1 0 1
     35 ns 1 1 0
```

Plus a waveform file `exor2.vcd` (open with GTKWave / Surfer / WaveTrace).

## Design notes

- **`dont_initialize()` on the stimulus thread** — without it the thread runs once at
  `t = 0` before the first clock edge and the first vector `(0,0)` gets consumed
  before the monitor ever samples it.
- **Monitor samples on `Clk.neg()`** — mid-cycle, after the combinational NAND network
  has settled and while the stimulus is *not* concurrently writing. Sampling on
  `Clk.pos()` races with the stimulus and yields stale / unsettled reads.
- **`std::setw`** must be qualified — `systemc.h` does not pull `setw` into the global
  namespace even though it drags in most of `<iostream>`.
- Remove the three `sc_trace(...)` lines in `main.cpp` if you don't want the VCD.
