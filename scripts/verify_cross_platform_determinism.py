#!/usr/bin/env python3
"""
Cross-Platform Determinism & Mathematical Oracle Verification Script.
Tests all 5 test cases in shared_tournament_fixtures.json against pure reference math.
"""

import json
import math
import sys
from pathlib import Path

def solve_bradley_terry(win_matrix, epsilon=1e-6, max_iter=50):
    n = len(win_matrix)
    wins = [sum(win_matrix[i][j] for j in range(n) if i != j) for i in range(n)]
    played = [sum(win_matrix[i][j] + win_matrix[j][i] for j in range(n) if i != j) for i in range(n)]

    # Check for undefeated or winless items
    has_boundary = any(played[i] > 0 and (wins[i] == played[i] or wins[i] == 0) for i in range(n))

    if has_boundary:
        # Laplace smoothing fallback
        raw_p = [(wins[i] + 1.0) / (played[i] + 2.0) for i in range(n)]
        sum_p = sum(raw_p)
        p = [x / sum_p for x in raw_p]
        return "LaplaceFallback", 0, p

    # Bradley-Terry MM
    p = [1.0 / n] * n
    iters = 0
    for it in range(1, max_iter + 1):
        iters = it
        p_next = [0.0] * n
        for i in range(n):
            if wins[i] == 0:
                p_next[i] = 0.0
                continue
            denom = 0.0
            for j in range(n):
                if i != j:
                    total_ij = win_matrix[i][j] + win_matrix[j][i]
                    if total_ij > 0:
                        denom += total_ij / (p[i] + p[j])
            p_next[i] = wins[i] / denom if denom > 1e-12 else 0.0

        sum_next = sum(p_next)
        if sum_next > 0.0:
            p_next = [x / sum_next for x in p_next]

        max_diff = max(abs(p_next[i] - p[i]) for i in range(n))
        p = p_next
        if max_diff < epsilon:
            break

    return "BradleyTerryMM", iters, p

def calculate_circular_triads(win_matrix, total_matches):
    n = len(win_matrix)
    wins = [sum(win_matrix[i][j] for j in range(n) if i != j) for i in range(n)]
    scale = 1.0 if (total_matches == 45 or total_matches <= 0) else (45.0 / total_matches)
    sum_s_sq = sum((w * scale) ** 2 for w in wins)
    c = 142.5 - 0.5 * sum_s_sq
    return max(0.0, min(40.0, c))

def calculate_zeta(c):
    return max(0.0, min(1.0, 1.0 - c / 40.0))

def calculate_confidence(zeta, delta, ch):
    raw_conf = (0.45 * zeta + 0.35 * math.tanh(3.0 * delta) + 0.20 * ch) * 100.0
    return max(15.0, min(99.0, raw_conf))

def calculate_entropy(p):
    h = 0.0
    for prob in p:
        if prob > 1e-12:
            h -= prob * math.log(prob)
    return h

def main():
    repo_root = Path(__file__).resolve().parent.parent
    fixtures_path = repo_root / "tests" / "test_fixtures" / "shared_tournament_fixtures.json"
    styles_path = repo_root / "shared" / "data" / "styles.json"

    with open(fixtures_path, "r", encoding="utf-8") as f:
        fixtures_data = json.load(f)

    with open(styles_path, "r", encoding="utf-8") as f:
        styles_data = json.load(f)

    styles = {s["id"]: s for s in styles_data["styles"]}
    categories = {c["id"]: c for c in styles_data["categories"]}

    schedule = fixtures_data["canonical_schedule_45"]

    print("================================================================")
    print(" Verifying shared_tournament_fixtures.json with Python Oracle   ")
    print("================================================================")

    all_passed = True

    for tc in fixtures_data["test_cases"]:
        tc_id = tc["id"]
        tc_name = tc["name"]
        mode = tc["mode"]
        match_count = tc["match_count"]
        winners = tc["match_winners"]
        expected = tc["expected"]

        print(f"\n--- Checking {tc_id}: {tc_name} ({mode}, {match_count} matches) ---")

        # Build win matrix
        win_matrix = [[0] * 10 for _ in range(10)]
        for idx in range(match_count):
            m = schedule[idx]
            w = winners[idx]
            u, v = m["left_style_id"], m["right_style_id"]
            loser = v if w == u else u
            win_matrix[w - 1][loser - 1] += 1

        # Bradley-Terry / Laplace
        algo, iters, p = solve_bradley_terry(win_matrix)
        print(f"Algorithm: {algo} (expected: {expected['algorithm']}), iters: {iters} (expected: {expected['iterations']})")
        if algo != expected["algorithm"]:
            print(f"  [MISMATCH] Algorithm mismatch: got {algo}, expected {expected['algorithm']}")
            all_passed = False
        if iters != expected["iterations"]:
            print(f"  [MISMATCH] Iterations mismatch: got {iters}, expected {expected['iterations']}")
            all_passed = False

        # Sorted probabilities (stable sort)
        indexed_p = [(i + 1, p[i]) for i in range(10)]
        # Stable sort descending by probability
        ranked_styles = sorted(indexed_p, key=lambda x: x[1], reverse=True)
        dominant_style_id = ranked_styles[0][0]
        delta = ranked_styles[0][1] - ranked_styles[1][1]

        # Triads & Zeta
        c = calculate_circular_triads(win_matrix, match_count)
        zeta = calculate_zeta(c)

        # Entropy
        h = calculate_entropy(p)
        ch = 1.0 - (h / math.log(10.0))
        conf = calculate_confidence(zeta, delta, ch)

        print(f"Dominant Style: {dominant_style_id} (expected {expected['dominant_style_id']})")
        print(f"Circular Triads: {c:.4f} (expected {expected['circular_triads']})")
        print(f"Zeta: {zeta:.4f} (expected {expected['zeta']})")
        print(f"Delta: {delta:.6f} (expected {expected['delta']:.6f})")
        print(f"Confidence: {conf:.4f}% (expected {expected['confidence_pct']}%)")

        if dominant_style_id != expected["dominant_style_id"]:
            print(f"  [MISMATCH] Dominant style ID")
            all_passed = False
        if abs(c - expected["circular_triads"]) > 0.01:
            print(f"  [MISMATCH] Circular triads")
            all_passed = False
        if abs(zeta - expected["zeta"]) > 1e-4:
            print(f"  [MISMATCH] Zeta")
            all_passed = False
        if abs(delta - expected["delta"]) > 1e-4:
            print(f"  [MISMATCH] Delta")
            all_passed = False
        if abs(conf - expected["confidence_pct"]) > 0.01:
            print(f"  [MISMATCH] Confidence %")
            all_passed = False

        # Category Aggregation
        cat_scores = {c_id: 0.0 for c_id in categories}
        for s_id, prob in indexed_p:
            c_id = styles[s_id]["category_id"]
            cat_scores[c_id] += prob

        ranked_cats = sorted(cat_scores.items(), key=lambda x: x[1], reverse=True)
        dominant_cat = ranked_cats[0][0]
        print(f"Dominant Category: {dominant_cat} (expected {expected['dominant_category']})")
        if dominant_cat != expected["dominant_category"]:
            print(f"  [MISMATCH] Dominant category")
            all_passed = False

        # Category percentages check
        for exp_cat in expected.get("category_rankings", []):
            cid = exp_cat["category_id"]
            actual_pct = cat_scores[cid] * 100.0
            exp_pct = exp_cat["percentage"]
            if abs(actual_pct - exp_pct) > 0.05:
                print(f"  [MISMATCH] Category {cid} pct: got {actual_pct:.4f}%, expected {exp_pct:.4f}%")
                all_passed = False

        # Style rankings check
        for exp_s in expected.get("style_rankings", []):
            sid = exp_s["style_id"]
            actual_pct = p[sid - 1] * 100.0
            exp_pct = exp_s["percentage"]
            if abs(actual_pct - exp_pct) > 0.05:
                print(f"  [MISMATCH] Style {sid} pct: got {actual_pct:.4f}%, expected {exp_pct:.4f}%")
                all_passed = False

    print("\n================================================================")
    if all_passed:
        print("[SUCCESS] All 5 shared fixtures matched reference oracle perfectly!")
    else:
        print("[FAILURE] Some fixtures exhibited discrepancies!")
    print("================================================================")
    return all_passed

if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)
