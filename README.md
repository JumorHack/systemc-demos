# sc_demo — SystemC learning demos

A small, growing collection of self-contained [SystemC](https://systemc.org/) examples,
managed by a single top-level CMake project. Each demo lives in its own folder with
its own `CMakeLists.txt` and `README.md`.

Tested with **Accellera SystemC 3.0.2** on macOS (arm64) + AppleClang, but nothing
here is platform-specific.

## Demos

| Folder | Target | What it shows |
|--------|--------|---------------|
| [`demo_xor/`](demo_xor/) | `exor2_tb` | Structural modelling: a 2-input XOR built from four NAND gates, driven by a clocked stimulus/monitor testbench, with a VCD waveform dump. |
| [`demo_hier_channel/`](demo_hier_channel/) | `stack_tb` | Hierarchical channel: an `sc_module` that also implements two interfaces (`stack_write_if` / `stack_read_if`) and is bound to `producer` / `consumer` ports via `sc_port<IF>`. |
| [`demo_prim_channel/`](demo_prim_channel/) | `fifo_tb` | Primitive channel: an `sc_prim_channel` blocking FIFO. Shows the evaluate/update split, `request_update()` / `update()`, and blocking through dynamic sensitivity (`wait(sc_event&)`). |
| [`demo_tlm2_gs1/`](demo_tlm2_gs1/) | `tlm2_gs1` | TLM-2.0 getting started (1): one initiator bound directly to one target (memory), loosely-timed `b_transport` with the generic payload and timing annotation. |
| [`demo_tlm2_gs2/`](demo_tlm2_gs2/) | `tlm2_gs2` | TLM-2.0 getting started (2): adds the Direct Memory Interface + DMI hint, the debug transport interface, and per-error response-status codes. |

## Prerequisites

- **SystemC 3.0.x** installed. The build looks in `/opt/systemc` by default; override with
  `-DCMAKE_PREFIX_PATH=/path/to/systemc`.
- **CMake ≥ 3.16**
- A **C++17** compiler (SystemC 3.0 requires it)
- **Ninja** (optional; any generator works)

## Build & run — all demos

```sh
cmake -S . -B build -G Ninja
cmake --build build

./build/demo_xor/exor2_tb
./build/demo_hier_channel/stack_tb
./build/demo_prim_channel/fifo_tb
./build/demo_tlm2_gs1/tlm2_gs1
./build/demo_tlm2_gs2/tlm2_gs2
```

## Build & run — one demo on its own

Every demo also configures standalone (it falls back to its own `find_package`):

```sh
cd demo_xor
cmake -S . -B build -G Ninja && cmake --build build
./build/exor2_tb
```

## Adding a new demo

1. Create a folder next to the existing demos.
2. Drop in your sources plus a `CMakeLists.txt` (copy one of the existing ones —
   keep the `if(NOT TARGET SystemC::systemc)` standalone fallback).
3. Re-run `cmake --build build`. The top-level `CMakeLists.txt` globs sub-directories
   with `CONFIGURE_DEPENDS`, so the new demo is picked up automatically.

## Layout

```
sc_demo/
├── CMakeLists.txt          # shared config + auto-registers every demo sub-dir
├── demo_xor/
│   ├── CMakeLists.txt
│   ├── README.md
│   └── *.cpp / *.h
├── demo_hier_channel/
│   ├── CMakeLists.txt
│   ├── README.md
│   └── *.cpp / *.h
├── demo_prim_channel/
│   ├── CMakeLists.txt
│   ├── README.md
│   └── *.cpp / *.h
├── demo_tlm2_gs1/           # initiator.h / target.h / top.h / testbench.cpp
│   ├── CMakeLists.txt
│   ├── README.md
│   └── *.cpp / *.h
└── demo_tlm2_gs2/           # + DMI, debug transport, response status
    ├── CMakeLists.txt
    ├── README.md
    └── *.cpp / *.h
```

The two TLM-2.0 demos follow the file layout of
[SingularityKChen/SystemC-Training](https://github.com/SingularityKChen/SystemC-Training)
(one header per component + `testbench.cpp`).
