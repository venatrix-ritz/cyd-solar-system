# CYD Solar System - Design Spec

How the firmware in `src/main.cpp` works: architecture, the orbit and timing math with its parameters, interfaces, error handling, device constraints, and the reasons behind the main choices.
Companion to the [application spec](../application-spec/README.md). Facts come from reading `src/main.cpp`, `platformio.ini`, and `README.md` on 2026-10-22. `file:line` anchors refer to those files. **Unverified** marks anything that was not traced or run. Nothing was built or flashed for this spec.

| # | File | Topic |
|---|---|---|
| 01 | [01-architecture-dataflow.md](01-architecture-dataflow.md) | Module layout, per-frame flow, data-flow diagram |
| 02 | [02-algorithms-timing.md](02-algorithms-timing.md) | Date conversion, orbit math, sim clock, warp, label and star math, timing |
| 03 | [03-interfaces-errors-constraints.md](03-interfaces-errors-constraints.md) | Functions and state, error paths, memory and display constraints |
| 04 | [04-decisions-roadmap.md](04-decisions-roadmap.md) | Design decisions, rejected alternatives, and the roadmap status |
