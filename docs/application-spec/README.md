# CYD Solar System - Application Spec

What the CYD Solar System firmware is, what it shows, what it takes in and puts out, and the rules it follows.
Written from a read of the repository files on 2026-10-22 (`src/main.cpp`, `platformio.ini`, `README.md`, `.gitignore`). Every statement is anchored to `file:line` or marked **unverified**. Nothing in this spec was flashed, built, or run on hardware.

The companion [design spec](../design-spec/README.md) explains how each part works.

| # | File | Topic |
|---|---|---|
| 01 | [01-purpose-users-workflows.md](01-purpose-users-workflows.md) | Purpose, scope, users, and workflows |
| 02 | [02-features-controls-io.md](02-features-controls-io.md) | Features, touch controls, inputs, outputs |
| 03 | [03-hardware-configuration.md](03-hardware-configuration.md) | Target hardware, pin map, build configuration |
| 04 | [04-requirements-testing-limitations.md](04-requirements-testing-limitations.md) | As-coded requirements, testing, known limitations and discrepancies |
| 05 | [05-glossary.md](05-glossary.md) | Terms |

**Scope of this repo.** This repository contains one firmware project (`solar-system`). The parent folder `G:\Projects\Hardware\CYD2USB` also holds `denki-kurage/` and `puttview-display/`; those are separate and are not documented here.
