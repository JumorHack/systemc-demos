# demo_tlm2_gs4 — TLM-2.0 getting started (example 4)

The big jump: gs1–gs3 are all **loosely-timed** (`b_transport` + one `delay`
annotation). This one is **approximately-timed (AT)** — the non-blocking
transport interface with the 4-phase base protocol, payload event queues,
pipelining with the exclusion rules, and an explicit memory manager with
reference counting.

Verbatim from the Doulos *TLM-2.0 Getting Started* example 4, split per component
in the style of
[SingularityKChen/SystemC-Training](https://github.com/SingularityKChen/SystemC-Training).

## Files

| File | Contents |
|------|----------|
| [`common.h`](common.h) | shared includes, `#define DEBUG`, the trace stream `fout`, `rand_ps()` (power-law random delay), and `class mm` — the transaction-pool memory manager. |
| [`initiator.h`](initiator.h) | `Initiator`: issues 1000 random transactions via `nb_transport_fw`, honours the BEGIN_REQ rule, handles the backward path in `nb_transport_bw` + a PEQ, then one nominal `b_transport`. |
| [`target.h`](target.h) | `Target`: `nb_transport_fw` + a PEQ state machine; 2 transactions in flight, BEGIN_RESP rule, back-pressure via deferred END_REQ. Declares the extended phase `internal_ph`. |
| [`top.h`](top.h) | `SC_MODULE(Top)` — instantiate both, bind sockets. |
| [`testbench.cpp`](testbench.cpp) | Doulos Apache-2.0 header + `sc_main`. |

## Output

Everything goes to **`foo.txt`** in the working directory (1000 transactions ×
several phase-transition lines each ≈ 9000 lines — far too much for stdout).
stdout only gets a one-line "done" notice.

```sh
cmake -S . -B build -G Ninja && cmake --build build   # from repo root
./build/demo_tlm2_gs4/tlm2_gs4
less foo.txt
```

## The 4-phase base protocol

```
initiator ── nb_transport_fw(BEGIN_REQ) ──▶ target
initiator ◀─ nb_transport_bw(END_REQ)   ──  target     (unblocks next BEGIN_REQ)
initiator ◀─ nb_transport_bw(BEGIN_RESP)──  target
initiator ── nb_transport_fw(END_RESP)  ──▶ target     (frees the transaction)
```

Each `nb_transport_*` returns immediately with a `tlm_sync_enum`
(`TLM_ACCEPTED` / `TLM_UPDATED` / `TLM_COMPLETED`). Here every hop returns
`TLM_ACCEPTED` and the next phase arrives as a **separate** call later — the
"backward path", never the "return path".

## Payload event queues

`tlm_utils::peq_with_cb_and_phase` on **both** sides. A `(trans, phase, delay)`
triple is queued; when `delay` expires the queue calls `peq_cb`. This is how
timing annotations and internal delays are realised without blocking:

- initiator processing delay — the `delay` on the `BEGIN_REQ` call
- target accept delay — the `delay` on the `END_REQ` call
- target latency — a self-notification with the **extended phase** `internal_ph`
  (`DECLARE_EXTENDED_PHASE`), whose `peq_cb` case actually performs the read/write

## Pipelining + exclusion rules

- **BEGIN_REQ rule** (initiator): no new `BEGIN_REQ` until the previous one is
  answered with `END_REQ` — `if (request_in_progress) wait(end_request_event);`
- **BEGIN_RESP rule** (target): no `BEGIN_RESP` until the previous `END_RESP`.
- The target keeps **2 transactions in flight** (`n_trans`); at 2 it withholds
  `END_REQ` (`end_req_pending`) to back-pressure the initiator, and buffers a
  second response in `next_response_pending`.

## Memory manager + reference counting

`class mm : tlm::tlm_mm_interface` keeps a free-list pool. Under AT a
transaction outlives the call that created it (it travels through both PEQs), so:

- `m_mm.allocate()` + `trans->acquire()` at the initiator
- `trans->acquire()` again at the target on `BEGIN_REQ`
- `trans->release()` on each side when done; at refcount 0 `mm::free()` recycles it

With `#define DEBUG` the `allocate()` / `free()` lines print a running `#trans`
count — it returns to 0 at the end, i.e. no leak.

## Not shown

Temporal decoupling / quantum, DMI, debug transport. The final `b_transport`
call exists only to exercise the `simple_target_socket` b→nb adapter (the target
implements `nb_transport_fw` only).

## Changes vs. the reference snippet

- `sprintf` → `snprintf(txt, sizeof txt, …)` (`sprintf` is deprecated by the
  platform toolchain).
- `fout` is an `inline` variable; `mm::allocate` / `mm::free` / `rand_ps` are
  `inline` — they now live in a shared header cleanly.
- Two genuinely unused locals cast to `void`.
- One line to stdout so a bare run isn't silent.
