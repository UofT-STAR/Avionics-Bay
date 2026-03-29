#!/usr/bin/env python3
"""
Offline replay checks for rocket flight logs.

This script validates key invariants from DATALOG.TXT so flight software
changes (like logging a different accel axis) are easier to catch.
"""

from __future__ import annotations

import argparse
import re
import sys
from dataclasses import dataclass
from pathlib import Path
from statistics import mean


NUMERIC_ROW = re.compile(r"^\s*(-?\d+)\s*,\s*(-?\d+)\s*,\s*(-?\d+)\s*,\s*(-?\d+)\s*$")
THREE_COL_ROW = re.compile(r"^\s*(-?\d+)\s*,\s*(-?\d+)\s*,\s*(-?\d+)\s*$")


@dataclass
class Row:
    state: int
    alt_dm: int
    acc_cg: int
    temp_cc: int


@dataclass
class CheckResult:
    name: str
    ok: bool
    detail: str


def parse_rows(path: Path) -> tuple[list[Row], int | None]:
    rows: list[Row] = []
    transition_line: int | None = None
    inferred_state = 0

    with path.open("r", encoding="utf-8", errors="ignore") as f:
        for i, raw in enumerate(f, start=1):
            line = raw.strip()
            if not line:
                continue

            if "PRE_LAUNCH->ASCENT" in line and transition_line is None:
                transition_line = i
                inferred_state = 1
                continue

            m = NUMERIC_ROW.match(line)
            if m:
                state, alt_dm, acc_cg, temp_cc = (int(m.group(1)), int(m.group(2)), int(m.group(3)), int(m.group(4)))
                inferred_state = state
                rows.append(Row(state=state, alt_dm=alt_dm, acc_cg=acc_cg, temp_cc=temp_cc))
                continue

            # Some logs contain 3-column rows near transitions: alt,acc,temp.
            m3 = THREE_COL_ROW.match(line)
            if m3:
                alt_dm, acc_cg, temp_cc = (int(m3.group(1)), int(m3.group(2)), int(m3.group(3)))
                rows.append(Row(state=inferred_state, alt_dm=alt_dm, acc_cg=acc_cg, temp_cc=temp_cc))

    return rows, transition_line


def check_has_data(rows: list[Row]) -> CheckResult:
    return CheckResult(
        name="has-data",
        ok=len(rows) > 100,
        detail=f"parsed rows={len(rows)} (expected >100)",
    )


def check_transition(rows: list[Row], transition_line: int | None) -> CheckResult:
    first_ascent_idx = next((idx for idx, r in enumerate(rows) if r.state == 1), None)
    ok = transition_line is not None and first_ascent_idx is not None
    detail = (
        f"transition_line={transition_line}, first_state_1_index={first_ascent_idx}"
        if ok
        else "missing transition marker or state=1 samples"
    )
    return CheckResult(name="transition-present", ok=ok, detail=detail)


def check_prelaunch_acc_baseline(rows: list[Row], low: int, high: int, near_target_band: int) -> CheckResult:
    pre = [r.acc_cg for r in rows if r.state == 0]
    if len(pre) < 50:
        return CheckResult("prelaunch-acc-baseline", False, f"too few prelaunch samples ({len(pre)})")

    avg = mean(pre)
    near_100_ratio = sum(1 for v in pre if abs(v - 100) <= near_target_band) / len(pre)

    # Baseline around +1g is a useful sanity check for a vertical axis at rest.
    ok = (low <= avg <= high) and (near_100_ratio >= 0.40)
    return CheckResult(
        name="prelaunch-acc-baseline",
        ok=ok,
        detail=(
            f"mean={avg:.2f}cg expected [{low},{high}], "
            f"near_100_ratio={near_100_ratio:.2%} expected >=40%"
        ),
    )


def check_launch_impulse(rows: list[Row], min_impulse_cg: int) -> CheckResult:
    pre = [r.acc_cg for r in rows if r.state == 0]
    if not pre:
        return CheckResult("launch-impulse", False, "no prelaunch rows")

    peak = max(pre)
    ok = peak >= min_impulse_cg
    return CheckResult(
        name="launch-impulse",
        ok=ok,
        detail=f"prelaunch_peak={peak}cg expected >= {min_impulse_cg}cg",
    )


def check_ascent_altitude_profile(rows: list[Row]) -> CheckResult:
    ascent = [r.alt_dm for r in rows if r.state == 1]
    if len(ascent) < 10:
        return CheckResult("ascent-profile", False, f"too few ascent samples ({len(ascent)})")

    peak = max(ascent)
    first = ascent[0]
    last = ascent[-1]

    # A short ascent window should usually contain a rise and then decline toward descent.
    ok = (peak - first >= 100) and (peak - last >= 500)
    return CheckResult(
        name="ascent-profile",
        ok=ok,
        detail=f"first={first}dm peak={peak}dm last={last}dm",
    )


def run_checks(args: argparse.Namespace) -> int:
    path = Path(args.log)
    if not path.exists():
        print(f"ERROR: file not found: {path}")
        return 2

    rows, transition_line = parse_rows(path)

    checks = [
        check_has_data(rows),
        check_transition(rows, transition_line),
        check_prelaunch_acc_baseline(
            rows,
            low=args.prelaunch_mean_low,
            high=args.prelaunch_mean_high,
            near_target_band=args.near_100_band,
        ),
        check_launch_impulse(rows, min_impulse_cg=args.min_launch_impulse),
        check_ascent_altitude_profile(rows),
    ]

    failed = [c for c in checks if not c.ok]

    print(f"Log: {path}")
    print(f"Rows: {len(rows)}")
    print("\nChecks:")
    for c in checks:
        status = "PASS" if c.ok else "FAIL"
        print(f" - [{status}] {c.name}: {c.detail}")

    if failed:
        print(f"\nResult: FAIL ({len(failed)} failed check(s))")
        return 1

    print("\nResult: PASS")
    return 0


def build_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(description="Replay-validate a rocket DATALOG.TXT")
    p.add_argument("--log", required=True, help="Path to DATALOG.TXT")
    p.add_argument("--prelaunch-mean-low", type=int, default=60, help="Lower bound for prelaunch mean accel (cg)")
    p.add_argument("--prelaunch-mean-high", type=int, default=140, help="Upper bound for prelaunch mean accel (cg)")
    p.add_argument("--near-100-band", type=int, default=20, help="Band around 100cg considered near 1g")
    p.add_argument("--min-launch-impulse", type=int, default=180, help="Minimum peak accel in prelaunch rows (cg)")
    return p


if __name__ == "__main__":
    parser = build_parser()
    sys.exit(run_checks(parser.parse_args()))
