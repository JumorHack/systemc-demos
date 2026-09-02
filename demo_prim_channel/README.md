# demo_prim_channel — a primitive channel (blocking FIFO)

A drastically simplified, char-only, blocking version of `sc_fifo`, written as a
**primitive channel**. It shows the parts of the SystemC kernel that primitive
channels are built on: the *evaluate / update* split, `request_update()` /
`update()`, and blocking via *dynamic sensitivity* (`wait(sc_event&)`).

## Files

| File | Contents |
|------|----------|
| [`fifo_if.h`](fifo_if.h) | `fifo_out_if` (`write`, `num_free`) and `fifo_in_if` (`read` ×2, `num_available`), both `virtual public sc_interface`, both non-copyable. |
| [`fifo.h`](fifo.h) | `class fifo : public sc_prim_channel, public fifo_out_if, public fifo_in_if` — circular buffer, two `sc_event`s, `update()`. |
| [`producer.h`](producer.h) | `sc_port<fifo_out_if> out`. One `write()` per fast-clock edge. |
| [`consumer.h`](consumer.h) | `sc_port<fifo_in_if> in`. One `read()` per slow-clock edge. |
| [`main.cpp`](main.cpp) | `ClkFast` = 1 ns, `ClkSlow` = 500 ns, one `fifo`, one `producer`, one `consumer`. |

## Primitive vs. hierarchical channel

|  | primitive channel | hierarchical channel ([`../demo_hier_channel`](../demo_hier_channel/)) |
|--|-------------------|----------------------|
| base class | `sc_prim_channel` | `sc_module` |
| can contain processes / sub-modules | no | yes |
| gets `update()` / `request_update()` | yes | no |
| typical use | signal-like things that must not change instantaneously | structured sub-systems that present an interface |

## How the blocking works

1. `write()` / `read()` run in the **evaluation phase**. They touch the buffer,
   adjust counters, and call `request_update()` — they do **not** notify anything yet.
2. If there is no room (`write`) or no data (`read`), the method calls
   `wait(data_read_event)` / `wait(data_written_event)`. This is *dynamic
   sensitivity*: the calling thread suspends on that event instead of its clock.
3. In the **update phase** the kernel calls `fifo::update()`, which posts
   `notify(SC_ZERO_TIME)` (a *delta* notification) on the relevant event and
   recomputes `num_readable`.
4. On the next delta the blocked thread resumes inside `write()` / `read()` and
   finishes.

## Build & run

```sh
cmake -S . -B build -G Ninja && cmake --build build   # from repo root
./build/demo_prim_channel/fifo_tb
```

## Expected output (abridged)

```
W 'T'  free=15  at 0 s
        R 'T'  avail=0  at 0 s
W 'h'  free=15  at 1 ns
W 'e'  free=14  at 2 ns
...
W 'f'  free=0  at 16 ns          <- FIFO (size 16) full, producer now blocks
        R 'h'  avail=15  at 500 ns
W 'o'  free=0  at 500 ns         <- freed slot immediately refilled
        R 'e'  avail=15  at 1 us
...
```

The fast producer fills the 16-slot FIFO in the first 16 ns, then blocks; from
then on it advances exactly one character per consumer read (every 500 ns).

## Bugs fixed vs. the tutorial snippet

- **`num_readable` / `num_read` / `num_written` were never initialised** (not in
  the constructor, not in `reset()`). The first `write()` then computes
  `num_free()` from indeterminate values — undefined behaviour, and a possible
  permanent deadlock if the garbage makes `num_free()` look like 0. Fixed by
  zeroing all three in `reset()` (which the constructor calls).
- The member `free` was renamed to `freespace` — `free` is also `<cstdlib>`'s
  `free()`; legal but needlessly confusing.
- `fifo_if.h` copy/assign are `= delete`d rather than declared-private-undefined;
  `update()` / interface methods are marked `override`; include guards added.
