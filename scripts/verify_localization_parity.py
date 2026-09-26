#!/usr/bin/env python3
"""
scripts/verify_localization_parity.py
Adversarial Verification of Android String Localization Parity.

Verifies:
1. XML syntactical validity of default (EN) and Italian (IT) string resources.
2. 100% key parity (symmetric set difference is empty).
3. Zero empty, null, or whitespace-only string values in either resource.
4. No duplicate keys within either resource file.
5. Format specifier consistency across locales (%1$d, %1$s, etc.).
"""

import sys
import os
import re
import xml.etree.ElementTree as ET
from pathlib import Path

def parse_strings_xml(file_path: Path):
    if not file_path.exists():
        raise FileNotFoundError(f"Strings file not found: {file_path}")

    # Parse with ET
    tree = ET.parse(file_path)
    root = tree.getroot()
    if root.tag != "resources":
        raise ValueError(f"Root tag in {file_path} is {root.tag}, expected <resources>")

    keys = []
    key_values = {}
    duplicates = []

    for child in root:
        if child.tag == "string":
            name = child.get("name")
            if not name:
                raise ValueError(f"Found <string> without 'name' attribute in {file_path}")
            
            if name in key_values:
                duplicates.append(name)
            
            keys.append(name)
            text_val = child.text if child.text is not None else ""
            key_values[name] = text_val

    return keys, key_values, duplicates

def extract_format_specifiers(s: str):
    # Matches %1$d, %1$s, %2$.1f, %d, %s, etc.
    return sorted(re.findall(r"%(\d+\$)?[0-9]*\.?[0-9]*[a-zA-Z%]", s))

def verify_localization_parity():
    repo_root = Path(__file__).resolve().parent.parent
    en_path = repo_root / "android" / "app" / "src" / "main" / "res" / "values" / "strings.xml"
    it_path = repo_root / "android" / "app" / "src" / "main" / "res" / "values-it" / "strings.xml"

    print("=================================================================")
    print(" Adversarial Android Localization Parity Stress Test            ")
    print("=================================================================")
    print(f"Default (EN) path: {en_path}")
    print(f"Italian (IT) path: {it_path}")

    en_keys, en_dict, en_dups = parse_strings_xml(en_path)
    it_keys, it_dict, it_dups = parse_strings_xml(it_path)

    print(f"\n[+] Total default (EN) strings: {len(en_keys)}")
    print(f"[+] Total Italian (IT) strings: {len(it_keys)}")

    all_passed = True

    # 1. Duplicates Check
    if en_dups:
        print(f"[FAIL] Duplicate keys in EN strings: {en_dups}")
        all_passed = False
    else:
        print("[OK] No duplicate keys found in EN strings.")

    if it_dups:
        print(f"[FAIL] Duplicate keys in IT strings: {it_dups}")
        all_passed = False
    else:
        print("[OK] No duplicate keys found in IT strings.")

    # 2. Key Parity Check
    en_key_set = set(en_dict.keys())
    it_key_set = set(it_dict.keys())

    missing_in_it = en_key_set - it_key_set
    missing_in_en = it_key_set - en_key_set

    if missing_in_it:
        print(f"\n[FAIL] Keys present in EN but MISSING in IT ({len(missing_in_it)}):")
        for k in sorted(missing_in_it):
            print(f"  - {k}")
        all_passed = False
    else:
        print("[OK] Zero keys missing in Italian (100% EN -> IT coverage).")

    if missing_in_en:
        print(f"\n[FAIL] Keys present in IT but MISSING in EN ({len(missing_in_en)}):")
        for k in sorted(missing_in_en):
            print(f"  - {k}")
        all_passed = False
    else:
        print("[OK] Zero keys missing in English (100% IT -> EN coverage).")

    # 3. Empty or Whitespace-Only Check
    empty_en = [k for k, v in en_dict.items() if not v or v.strip() == ""]
    empty_it = [k for k, v in it_dict.items() if not v or v.strip() == ""]

    if empty_en:
        print(f"\n[FAIL] Empty or whitespace-only keys in EN ({len(empty_en)}):")
        for k in sorted(empty_en):
            print(f"  - {k}: '{en_dict[k]}'")
        all_passed = False
    else:
        print("[OK] All EN string values are non-empty and non-whitespace.")

    if empty_it:
        print(f"\n[FAIL] Empty or whitespace-only keys in IT ({len(empty_it)}):")
        for k in sorted(empty_it):
            print(f"  - {k}: '{it_dict[k]}'")
        all_passed = False
    else:
        print("[OK] All IT string values are non-empty and non-whitespace.")

    # 4. Format Specifier Consistency
    common_keys = en_key_set.intersection(it_key_set)
    specifier_mismatches = []
    for k in sorted(common_keys):
        en_spec = extract_format_specifiers(en_dict[k])
        it_spec = extract_format_specifiers(it_dict[k])
        if en_spec != it_spec:
            specifier_mismatches.append((k, en_spec, it_spec))

    if specifier_mismatches:
        print(f"\n[FAIL] Format specifier mismatches ({len(specifier_mismatches)}):")
        for k, en_s, it_s in specifier_mismatches:
            print(f"  - {k}: EN={en_s} vs IT={it_s}")
        all_passed = False
    else:
        print("[OK] Format specifiers (%1$d, %1$s, etc.) match exactly across EN and IT.")

    print("\n=================================================================")
    if all_passed:
        print("[SUCCESS] 100% ANDROID LOCALIZATION PARITY VERIFIED (0 ERRORS)")
    else:
        print("[FAILURE] ANDROID LOCALIZATION PARITY CHECKS FAILED")
    print("=================================================================")
    return all_passed

if __name__ == "__main__":
    passed = verify_localization_parity()
    sys.exit(0 if passed else 1)
