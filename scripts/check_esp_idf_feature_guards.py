#!/usr/bin/env python3
"""Check ESP-IDF version feature guards for native API drift.

The ESP-IDF port can include headers from older IDF releases where some native
Wi-Fi types or fields are not declared. These source-level checks catch
unguarded references before the slower ESP-IDF build matrix reaches them.
"""

from pathlib import Path
import re
import sys


ROOT = Path(__file__).resolve().parents[1]


CHECKS = [
    {
        "path": "host/port/esp-idf/h_wifi_type_adapt.h",
        "tokens": [
            r"\bwifi_band_mode_t\b",
            r"\bwifi_protocols_t\b",
            r"\bwifi_bandwidths_t\b",
        ],
        "guard": "H_WIFI_DUALBAND_SUPPORT",
        "why": "IDF v5.3 may not declare dual-band native helper types",
    },
    {
        "path": "host/port/esp-idf/h_wifi_type_adapt.c",
        "tokens": [
            r"\bwifi_band_mode_t\b",
            r"\bwifi_protocols_t\b",
            r"\bwifi_bandwidths_t\b",
            r"\bWIFI_BAND_MODE_[A-Z0-9_]+\b",
        ],
        "guard": "H_WIFI_DUALBAND_SUPPORT",
        "why": "IDF v5.3 may not declare dual-band native helper types",
    },
    {
        "path": "host/api/src/esp_hosted_api.c",
        "tokens": [r"config->twt_enable_keep_alive\b"],
        "guard": "H_GOT_TWT_ENABLE_KEEP_ALIVE",
        "why": "wifi_twt_config_t::twt_enable_keep_alive is absent in IDF v5.3.0",
    },
]

FORBIDDEN_LINE_CHECKS = [
    {
        "path": "host/port/esp-idf/h_wifi_type_adapt.h",
        "tokens": [r'^\s*#\s*include\s+"h_port_config\.h"'],
        "why": "adapter headers are included by Linux mock stubs and must not pull ESP-IDF-only port config",
    },
]


def strip_block_comments_preserve_lines(text: str) -> str:
    def repl(match: re.Match[str]) -> str:
        return "\n" * match.group(0).count("\n")

    return re.sub(r"/\*.*?\*/", repl, text, flags=re.S)


def update_guard_stack(line: str, stack: list[str]) -> None:
    match = re.match(r"^\s*#\s*(if|ifdef|ifndef|elif)\b(.*)", line)
    if match:
        kind = match.group(1)
        expr = match.group(2).strip()
        if kind in {"if", "ifdef", "ifndef"}:
            stack.append(expr)
        elif stack:
            stack[-1] = expr
        return

    if re.match(r"^\s*#\s*else\b", line):
        if stack:
            stack[-1] = f"else({stack[-1]})"
        return

    if re.match(r"^\s*#\s*endif\b", line):
        if stack:
            stack.pop()


def is_under_guard(stack: list[str], guard: str) -> bool:
    return any(guard in expr for expr in stack)


def check_file(spec: dict[str, object]) -> list[str]:
    path = ROOT / str(spec["path"])
    text = strip_block_comments_preserve_lines(path.read_text(encoding="utf-8"))
    token_re = re.compile("|".join(str(token) for token in spec["tokens"]))
    guard = str(spec["guard"])
    why = str(spec["why"])
    stack: list[str] = []
    failures: list[str] = []

    for lineno, raw_line in enumerate(text.splitlines(), start=1):
        line = re.sub(r"//.*", "", raw_line)
        update_guard_stack(line, stack)
        if token_re.search(line) and not is_under_guard(stack, guard):
            failures.append(f"{spec['path']}:{lineno}: missing {guard} guard: {why}")

    return failures


def check_forbidden_lines(spec: dict[str, object]) -> list[str]:
    path = ROOT / str(spec["path"])
    text = strip_block_comments_preserve_lines(path.read_text(encoding="utf-8"))
    token_re = re.compile("|".join(str(token) for token in spec["tokens"]))
    why = str(spec["why"])
    failures: list[str] = []

    for lineno, raw_line in enumerate(text.splitlines(), start=1):
        line = re.sub(r"//.*", "", raw_line)
        if token_re.search(line):
            failures.append(f"{spec['path']}:{lineno}: forbidden line: {why}")

    return failures


def main() -> int:
    failures: list[str] = []
    for spec in CHECKS:
        failures.extend(check_file(spec))
    for spec in FORBIDDEN_LINE_CHECKS:
        failures.extend(check_forbidden_lines(spec))

    if failures:
        print("ERROR: ESP-IDF feature guard regressions found")
        for failure in failures:
            print(f"  {failure}")
        return 1

    print("OK: ESP-IDF version feature guards are consistent")
    return 0


if __name__ == "__main__":
    sys.exit(main())
