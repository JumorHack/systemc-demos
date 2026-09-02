# demo_tlm2_gs6 — TLM-2.0 getting started (example 6)

Same shape as gs5's bus, but the interconnect uses **multi-sockets**
(`multi_passthrough_initiator_socket` / `multi_passthrough_target_socket`)
instead of arrays of tagged sockets — one socket object bound many times. The
initiator and target are the **approximately-timed** ones from gs4.

**4 initiators → 1 `Bus` → 4 targets**, everything over the forward/backward
`nb_transport` interface.

From the Doulos *TLM-2.0 Getting Started* example 6, split per component in the
style of
[SingularityKChen/SystemC-Training](https://github.com/SingularityKChen/SystemC-Training).

## Files

| File | Contents |
|------|----------|
| [`common.h`](common.h) | shared includes, `#define DEBUG`, the trace stream `fout`, `rand_ps()`, `class mm` (transaction pool). |
| [`initiator.h`](initiator.h) | `Initiator` — the AT initiator from gs4: 1000 pipelined transactions via `nb_transport_fw`, BEGIN_REQ rule, PEQ on the backward path. |
| [`bus.h`](bus.h) | `Bus` — **one** `multi_passthrough_target_socket` + **one** `multi_passthrough_initiator_socket`. Every callback takes an `int id` = which binding the call arrived on. |
| [`target.h`](target.h) | `Target` — the AT target from gs4, with `end_req_pending` as a `std::queue` (v3 of the Doulos example). |
| [`top.h`](top.h) | `SC_MODULE(Top)` — 4 `Initiator`, 1 `Bus`, 4 `Target`; binds each multi-socket 4 times. |
| [`testbench.cpp`](testbench.cpp) | Doulos Apache-2.0 header + `sc_main`. |

## Multi-socket vs. tagged socket (gs5 → gs6)

| | gs5 (tagged) | gs6 (multi) |
|--|--------------|-------------|
| declaration | `simple_*_socket_tagged<Bus>* sock[N];` — an explicit array | `multi_passthrough_*_socket<Bus> sock;` — one object |
| sizing | fixed `N` template params on `Bus` | grows as things `bind()` to it; `sock.size()` at run time |
| binding | `init->socket.bind(*bus->targ_socket[i])` | `init->socket.bind(bus->targ_socket)` — same socket, 4 times |
| call out | `(*init_socket[t])->nb_transport_fw(...)` | `init_socket[t]->nb_transport_fw(...)` |
| identifying the caller | the tag baked in at `register_*(..., i)` | the `int id` argument the multi-socket prepends to every callback |

The routing logic is identical: `nb_transport_fw` records `m_id_map[&trans] = id`
so the matching `nb_transport_bw` can send the response back to the right
initiator; `decode_address` picks the target from `address & 0x3` (this example
passes the address through unchanged, `compose_address` is the identity).

## Output

Everything goes to **`foo.txt`** (4 × 1000 transactions ≈ 28 000 lines). stdout
gets a one-line "done" notice. Each line is prefixed with the module's
hierarchical `name()`, so you can see all four initiators and all four targets
interleaving:

```sh
cmake -S . -B build -G Ninja && cmake --build build   # from repo root
./build/demo_tlm2_gs6/tlm2_gs6
less foo.txt
```

```
41a7     top.init_0 new, cmd=W, ... at time 0 s
1c06dac8 top.init_1 new, cmd=R, ... at time 0 s          <- 4 initiators run concurrently
...
46cdbe2  top.target_2 BEGIN_REQ at 196 ps                <- bus routed init_3 -> target_2
41a7     top.target_3 Execute WRITE, target = top.target_3 data = 60b7acd9
41a7     top.init_0 BEGIN_RESP at 2143 ps
...
```

## Changes vs. the reference snippet

- `sprintf` → `snprintf(txt, sizeof txt, …)` in `initiator.h` and `top.h`.
- `fout` is an `inline` variable; `mm::allocate` / `mm::free` / `rand_ps` are
  `inline`, so they share `common.h` cleanly.
- `assert(id < sock.size())` made signed/unsigned-clean (`id < int(sock.size())`);
  a few dead locals / params cast to `void`.
- One line to stdout so a bare run isn't silent.
