#!/usr/bin/env python3
"""
Empirical verification of 1-factorization schedule invariants for K_10 tournament.
Tests canonical schedule from shared_tournament_fixtures.json as well as algorithmic rules.
"""

import json
import sys
from pathlib import Path
from collections import defaultdict

def verify_schedule():
    repo_root = Path(__file__).resolve().parent.parent
    fixtures_path = repo_root / "tests" / "test_fixtures" / "shared_tournament_fixtures.json"

    with open(fixtures_path, "r", encoding="utf-8") as f:
        data = json.load(f)

    schedule = data.get("canonical_schedule_45", [])
    print(f"[1] Loaded schedule with {len(schedule)} matches.")

    errors = []

    # 1. Total matches == 45
    if len(schedule) != 45:
        errors.append(f"Expected 45 matches, got {len(schedule)}")

    # 2. 9 rounds of 5 matches
    rounds = [schedule[i:i+5] for i in range(0, len(schedule), 5)]
    if len(rounds) != 9:
        errors.append(f"Expected 9 rounds of 5 matches, got {len(rounds)}")

    # Check 1-factor in each round: all 10 styles appear exactly once per round
    for r_idx, r in enumerate(rounds):
        round_styles = []
        for m in r:
            round_styles.extend([m["left_style_id"], m["right_style_id"]])
        if sorted(round_styles) != list(range(1, 11)):
            errors.append(f"Round {r_idx} is not a 1-factor on K_10: styles = {sorted(round_styles)}")

    # 3. All 45 unique unordered pairs
    pairs = set()
    for m in schedule:
        u, v = m["left_style_id"], m["right_style_id"]
        if u == v:
            errors.append(f"Match {m['match_index']} has self-play: {u} vs {v}")
        pair = (min(u, v), max(u, v))
        if pair in pairs:
            errors.append(f"Duplicate pair {pair} in match {m['match_index']}")
        pairs.add(pair)

    if len(pairs) != 45:
        errors.append(f"Expected 45 distinct unordered pairs, got {len(pairs)}")

    # 4. Quick tournament: first 15 matches (first 3 rounds)
    quick_schedule = schedule[:15]
    quick_counts = defaultdict(int)
    for m in quick_schedule:
        quick_counts[m["left_style_id"]] += 1
        quick_counts[m["right_style_id"]] += 1

    for s in range(1, 11):
        if quick_counts[s] != 3:
            errors.append(f"Quick tournament style {s} appears {quick_counts[s]} times, expected 3 (3-regular)")

    # 5. Spacing constraint:
    # For every match pair (u, v), does style u and style v appear at least 4 matches apart from their previous appearance?
    # Let's check distance: match_index_current - match_index_previous
    last_seen = {}
    spacing_violations = []
    min_observed_spacing = 999
    for idx, m in enumerate(schedule):
        u, v = m["left_style_id"], m["right_style_id"]
        for s in (u, v):
            if s in last_seen:
                dist = idx - last_seen[s]
                min_observed_spacing = min(min_observed_spacing, dist)
                if dist < 4:
                    spacing_violations.append(f"Style {s} at match {idx} previously appeared at match {last_seen[s]} (distance {dist} < 4)")
            last_seen[s] = idx

    if spacing_violations:
        errors.extend(spacing_violations)
    else:
        print(f"[PASS] Spacing constraint verified. Minimum observed spacing = {min_observed_spacing} (>= 4 matches apart).")

    # 6. Left/Right presentation balance:
    # In 45 matches, does each style appear on the left 4 to 5 times and on the right 4 to 5 times?
    left_counts = defaultdict(int)
    right_counts = defaultdict(int)
    for m in schedule:
        left_counts[m["left_style_id"]] += 1
        right_counts[m["right_style_id"]] += 1

    for s in range(1, 11):
        l = left_counts[s]
        r = right_counts[s]
        if l < 4 or l > 5 or r < 4 or r > 5 or (l + r) != 9:
            errors.append(f"Style {s} balance violation: left={l}, right={r} (expected 4..5 each, sum=9)")

    print("\n--- Left/Right Distribution ---")
    for s in range(1, 11):
        print(f"Style {s:2d}: Left={left_counts[s]}, Right={right_counts[s]}, Total={left_counts[s] + right_counts[s]}")

    if errors:
        print(f"\n[FAIL] {len(errors)} invariant violations found:")
        for err in errors:
            print("  - " + err)
        return False
    else:
        print("\n[SUCCESS] All Schedule Invariants PASSED strictly.")
        return True

if __name__ == "__main__":
    success = verify_schedule()
    sys.exit(0 if success else 1)
