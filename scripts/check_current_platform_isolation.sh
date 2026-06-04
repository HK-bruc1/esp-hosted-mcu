#!/usr/bin/env bash
# scripts/check_current_platform_isolation.sh
#
# Checks current-platform active path isolation beyond core/.
# Verifies that files in the current-platform active path for a given
# transport do not leak platform dependencies into the core layer,
# and that control adapter boundaries are respected.
#
# Usage: scripts/check_current_platform_isolation.sh [spi|spi_hd|sdio|uart]
#
# Reference: docs/felix/current_platform_active_path_matrix.md
# See also: scripts/check_core_isolation.sh (portable subset)
#           docs/felix/22.Host通用框架移植到好型收口实施计划.md

set -euo pipefail

# ── Colors ────────────────────────────────────────────────────────────
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
BOLD='\033[1m'
NC='\033[0m' # No Color

# ── Usage ─────────────────────────────────────────────────────────────
TRANSPORT="${1:-}"

if [ -z "$TRANSPORT" ]; then
    echo "Usage: $0 <transport>"
    echo "  transport: spi | spi_hd | sdio | uart"
    echo ""
    echo "Checks current-platform active path isolation for the given transport."
    echo "Validates control adapter boundaries and transport adapter isolation."
    echo ""
    echo "Reference: docs/felix/current_platform_active_path_matrix.md"
    exit 1
fi

case "$TRANSPORT" in
    spi|spi_hd|sdio|uart) ;;
    *)
        echo -e "${RED}ERROR:${NC} Unknown transport '$TRANSPORT'. Must be: spi, spi_hd, sdio, or uart."
        exit 1
        ;;
esac

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$REPO_ROOT"

echo ""
echo -e "${BOLD}=== Current-Platform Isolation Check: ${TRANSPORT} ===${NC}"
echo ""

# ── File Groups ────────────────────────────────────────────────────────
#
# These are the files that participate in the current-platform active path.
# Defined per docs/felix/current_platform_active_path_matrix.md §1.

# Core files: exactly 12 .c files under host/core/src/
# (same canonical set as scripts/check_core_isolation.sh)
CORE_FILES=(
    host/core/src/h_init.c
    host/core/src/h_api.c
    host/core/src/h_event.c
    host/core/src/h_serial_if.c
    host/core/src/h_rpc_utils.c
    host/core/src/h_transport_util.c
    host/core/src/h_rpc_core.c
    host/core/src/h_rpc_wrap.c
    host/core/src/h_transport_drv.c
    host/core/src/h_rpc_req.c
    host/core/src/h_rpc_rsp.c
    host/core/src/h_rpc_evt.c
)

# Transport driver file(s)
TRANSPORT_DRV_DIR="host/drivers/transport/${TRANSPORT}"
TRANSPORT_DRV_FILES=()
if [ -d "$TRANSPORT_DRV_DIR" ]; then
    for f in "$TRANSPORT_DRV_DIR"/*.c; do
        [ -f "$f" ] && TRANSPORT_DRV_FILES+=("$f")
    done
fi

# New port transport file
PORT_TRANSPORT_FILE="host/port/esp-idf/h_transport_${TRANSPORT}.c"

# Control adapter files (always active regardless of transport)
# These are in host/port/esp-idf/ or host/drivers/ — checked by Rule 1 for
# g_h.funcs (legacy vtable), exempt from core-level Rules 2-5.
CONTROL_ADAPTER_FILES=(
    host/port/esp-idf/h_control_serial_adapter.c
    host/drivers/serial/serial_ll_if.c
    host/drivers/rpc/slaveif/rpc_slave_if.c
    host/drivers/virtual_serial_if/serial_if.c
)

# API layer — always compiled, checked by Rule 1
API_FILES=(
    host/api/src/esp_hosted_api.c
    host/api/src/esp_hosted_ota_api.c
    host/api/src/esp_hosted_transport_config.c
)

# Tools — always compiled, checked by Rule 1
TOOLS_FILES=(
    host/port/esp-idf/tools/stats.c
)

# Build the full active path file list for this transport
ACTIVE_PATH_FILES=("${CORE_FILES[@]}")
[ ${#TRANSPORT_DRV_FILES[@]} -gt 0 ] && ACTIVE_PATH_FILES+=("${TRANSPORT_DRV_FILES[@]}")
[ -f "$PORT_TRANSPORT_FILE" ] && ACTIVE_PATH_FILES+=("$PORT_TRANSPORT_FILE")
ACTIVE_PATH_FILES+=("${CONTROL_ADAPTER_FILES[@]}")
ACTIVE_PATH_FILES+=("${API_FILES[@]}")
ACTIVE_PATH_FILES+=("${TOOLS_FILES[@]}")

# ── Allowlist ──────────────────────────────────────────────────────────
#
# Hardcoded per WP 1 plan (docs/felix/current_platform_active_path_matrix.md §7).
# These will shrink as migration work packages complete.

# Files allowed to use g_h.funcs or ->_h_ (Rule 1)
# — WP 1 migrated serial_ll_if.c and moved h_rpc_slave_if.c to port layer;
#   API and tools migrated; remaining allowlist entries have documented reasons.
A1_GH_FUNCS=(
    "host/api/src/esp_hosted_api.c"       # 1 g_h.funcs call in #if 0 block (dead code)
)

# Files allowed to include serial_ll_if.h (Rule 2)
# — None. h_rpc_slave_if.c was moved to host/port/esp-idf/ (WP 1).
A2_SERIAL_LL_IF_H=(
)

# Files allowed to reference serial_ll_if_g->fops (Rule 3)
# — None. h_rpc_slave_if.c was moved to host/port/esp-idf/ (WP 1).
A3_SERIAL_LL_IF_FOPS=(
)

# Files allowed to use transport_pserial_* (Rule 4)
# — None. h_rpc_slave_if.c was moved to host/port/esp-idf/ (WP 1).
A4_TRANSPORT_PSERIAL=(
)

# Files allowed to have ESP-IDF / FreeRTOS calls in core (Rule 5)
# — None. h_rpc_slave_if.c was moved to host/port/esp-idf/ (WP 1).
A5_ESP_IDF=(
)

# ── Global Counters ────────────────────────────────────────────────────
VIOLATIONS=0
ALLOWLIST_HITS=0
CHECKS_PASSED=0
CHECKS_FAILED=0

# ── Helper Functions ───────────────────────────────────────────────────

# in_allowlist <file> <array_name>
# Returns 0 if $file matches any entry in the named allowlist array.
in_allowlist() {
    local file="$1"
    local -n arr="$2"
    local basename_file
    basename_file="$(basename "$file")"
    for entry in "${arr[@]}"; do
        if [ "$file" = "$entry" ] || [ "$basename_file" = "$(basename "$entry")" ]; then
            return 0
        fi
    done
    return 1
}

# grep_source <pattern> <files...>
# Greps pattern in source files, filtering out comment-only lines and
# REMOVED/CHECK annotation lines. Output is "file:lineno:content".
grep_source() {
    local pattern="$1"
    shift
    if [ $# -eq 0 ]; then
        return 0
    fi
    grep -rn "$pattern" "$@" --include="*.c" --include="*.h" 2>/dev/null \
        | grep -v ': *//' \
        | grep -v ': */\*' \
        | grep -v ':[[:space:]]*\*' \
        | grep -v ':.*// REMOVED:' \
        | grep -v ':.*// CHECK:' \
        || true
}

# run_rule_check <rule_name> <pattern> <allowlist_array_name> <scope_description> <files...>
#
# Greps <pattern> across <files>. For each hit:
#   - If the file is in the allowlist: prints [WARN] with reason, increments ALLOWLIST_HITS
#   - Otherwise: prints [FAIL], increments VIOLATIONS
#
# Always returns 0 (violations tracked in global VIOLATIONS).
run_rule_check() {
    local rule_name="$1"
    local pattern="$2"
    local allowlist_var="$3"
    local scope_desc="$4"
    shift 4
    local files=("$@")

    local hits
    hits="$(grep_source "$pattern" "${files[@]}" 2>/dev/null || true)"

    echo -e "${BOLD}── ${rule_name}${NC}"
    echo "   Scope: ${scope_desc}"

    if [ -z "$hits" ]; then
        echo -e "   ${GREEN}[PASS]${NC} No violations found."
        ((CHECKS_PASSED++)) || true
        echo ""
        return 0
    fi

    local rule_violations=0
    local rule_allowlist=0

    while IFS= read -r line; do
        [ -z "$line" ] && continue
        local hit_file="${line%%:*}"
        local hit_rest="${line#*:}"  # lineno:content
        local hit_lineno="${hit_rest%%:*}"

        if in_allowlist "$hit_file" "$allowlist_var"; then
            # Allowlist hit: warn but don't fail
            local reason=""
            case "$allowlist_var" in
                A1_GH_FUNCS)       reason="legacy calls being migrated" ;;
                A2_SERIAL_LL_IF_H) reason="moving to adapter WP 1" ;;
                A3_SERIAL_LL_IF_FOPS) reason="moving to adapter WP 1" ;;
                A4_TRANSPORT_PSERIAL) reason="pserial transport abstraction" ;;
                A5_ESP_IDF)        reason="moving to adapter WP 1" ;;
            esac
            echo -e "   ${YELLOW}[WARN]${NC} ${hit_file}:${hit_lineno}  (allowlisted: ${reason})"
            ((rule_allowlist++)) || true
        else
            echo -e "   ${RED}[FAIL]${NC} ${hit_file}:${hit_lineno}"
            ((rule_violations++)) || true
        fi
    done <<< "$hits"

    if [ "$rule_violations" -gt 0 ]; then
        echo -e "   ${RED}Result: ${rule_violations} violation(s)${NC}"
        ((CHECKS_FAILED++)) || true
    fi
    if [ "$rule_allowlist" -gt 0 ]; then
        echo -e "   ${YELLOW}Allowlist: ${rule_allowlist} hit(s)${NC}"
    fi

    VIOLATIONS=$((VIOLATIONS + rule_violations))
    ALLOWLIST_HITS=$((ALLOWLIST_HITS + rule_allowlist))

    echo ""
    return 0
}

# ── Rule 1: Legacy calls in active path ────────────────────────────────
#
# Active path files must NOT contain un-annotated g_h.funcs or ->_h_
# (the old vtable access pattern). These should use h_* wrapper macros.

# Build the Rule 1 check file set:
#   core/src/*.c + transport driver + new port + control adapters
R1_FILES=("${CORE_FILES[@]}")
[ ${#TRANSPORT_DRV_FILES[@]} -gt 0 ] && R1_FILES+=("${TRANSPORT_DRV_FILES[@]}")
[ -f "$PORT_TRANSPORT_FILE" ] && R1_FILES+=("$PORT_TRANSPORT_FILE")
R1_FILES+=("${CONTROL_ADAPTER_FILES[@]}")
R1_FILES+=("${API_FILES[@]}")
	R1_FILES+=("${TOOLS_FILES[@]}")

run_rule_check \
    "Rule 1: Legacy vtable calls (g_h.funcs / ->_h_ / HOSTED_* macros)" \
    'g_h\.funcs\|->_h_\|HOSTED_FREE\|HOSTED_CALLOC\|HOSTED_MALLOC\|HOSTED_FREE_HANDLE' \
    "A1_GH_FUNCS" \
    "core + transport driver + port + control adapters + API + tools" \
    "${R1_FILES[@]}"

# ── Rule 2: serial_ll_if.h in core ─────────────────────────────────────
#
# Core files must NOT include serial_ll_if.h. This is a control adapter header
# that should not leak into the core layer.

run_rule_check \
    "Rule 2: serial_ll_if.h include in core" \
    '#include.*serial_ll_if\.h' \
    "A2_SERIAL_LL_IF_H" \
    "core/src/*.c (excluding h_rpc_slave_if.c)" \
    "${CORE_FILES[@]}"

# ── Rule 3: serial_ll_if_g->fops in core ───────────────────────────────
#
# Core files must NOT access serial_ll_if_g->fops. This is a control adapter
# global that should not be directly referenced in the core layer.

run_rule_check \
    "Rule 3: serial_ll_if_g->fops in core" \
    'serial_ll_if_g->fops' \
    "A3_SERIAL_LL_IF_FOPS" \
    "core/src/*.c (excluding h_rpc_slave_if.c)" \
    "${CORE_FILES[@]}"

# ── Rule 4: transport_pserial_* in core ────────────────────────────────
#
# Core files must NOT use transport_pserial_* functions. These are the
# legacy pserial transport abstraction that should be accessed via
# h_serial_if instead.

run_rule_check \
    "Rule 4: transport_pserial_* in core" \
    'transport_pserial_' \
    "A4_TRANSPORT_PSERIAL" \
    "core/src/*.c" \
    "${CORE_FILES[@]}"

# ── Rule 5: ESP-IDF / FreeRTOS in core ─────────────────────────────────
#
# Core files must NOT contain direct ESP-IDF or FreeRTOS calls.
# This is the same check as scripts/check_core_isolation.sh but
# with allowlist support for files in transition.

R5_FILES=("${CORE_FILES[@]}")

# 5a: ESP-IDF includes
# Checks for specific ESP-IDF headers and FreeRTOS includes.
# Project headers (esp_hosted_*) are NOT ESP-IDF and are excluded by naming.
run_rule_check \
    "Rule 5a: ESP-IDF includes (esp_err.h / esp_wifi.h / esp_log.h / FreeRTOS)" \
    '#include.*"esp_err\.h"\|#include.*"esp_wifi\.h"\|#include.*"esp_log\.h"\|#include.*"esp_timer\.h"\|#include.*"esp_heap_caps\.h"\|#include.*"esp_private/\|#include.*"esp_event\.h"\|#include.*"esp_netif\b"\|#include.*freertos/' \
    "A5_ESP_IDF" \
    "core/src/*.c" \
    "${R5_FILES[@]}"

# 5b: FreeRTOS API calls
run_rule_check \
    "Rule 5b: FreeRTOS API calls (vTask*, xQueue*, xSemaphore*, etc.)" \
    'vTaskDelay\|vTaskDelete\|xQueueCreate\|xQueueSend\|xQueueReceive\|xSemaphoreCreate\|xSemaphoreTake\|xSemaphoreGive\|xTaskCreate\|portMAX_DELAY' \
    "A5_ESP_IDF" \
    "core/src/*.c" \
    "${R5_FILES[@]}"

# 5c: ESP-IDF logging macros
run_rule_check \
    "Rule 5c: ESP-IDF log macros (ESP_LOG*, ESP_EARLY_LOG*, ESP_ERROR_CHECK)" \
    'ESP_LOG\|ESP_EARLY_LOG\|ESP_ERROR_CHECK' \
    "A5_ESP_IDF" \
    "core/src/*.c" \
    "${R5_FILES[@]}"

# 5d: ESP-IDF types
run_rule_check \
    "Rule 5d: ESP-IDF types (esp_err_t, esp_event_base_t)" \
    '\besp_err_t\b\|\besp_event_base_t\b' \
    "A5_ESP_IDF" \
    "core/src/*.c" \
    "${R5_FILES[@]}"

# 5e: heap_caps and other ESP-IDF functions
run_rule_check \
    "Rule 5e: heap_caps / esp_wifi_internal / esp_event_loop / esp_netif / constructor" \
    '\bheap_caps_\|esp_wifi_internal_\|esp_event_loop_\|esp_netif_\|__attribute__.*constructor' \
    "A5_ESP_IDF" \
    "core/src/*.c" \
    "${R5_FILES[@]}"

# ── Coverage Report ────────────────────────────────────────────────────

# Count actual files that exist
FILES_EXIST=0
FILES_MISSING=0
for f in "${ACTIVE_PATH_FILES[@]}"; do
    if [ -f "$f" ]; then
        ((FILES_EXIST++)) || true
    else
        ((FILES_MISSING++)) || true
        echo -e "   ${YELLOW}[WARN]${NC} File not found (skipped): $f"
    fi
done

echo ""

# ── Summary ────────────────────────────────────────────────────────────
echo -e "${BOLD}=== Summary ===${NC}"
echo ""
echo "  Transport:           ${TRANSPORT}"
echo "  Active files listed: ${#ACTIVE_PATH_FILES[@]}"
echo "  Files present:       ${FILES_EXIST}"
if [ "$FILES_MISSING" -gt 0 ]; then
    echo "  Files missing:       ${FILES_MISSING}"
fi
echo "  Checks passed:       ${CHECKS_PASSED}"
echo "  Checks failed:       ${CHECKS_FAILED}"
echo "  Violations (FAIL):   ${VIOLATIONS}"
echo "  Allowlist hits:      ${ALLOWLIST_HITS}"
echo ""

if [ "$VIOLATIONS" -gt 0 ]; then
    echo -e "${RED}${BOLD}RESULT: FAIL${NC} — ${VIOLATIONS} isolation violation(s) found."
    echo ""
    echo "  These files in the current-platform active path use patterns that"
    echo "  should be accessed through the port contract (h_* wrappers) or"
    echo "  belong in adapter code outside core/."
    echo ""
    echo "  Refer to:"
    echo "    docs/felix/current_platform_active_path_matrix.md  (active path + allowlist)"
    echo "    docs/felix/22.Host通用框架移植到好型收口实施计划.md   (migration plan)"
    echo "    scripts/check_core_isolation.sh                    (portable core check)"
    exit 1
else
    echo -e "${GREEN}${BOLD}RESULT: PASS${NC} — No isolation violations in current-platform active path."
    if [ "$ALLOWLIST_HITS" -gt 0 ]; then
        echo ""
        echo "  ${ALLOWLIST_HITS} allowlisted item(s) found. These are known items"
        echo "  pending migration (see docs/felix/current_platform_active_path_matrix.md §7)."
    fi
    exit 0
fi
