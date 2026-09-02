# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this repository is

This is a **personal fork of the Open Source Risk Engine (ORE)** (`OpenSourceRisk/engine`,
currently tracking upstream release v1.8.16.0 / "ORE v16"). The goal is to follow upstream
while carrying a small set of local customizations. The uncommitted / untracked local
additions are:

- `scripts/` — convenience build scripts (not in upstream).
- Extra SWIG interface files exposing curve/interpolation classes to Python:
  `ORE-SWIG/QuantExt-SWIG/SWIG/qle_interpolation.i`, `qle_zerocurve.i`, `qle_piecewiseyieldcurve.i`,
  plus edits to `qle.i`, `qle_common.i`, `ored_conventions.i`, and `ORE-SWIG/setup.py`.
- `.gitattributes`, `.omc/`.

Git remotes: `upstream` = OpenSourceRisk, `origin` = personal fork. `todo.md` is an open
design note (in Korean) about the branch/merge strategy for keeping the fork in sync — read
it before making git-workflow decisions. When merging upstream, expect conflicts in
`ORE-SWIG/setup.py` and the SWIG `.i` files above.

## Architecture

ORE is a stack of static C++ libraries, each building on the one below. Build order and
target names (from `cmake/commonSettings.cmake` and the top-level `CMakeLists.txt`):

1. **QuantLib** (`QuantLib/`, git submodule — the OSR *fork*, not upstream QuantLib) → `ql_library`.
2. **QuantExt** (`QuantExt/qle`) → `QuantExt`. Extends QuantLib: simulation models, extra
   instruments, pricing engines, term structures.
3. **OREData** (`OREData/ored`) → `OREData`. Trade & market-data domain model: XML
   parsing (rapidxml), portfolio, curve configuration, market conventions, `xsd/` schemas.
4. **OREAnalytics** (`OREAnalytics/orea`) → `OREAnalytics`. Risk analytics: XVA, exposure
   simulation, sensitivity/stress, SIMM/initial margin, market/credit risk.
5. **App** (`App/ore.cpp`) → the `ore` executable — the driver that reads an XML master
   config and runs analytics. This is what the `Examples/` invoke.
6. **ORE-SWIG** (`ORE-SWIG/`) → Python bindings: the `_ORE` C extension + `ORE.py`,
   packaged as the `open-source-risk-engine` wheel.

`QuantExt`, `OREData`, `OREAnalytics` are ordinary subdirectories of this repo (not
submodules). Only `QuantLib` and `ORE-SWIG/QuantLib-SWIG` are submodules — keep them
checked out (`git submodule update --init --recursive`).

### SWIG binding layout

- Interface files live in `ORE-SWIG/{QuantLib,QuantExt,OREData,OREAnalytics}-SWIG/SWIG/*.i`.
- The Python module is assembled from `OREAnalytics-SWIG/SWIG/oreanalytics.i`; `qle.i`
  aggregates all `qle_*.i` QuantExt interfaces; likewise `ored_*.i` / `orea_*.i`.
- `python setup.py wrap` runs `swig -python -c++` with `-I` for all four SWIG dirs and
  emits `ORE-SWIG/oreanalytics_wrap.cpp`.
- **To expose a new C++ class:** add or edit the relevant `qle_*.i` / `ored_*.i`, `%include`
  it from the aggregator (`qle.i` etc.), then rebuild the wheel. Use `%shared_ptr(...)`
  before `%template(...)` for classes held via `boost::shared_ptr`.
- The MSVC link library names in `ORE-SWIG/setup.py` (`QuantExt-x64-mt-s`, …) must match
  the static libs produced by the C++ build (`-mt-s` = static runtime, no `-gd` in Release).

## Building

Build config is driven by the scripts in `scripts/`, which set environment variables and
then call CMake. **These scripts contain machine-specific absolute paths** (`D:\code\util\swigwin-*`,
`D:\code\util\cmake-*`, `../boost_1_86_0`); every path is overridable via a pre-set env var
(`SWIG_DIR`, `BOOST_ROOT`, `ORE_SWIG_DIR`, …). Run them from the repo root.

### Windows / MSVC (primary environment)

| Step | Command | Notes |
|---|---|---|
| Edit build settings | `scripts/config.cmd` | Sets `BUILD_TYPE` (Debug/Release/RelWithDebInfo), `GENERATOR` (VS 17 2022), `QL_ENABLE_SESSIONS`, `QL_USE_STD_CLASSES`, static-runtime flag. Sourced by every other `.bat`. |
| Build Boost | `scripts\build_boost.bat` | Runs `b2` with `WINVER=0x0A00`. Only needed once. |
| Build C++ libs + `ore.exe` | `scripts\build_msvc.bat` | CMake configure + `cmake --build build --config <BUILD_TYPE>`. x64, static libs, static CRT, C++20. |
| Build Python wheel | `scripts\build_swig.bat` | `setup.py wrap` → `build` → `bdist_wheel` in `ORE-SWIG/`. **Requires the C++ libs already built into `build/`.** Wheel lands in `ORE-SWIG/dist/`. |

### Linux

`scripts/config.sh` (edit `flag_use_vcpkg`, `BUILD_TYPE`, `CPU_N`) then
`scripts/build_linux.sh [target...]` (Ninja; Boost or vcpkg). Python: `scripts/build_swig.sh`.

### Direct CMake

Key options (see `cmake/commonSettings.cmake` for the full list and defaults):
`ORE_BUILD_APP`, `ORE_BUILD_SWIG`, `ORE_BUILD_TESTS`, `ORE_BUILD_EXAMPLES`, `ORE_USE_ZLIB`,
`ORE_BUILD_QL_SEPARATELY`, `QL_ENABLE_SESSIONS`, `QL_USE_STD_CLASSES`,
`MSVC_LINK_DYNAMIC_RUNTIME`, `BUILD_SHARED_LIBS`. C++20 is mandatory. `CMakePresets.json`
also defines named presets.

Note: the checked-in scripts build with `ORE_BUILD_TESTS=OFF` and `ORE_BUILD_EXAMPLES=OFF`.
To build tests, re-run CMake with `-DORE_BUILD_TESTS=ON` (add `-DORE_BUILD_EXAMPLES=ON` and
`-DQL_BUILD_TEST_SUITE=ON` as needed).

## Testing

- **C++ unit tests** (Boost.Test): per-module `QuantExt/test/`, `OREData/test/`,
  `OREAnalytics/test/`, shared helpers in `ORETest/`. Built only when `ORE_BUILD_TESTS=ON`.
  Run a single case: `<test-exe> --run_test=<Suite>/<Case> --log_level=message`.
- **Examples integration suite** (pytest): from `Examples/`, `pytest run_examples_testsuite.py`
  (registered as CTest test `examples`; needs `ORE_BUILD_EXAMPLES` + `ORE_BUILD_TESTS`).
  Individual example: `cd Examples/<Name> && python run.py` (or the example's own script).
- **Python binding tests:** `ORE-SWIG/runPythonTests.cmd` (Windows), or run
  `ORE-SWIG/OREAnalytics-SWIG/Python/Test/OREAnalyticsTestSuite.py` against the built module.
- CI reference: `.github/workflows/` (`build_windows.yaml`, `linux_build.yaml`,
  `linux_manylinux_wheels_02.yml`, `macos_ARM64_build.yaml`).

## Code style

`.clang-format` at the repo root (LLVM-based: 4-space indent, 120-column limit, attach
braces, `AccessModifierOffset: -4`, sorted includes). Match the surrounding file.
