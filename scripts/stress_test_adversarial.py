#!/usr/bin/env python3
"""
scripts/stress_test_adversarial.py
Adversarial Stress Harness for Android Strings AAPT2 Pitfalls & APK Signing Verification.
"""

import re
import sys
import struct
from pathlib import Path

def test_aapt2_string_pitfalls(xml_path: Path):
    print(f"\n[+] Testing AAPT2 pitfalls on: {xml_path.name}")
    raw_content = xml_path.read_text(encoding="utf-8")
    lines = raw_content.splitlines()

    issues = []
    for line_idx, line in enumerate(lines, 1):
        m = re.search(r'<string\s+name="([^"]+)">([^<]+)</string>', line)
        if m:
            key = m.group(1)
            val = m.group(2)

            # Check unescaped apostrophes
            temp = val.replace(r"\'", "")
            if "'" in temp:
                if not (val.startswith('"') and val.endswith('"')):
                    issues.append(f"Line {line_idx} ({key}): Unescaped apostrophe in '{val}'")

            # Check unescaped @ or ?
            if val.startswith("@") or val.startswith("?"):
                if not val.startswith(r"\@") and not val.startswith(r"\?"):
                    issues.append(f"Line {line_idx} ({key}): Starts with unescaped resource reference character: '{val}'")

    if issues:
        print(f"  [FAIL] {len(issues)} AAPT2 syntax issues found!")
        for issue in issues:
            print(f"    - {issue}")
        return False
    else:
        print(f"  [OK] No AAPT2 apostrophe/escaping pitfalls detected in {xml_path.name}.")
        return True

def verify_apk_signing(apk_path: Path):
    print(f"\n[+] Verifying APK signature structure: {apk_path.name}")
    if not apk_path.exists():
        print(f"  [FAIL] APK does not exist at {apk_path}")
        return False

    with open(apk_path, "rb") as f:
        data = f.read()

    # Search for APK Signing Block magic
    magic = b"APK Sig Block 42"
    magic_offset = data.find(magic)
    if magic_offset == -1:
        print("  [FAIL] APK Signing Block magic 'APK Sig Block 42' NOT found!")
        return False

    print(f"  [OK] APK Signing Block magic found at byte offset {magic_offset}")

    # Read block size before magic
    # In format: [uint64 size] ... [uint64 size] [16 bytes magic]
    block_end = magic_offset + len(magic)
    size_before_magic = struct.unpack("<Q", data[magic_offset - 8 : magic_offset])[0]
    block_start = block_end - 8 - size_before_magic

    print(f"  [+] APK Signing Block start: {block_start}, size: {size_before_magic} bytes")
    
    # Parse ID-value pairs in signing block
    pos = block_start + 8
    found_v2 = False
    found_v3 = False
    scheme_ids = []

    while pos < magic_offset - 8:
        pair_len = struct.unpack("<Q", data[pos : pos + 8])[0]
        pos += 8
        pair_id = struct.unpack("<I", data[pos : pos + 4])[0]
        scheme_ids.append(hex(pair_id))
        if pair_id == 0x7109871a:
            found_v2 = True
        elif pair_id == 0xf05368c0:
            found_v3 = True
        pos += pair_len

    print(f"  [+] Signature Scheme IDs detected: {scheme_ids}")
    if found_v2:
        print("  [OK] Android APK Signature Scheme v2 confirmed present (ID: 0x7109871a)!")
    if found_v3:
        print("  [OK] Android APK Signature Scheme v3 confirmed present (ID: 0xf05368c0)!")

    return found_v2 or found_v3

def main():
    repo_root = Path(__file__).resolve().parent.parent
    en_xml = repo_root / "android" / "app" / "src" / "main" / "res" / "values" / "strings.xml"
    it_xml = repo_root / "android" / "app" / "src" / "main" / "res" / "values-it" / "strings.xml"
    apk_file = repo_root / "android" / "app" / "build" / "outputs" / "apk" / "debug" / "app-debug.apk"

    p1 = test_aapt2_string_pitfalls(en_xml)
    p2 = test_aapt2_string_pitfalls(it_xml)
    p3 = verify_apk_signing(apk_file)

    all_passed = p1 and p2 and p3
    print(f"\n[OVERALL ADVERSARIAL STRESS RESULT]: {'PASSED' if all_passed else 'FAILED'}")
    return all_passed

if __name__ == "__main__":
    passed = main()
    sys.exit(0 if passed else 1)
