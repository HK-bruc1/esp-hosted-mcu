#!/bin/bash
# scripts/check_host_port_surface.sh
#
# Guards the best-practice boundaries established by WP 1-4.
# Ensures the host port surface remains clean:
#   - No legacy OS abstraction headers leak into the public/active scope
#   - No old vtable port headers leak into the public/active scope
#   - Root CMakeLists.txt does not hardcode port source files
#   - port.cmake exists as the canonical port entry point
#
# Usage: bash scripts/check_host_port_surface.sh

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$REPO_ROOT"

FAIL=0

# ── Scope directories (shared by Check 1 & 2) ─────────────────────────
SCOPE=(
    host/esp_hosted.h
    host/api/include
    host/api/src
    host/core
    host/drivers/serial
    host/drivers/rpc/slaveif
    host/drivers/virtual_serial_if
    host/drivers/power_save
    host/drivers/bt
    host/drivers/transport/spi
    host/drivers/transport/spi_hd
    host/drivers/transport/sdio
    host/drivers/transport/uart
    host/port/esp-idf
    host/port/include
    common
)

# ── Check 1: esp_hosted_os_abstraction.h must not appear in scope ─────
echo "=== Check 1: No esp_hosted_os_abstraction.h in public/active scope ==="
if rg -n 'esp_hosted_os_abstraction\.h' "${SCOPE[@]}" 2>/dev/null; then
    echo "FAIL: found forbidden esp_hosted_os_abstraction.h include"
    FAIL=1
else
    echo "PASS: 0 hits"
fi
echo ""

# ── Check 2: No #include of port_esp_hosted_host_* in scope ──────────
# Only catches actual #include directives, not migration comments.
echo "=== Check 2: No #include port_esp_hosted_host_* in public/active scope ==="
if rg -n '#include.*port_esp_hosted_host_' "${SCOPE[@]}" 2>/dev/null; then
    echo "FAIL: found forbidden port_esp_hosted_host_ include"
    FAIL=1
else
    echo "PASS: 0 hits"
fi
echo ""

# ── Check 3: host/esp_hosted.h must not include legacy OS abstraction ─
echo "=== Check 3: host/esp_hosted.h must not include legacy OS abstraction ==="
if rg -n 'esp_hosted_os_abstraction' host/esp_hosted.h 2>/dev/null; then
    echo "FAIL: found legacy OS abstraction reference in esp_hosted.h"
    FAIL=1
else
    echo "PASS: 0 hits"
fi
echo ""

# ── Check 4: Root CMakeLists.txt must not hardcode port source files ──
echo "=== Check 4: Root CMakeLists.txt must not hardcode host/port/esp-idf/*.c ==="
if rg -n 'host/port/esp-idf/.+\.c' CMakeLists.txt 2>/dev/null; then
    echo "FAIL: found hardcoded port source files in CMakeLists.txt"
    FAIL=1
else
    echo "PASS: 0 hits"
fi
echo ""

# ── Check 5: host/port/esp-idf/port.cmake must exist ──────────────────
echo "=== Check 5: host/port/esp-idf/port.cmake must exist ==="
if [ -f host/port/esp-idf/port.cmake ]; then
    echo "PASS: port.cmake exists"
else
    echo "FAIL: host/port/esp-idf/port.cmake not found"
    FAIL=1
fi
echo ""

# ── Final result ──────────────────────────────────────────────────────
if [ "$FAIL" -ne 0 ]; then
    echo "FAILED"
    exit 1
fi
echo "ALL CHECKS PASSED"
exit 0
