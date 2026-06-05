# AGENTS.md

Build, test, and verification guidance for automated agents working in this repo.
This is an ESP-IDF component that enables using Espressif chips as Wi-Fi/Bluetooth co-processors
for host MCUs via SPI, SDIO, or UART. See also `CLAUDE.md` for deeper architecture details.

## Project Overview

- **Name**: ESP-Hosted-MCU (`esp_hosted` on the Espressif Component Registry)
- **Version**: `2.12.8` (single source of truth: `idf_component.yml`)
- **License**: Apache-2.0
- **Minimum IDF**: `>= 5.3`
- **Repo structure**:
  - `host/` — Host-side framework (portable core + ESP-IDF port + API + drivers)
  - `slave/` — Standalone ESP-IDF project for the co-processor firmware
  - `common/` — Shared protobuf schema, transport headers, mempool, utilities
  - `examples/` — 23 standalone host-side example projects
  - `tests/` — Linux mock unit tests (Unity framework) for the portable host core

## Build Commands

This repo is an **ESP-IDF component** (`idf_component.yml`). Use `idf.py`, never raw `cmake` or `make`.
**You cannot build from the repo root.**

### Co-processor (slave) firmware

```bash
cd slave
idf.py set-target <TARGET>                          # e.g. esp32c6
idf.py menuconfig                                   # transport under "ESP-Hosted config"
idf.py build
```

CI-style build with preset config:
```bash
cd slave
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.ci.spi" set-target esp32c6
idf.py build
```
Presets: `sdio`, `spi`, `spi_hd`, `uart`, `dpp`, `wifi_enterprise`, `all_features`.

### Host examples

Build against the **registered component** (default):
```bash
cd examples/<name>
idf.py set-target <TARGET>
idf.py add-dependency "espressif/esp_wifi_remote"
idf.py build
```

Build against **local repo changes** (CI / dev pattern):
```bash
cd examples/<name>
mkdir -p components
ln -s <path_to_esp_hosted_mcu_repo> components/esp_hosted
idf.py set-target esp32p4
idf.py build
```

The `host_compile_check` example is special: it uses `EXTRA_COMPONENT_DIRS` to compile the
local component directly without a symlink, and is used in CI for quick compile validation.
The `host_framework_validation` example is used in CI for host-build matrix verification.

### Linux Mock Unit Tests

The host portable core (`host/core/src/`) has Linux mock unit tests using the Unity framework.
These run on Ubuntu with zero hardware dependencies:

```bash
bash scripts/run_linux_mock_tests.sh
```

This script:
1. Compiles all 12 `host/core/src/*.c` files (compile-only verification)
2. Links and runs 21 Unity tests with AddressSanitizer + UBSan
3. Rebuilds and runs with ThreadSanitizer

The test suites cover: OSAL (malloc, mutex, semaphore, thread, queue), event register/post,
transport init/state-machine/teardown, RPC request/response matching, RPC bridge contract tests.

### Isolation Checks

Verify the portable core contains no ESP-IDF or FreeRTOS dependencies:
```bash
bash scripts/check_core_isolation.sh
```

Transport-specific active-path isolation:
```bash
bash scripts/check_current_platform_isolation.sh <transport>   # spi | spi_hd | sdio | uart
```

## Pre-commit Hooks (mandatory before commit)

```bash
pre-commit run --all-files
```

Individual checks:
```bash
python tools/check_fw_versions.py --update            # version sync (auto-generates .h files)
python tools/check_rpc_calls.py                       # RPC consistency
python tools/check_changelog.py                       # changelog
python tools/check_weak_functions.py --file host/api/src/esp_wifi_weak.c
```

Pre-commit hooks verify:
- Version sync across `idf_component.yml`, `host/esp_hosted_host_fw_ver.h`, and `slave/main/esp_hosted_coprocessor_fw_ver.h`
- RPC consistency between `common/proto/esp_hosted_rpc.proto` and `docs/implemented_rpcs.md`
- Weak function coverage in `host/api/src/esp_wifi_weak.c`
- Changelog updates on version bumps
- Copyright headers
- Spellcheck (codespell)

## Version Sync (edit ONLY `idf_component.yml`)

The two version header files are **auto-generated** by the pre-commit hook (`tools/check_fw_versions.py`):
- `host/esp_hosted_host_fw_ver.h` — marked "DO NOT MODIFY THIS FILE"
- `slave/main/esp_hosted_coprocessor_fw_ver.h` — marked "DO NOT MODIFY THIS FILE"

**When bumping version, edit ONLY `idf_component.yml` (line 1)**, then run `pre-commit run --all-files`.
The hook will auto-update both `.h` files. Do not hand-edit them.

## Adding an RPC

1. Add message to `common/proto/esp_hosted_rpc.proto`
2. Regenerate: `cd common/proto && protoc-c esp_hosted_rpc.proto --c_out=.`
3. Add host wrapper in `host/core/src/h_rpc_wrap.c`
4. Add slave handler in `slave/main/slave_control.c`
5. Document in `docs/implemented_rpcs.md`
6. Run `pre-commit run --all-files`

## Testing Strategy

There are **no ESP-IDF unit tests** in this component. Verification relies on four layers:

1. **Pre-commit hooks** — version, RPC, changelog, weak function, copyright, spellcheck.
2. **Linux mock tests** (`scripts/run_linux_mock_tests.sh`) — compiles all 12 host core files,
   links 21 Unity tests, runs under ASAN/UBSAN and TSAN. This is the closest thing to unit testing.
3. **CI build matrix** — sanity + regression builds across IDF versions, targets, transports
   (see `.gitlab/ci/` and `.github/workflows/`).
4. **Hardware tests** — manual scripts (`scripts/hw_smoke_test.sh`, `scripts/hw_acceptance.sh`)
   for physical validation.

## CI/CD

### GitHub Actions (`.github/workflows/`)

- **`host-core-test.yml`** — Three-tier CI on `ubuntu-latest`:
  - Tier 1: `scripts/check_core_isolation.sh` + `gcc -fsyntax-only` for all core files
  - Tier 2: `scripts/run_linux_mock_tests.sh` (ASAN/UBSAN + TSAN)
  - Tier 3: ESP-IDF slave build matrix (`esp32c6` × `sdio/spi/uart` × IDF `v5.3`)
  - Tier 3 supplement: ESP-IDF host build (`examples/host_framework_validation` on `esp32p4`)
- **`upload_component.yml`** — Uploads to `components.espressif.com` on push to `main`
- **`upstream-sync-check.yml`** — Verifies generated protobuf files exist
- **`sync-jira.yml`** — Syncs GitHub issues/PRs to Jira

### GitLab CI (`.gitlab-ci.yml` + `.gitlab/ci/`)

Primary CI with two pipelines:
- **Sanity pipeline** (merge requests to `staging`) — premerge checks, coprocessor builds
  for `esp32c6/c5/h2` across transports, example builds for `esp32p4/h2`.
- **Regression pipeline** (pushes to `staging`) — full matrix across IDF `v5.3` through `latest`,
  all targets (`esp32`, `c2`, `c3`, `s3`, `c6`, `c5`, `c61`, `h2`, `p4`), `all_features` build,
  ends with `promote_staging_to_main` deploy job.

## Quirks & Gotchas

- **Slave and every example are separate ESP-IDF projects** — each has its own `CMakeLists.txt`
  calling `project()`. You cannot build from the repo root.
- **`common/protobuf-c` is a git submodule** — clone with `--recurse-submodules` or run
  `git submodule update --init`.
- **The component uses `WHOLE_ARCHIVE`** (`idf_component_set_property WHOLE_ARCHIVE TRUE` in
  root `CMakeLists.txt`) to prevent the linker from dropping weak function overrides.
- **`esp_wifi_remote` is a separate component** (from registry) that provides empty weak
  `esp_wifi_*` stubs. ESP-Hosted provides the real implementations in `host/api/src/esp_wifi_weak.c`.
  The `check_weak_functions.py` tool ensures coverage.
- **The `-Wl,--wrap=esp_wifi_init`** linker flag in `slave/CMakeLists.txt` is how the slave
  intercepts `esp_wifi_init`.
- **ESP-IDF >= 5.3 required.**
- **Transport selection is compile-time via Kconfig**, not runtime. The CMakeLists conditionally
  includes only one transport driver at a time.
- **Dual port layers exist during transition**: both `host/port/esp-idf/` (new portable layer)
  and `host/port/esp/freertos/` (legacy) are compiled simultaneously. The new adapters call into
  legacy implementations. New code should target the core layer (`host/core/`) and contract wrappers
  (`host/port/include/h_wrapper.h`).
- **Legacy RPC code** in `host/drivers/rpc/` is superseded by `host/core/src/h_rpc_*.c`.
  Only headers from `host/drivers/rpc/` are used as include paths; sources are not compiled.

## Key File Map

| Path | Role |
|---|---|
| `host/api/src/esp_wifi_weak.c` | Real weak API implementations (checked by `check_weak_functions.py`) |
| `host/core/src/h_init.c` | Portable init with vtable contract validation |
| `host/core/src/h_transport_drv.c` | Transport driver state machine (portable) |
| `host/core/src/h_serial_if.c` | Serial interface abstraction for RPC |
| `host/core/src/h_rpc_core.c` | RPC core engine (request/response/encode/decode) |
| `host/core/src/h_rpc_wrap.c` | RPC → Wi-Fi API wrappers (host side) |
| `host/core/include/h_public/` | Public portable types (`h_types.h`, `h_wifi_types.h`, `h_event.h`) |
| `host/core/include/h_internal/` | Internal core headers (`h_rpc_core.h`, `h_serial_if.h`, `h_transport.h`) |
| `host/port/include/h_port_contract.h` | Three vtables: OSAL, event, transport |
| `host/port/esp-idf/` | New ESP-IDF port (OSAL, transport HAL, type adapters) |
| `host/port/linux/` | Linux mock port for unit tests |
| `slave/main/slave_control.c` | RPC handler dispatch (slave side) |
| `slave/main/slave_wifi_std.c` | Standard Wi-Fi RPC handlers |
| `common/proto/esp_hosted_rpc.proto` | Protobuf RPC schema |
| `common/proto/esp_hosted_rpc.pb-c.c/h` | Generated protobuf-c code (do not hand-edit) |
| `common/transport/esp_hosted_header.h` | Transport frame header definition |
| `host/drivers/transport/` | SPI, SDIO, UART transport drivers (legacy but active) |
| `slave/main/*_slave_api.c` | Slave-side transport drivers |
| `idf_component.yml` | Component manifest + version (single source of truth) |
| `Kconfig` | All Kconfig options (shared host/slave) |
| `slave/sdkconfig.ci.*` | CI preset sdkconfigs per transport |
