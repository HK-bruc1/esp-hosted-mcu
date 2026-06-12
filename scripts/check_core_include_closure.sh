#!/bin/bash
# scripts/check_core_include_closure.sh
#
# Checks for transitive ESP-IDF dependency leaks in the core portable boundary.
#
# Unlike check_core_isolation.sh (which checks direct includes and symbol usage),
# this script checks for:
#   1. Core files including headers from host/port/esp-idf/ (crossing the port boundary)
#   2. Core files using ESP-IDF native types not caught by the existing isolation check
#   3. Core files including project headers that transitively pull in ESP-IDF headers
#
# Mode: --report (default) prints findings but exits 0
#       --strict exits 1 on any finding
#
# Reference: tasks/host_framework_optimization_plan.md Phase 0

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$REPO_ROOT"

# Parse options
STRICT_MODE=false
for arg in "$@"; do
    case "$arg" in
        --strict|-s) STRICT_MODE=true ;;
        --report) STRICT_MODE=false ;;
        *) echo "Usage: $0 [--strict|--report]" ; exit 1 ;;
    esac
done

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BOLD='\033[1m'
NC='\033[0m'

CORE_SRC_DIR="host/core/src"
CORE_INC_DIR="host/core/include"

FINDINGS=0

echo ""
echo -e "${BOLD}=== Core Include-Closure Leak Check ===${NC}"
echo ""

# Check 1: Core includes port-specific headers
echo -e "${BOLD}-- Check 1: Core files must not include host/port/esp-idf/ headers --${NC}"

C1_HITS=""
for f in "${CORE_SRC_DIR}"/*.c "${CORE_INC_DIR}/h_public"/*.h "${CORE_INC_DIR}/h_internal"/*.h; do
    [ -f "$f" ] || continue
    while IFS=: read -r lineno content; do
        # Extract the included header name (quoted includes only)
        inc_header="$(echo "$content" | sed -n 's/#include.*"\([^"]*\)".*/\1/p')"
        [ -z "$inc_header" ] && continue
        # Does this header exist in host/port/esp-idf/?
        if [ -f "host/port/esp-idf/${inc_header}" ]; then
            C1_HITS="${C1_HITS}${f}:${lineno}: includes ${inc_header} from host/port/esp-idf/
"
        fi
    done < <(grep -n '#include' "$f" 2>/dev/null | grep -v '// REMOVED:' | grep -v '// CHECK:' || true)
done

if [ -z "$C1_HITS" ]; then
    echo -e "   ${GREEN}[PASS]${NC} No core file includes headers from host/port/esp-idf/"
else
    if $STRICT_MODE; then
        echo -e "   ${RED}[FAIL]${NC} Core files include port-specific headers:"
    else
        echo -e "   ${YELLOW}[REPORT]${NC} Core files include port-specific headers:"
    fi
    echo "$C1_HITS" | while IFS= read -r hit; do
        [ -z "$hit" ] && continue
        if $STRICT_MODE; then
            echo -e "   ${RED}${hit}${NC}"
        else
            echo -e "   ${YELLOW}${hit}${NC}"
        fi
    done
    FINDINGS=$((FINDINGS + 1))
fi
echo ""

# Check 2: Core uses ESP-IDF native types beyond existing isolation scope
echo -e "${BOLD}-- Check 2: ESP-IDF native type usage in core (beyond check_core_isolation.sh scope) --${NC}"

# Types that check_core_isolation.sh does NOT currently catch (commented-out or
# only checking direct includes).  This check uses report-only by default;
# --strict makes it a failure.
C2_PATTERN='\besp_supp_dpp_[A-Za-z0-9_]*\b|\bwifi_event_[A-Za-z0-9_]*\b|\bWIFI_EVENT_[A-Z0-9_]*\b|\bwifi_init_config_t\b|\bwifi_config_t\b|\bwifi_ap_record_t\b|\bwifi_scan_config_t\b|\bwifi_sta_list_t\b|\bwifi_country_t\b|\bwifi_phy_mode_t\b|\bwifi_storage_t\b|\bwifi_second_chan_t\b|\bwifi_band_t\b|\bwifi_band_mode_t\b|\bwifi_protocols_t\b|\bwifi_bandwidths_t\b|\bwifi_twt_config_t\b|\bwifi_twt_setup_config_t\b|\bwifi_itwt_setup_config_t\b|\besp_hosted_event_init_t\b|\bESP_HOSTED_EVENT_[A-Za-z0-9_]*\b|\bCONFIG_ESP_HOSTED_[A-Za-z0-9_]*\b|\bCONFIG_ESP_WIFI_[A-Za-z0-9_]*\b'

C2_HITS=""
for f in "${CORE_SRC_DIR}"/*.c; do
    [ -f "$f" ] || continue
    hits="$(grep -HnE "$C2_PATTERN" "$f" 2>/dev/null \
        | grep -v '// REMOVED:' \
        | grep -v '// CHECK:' \
        | grep -v '^\s*\*' \
        | grep -v '^\s*//' \
        || true)"
    if [ -n "$hits" ]; then
        C2_HITS="${C2_HITS}${hits}
"
    fi
done

if [ -z "$C2_HITS" ]; then
    echo -e "   ${GREEN}[PASS]${NC} No ESP-IDF native type usage beyond isolation scope"
else
    hit_count="$(echo "$C2_HITS" | wc -l)"
    if $STRICT_MODE; then
        echo -e "   ${RED}[FAIL]${NC} ${hit_count} ESP-IDF native type usage(s) in core:"
    else
        echo -e "   ${YELLOW}[REPORT]${NC} ${hit_count} ESP-IDF native type usage(s) in core (known transition items):"
    fi
    echo "$C2_HITS" | head -20 | while IFS= read -r hit; do
        [ -z "$hit" ] && continue
        if $STRICT_MODE; then
            echo -e "   ${RED}${hit}${NC}"
        else
            echo -e "   ${YELLOW}${hit}${NC}"
        fi
    done
    if [ "$hit_count" -gt 20 ]; then
        echo -e "   ... ($((hit_count - 20)) more, run script locally for full list)"
    fi
    FINDINGS=$((FINDINGS + 1))
fi
echo ""

# Check 3: Transitive ESP-IDF header dependency
echo -e "${BOLD}-- Check 3: Transitive ESP-IDF header dependency chains --${NC}"

# For each header included by core files, check if it itself includes ESP-IDF headers.
# This catches chains like: h_rpc_wrap.c -> h_wifi_type_adapt.h -> esp_wifi.h

C3_HITS=""
for f in "${CORE_SRC_DIR}"/*.c; do
    [ -f "$f" ] || continue
    while IFS=: read -r lineno content; do
        inc_header="$(echo "$content" | sed -n 's/#include.*"\([^"]*\)".*/\1/p')"
        [ -z "$inc_header" ] && continue

        # Find the actual header file (search multiple include paths)
        header_path=""
        for dir in "${CORE_INC_DIR}/h_public" "${CORE_INC_DIR}/h_internal" \
                   "host/port/include" "host" "host/api/include" \
                   "host/port/esp-idf" "common" "common/transport" \
                   "common/rpc" "common/proto" "common/log" "common/mempool/include"; do
            if [ -f "${dir}/${inc_header}" ]; then
                header_path="${dir}/${inc_header}"
                break
            fi
        done

        [ -z "$header_path" ] && continue

        # Check if this header includes ESP-IDF system headers.  Project headers
        # named esp_hosted_* are not ESP-IDF system headers.
        esp_idf_lines="$(grep -E '#include.*["<]esp_[^">]*[">]' "$header_path" 2>/dev/null \
            | grep -vE '#include.*["<]esp_hosted_' || true)"
        freertos_lines="$(grep -E '#include.*["<]freertos/' "$header_path" 2>/dev/null || true)"
        esp_idf_includes="$(printf '%s\n' "$esp_idf_lines" | grep -c . || true)"
        freertos_includes="$(printf '%s\n' "$freertos_lines" | grep -c . || true)"

        if [ "$esp_idf_includes" -gt 0 ] 2>/dev/null || [ "$freertos_includes" -gt 0 ] 2>/dev/null; then
            # List the ESP-IDF headers pulled in
            transitive="$(printf '%s\n%s\n' "$esp_idf_lines" "$freertos_lines" \
                | grep -v '^$' \
                | sed 's/#include.*"\([^"]*\)".*/\1/' \
                | sed 's/#include.*<\([^>]*\)>.*/\1/' \
                | tr '\n' ',' \
                | sed 's/,$//')"
            C3_HITS="${C3_HITS}${f}:${lineno}: ${inc_header} -> [${transitive}]
"
        fi
    done < <(grep -n '#include' "$f" 2>/dev/null | grep -v '// REMOVED:' | grep -v '// CHECK:' || true)
done

if [ -z "$C3_HITS" ]; then
    echo -e "   ${GREEN}[PASS]${NC} No transitive ESP-IDF header dependency chains found"
else
    if $STRICT_MODE; then
        echo -e "   ${RED}[FAIL]${NC} Transitive ESP-IDF dependency chains:"
    else
        echo -e "   ${YELLOW}[REPORT]${NC} Transitive ESP-IDF dependency chains:"
    fi
    echo "$C3_HITS" | while IFS= read -r hit; do
        [ -z "$hit" ] && continue
        if $STRICT_MODE; then
            echo -e "   ${RED}${hit}${NC}"
        else
            echo -e "   ${YELLOW}${hit}${NC}"
        fi
    done
    FINDINGS=$((FINDINGS + 1))
fi
echo ""

# Check 4: Always-defined boolean H_* flags must use value checks
echo -e "${BOLD}-- Check 4: Always-defined boolean H_* flags use #if/#if ! --${NC}"

C4_PATTERN='^[[:space:]]*#ifn?def[[:space:]]+H_(WIFI_AUTO_CONNECT_ON_STA_START|FW_VERSION_MISMATCH_WARNING_SUPPRESS|WIFI_NVS_ENABLED|PEER_DATA_TRANSFER)\b'
C4_HITS=""
for f in "${CORE_SRC_DIR}"/*.c "${CORE_INC_DIR}/h_public"/*.h "${CORE_INC_DIR}/h_internal"/*.h; do
    [ -f "$f" ] || continue
    hits="$(grep -HnE "$C4_PATTERN" "$f" 2>/dev/null || true)"
    if [ -n "$hits" ]; then
        C4_HITS="${C4_HITS}${hits}
"
    fi
done

if [ -z "$C4_HITS" ]; then
    echo -e "   ${GREEN}[PASS]${NC} No #ifdef/#ifndef usage for always-defined boolean H_* flags"
else
    if $STRICT_MODE; then
        echo -e "   ${RED}[FAIL]${NC} Always-defined boolean H_* flags used with #ifdef/#ifndef:"
    else
        echo -e "   ${YELLOW}[REPORT]${NC} Always-defined boolean H_* flags used with #ifdef/#ifndef:"
    fi
    echo "$C4_HITS" | while IFS= read -r hit; do
        [ -z "$hit" ] && continue
        if $STRICT_MODE; then
            echo -e "   ${RED}${hit}${NC}"
        else
            echo -e "   ${YELLOW}${hit}${NC}"
        fi
    done
    FINDINGS=$((FINDINGS + 1))
fi
echo ""

# Summary
echo -e "${BOLD}=== Summary ===${NC}"
echo ""
if $STRICT_MODE; then
    echo "  Mode:    --strict (findings = failures)"
else
    echo "  Mode:    --report (findings are informational)"
fi
echo "  Findings: $FINDINGS"
echo ""

if $STRICT_MODE && [ "$FINDINGS" -gt 0 ]; then
    echo -e "${RED}${BOLD}RESULT: FAIL${NC} -- $FINDINGS closure leak(s) found in strict mode."
    echo ""
    echo "  These must be resolved before core can compile without ESP-IDF headers."
    exit 1
else
    echo -e "${GREEN}${BOLD}RESULT: OK${NC} -- Report complete."
    if [ "$FINDINGS" -gt 0 ]; then
        echo "  Some findings were reported. Run with --strict to treat them as failures."
    fi
    exit 0
fi
