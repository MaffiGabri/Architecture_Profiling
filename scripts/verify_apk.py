#!/usr/bin/env python3
"""
scripts/verify_apk.py
Adversarial Empirical Verification for Android APK.
Validates:
1. APK ZIP integrity and structure
2. Asset presence and JSON validity of styles.json (10 items, required keys, schema conformity)
3. Asset presence and Pillow decoding of all 10 mock PNG images (resolution 800x600, mode, byte size)
4. Presence in DEX files of domain classes, models, and Compose UI components
5. AndroidManifest.xml binary XML structure, application element, MainActivity launcher intent-filter
"""

import sys
import os
import zipfile
import json
import io
import struct
import re
from PIL import Image

def verify_apk(apk_path: str) -> dict:
    if not os.path.exists(apk_path):
        raise FileNotFoundError(f"APK not found at: {apk_path}")

    apk_size = os.path.getsize(apk_path)
    print(f"[+] Inspecting APK: {apk_path} ({apk_size} bytes)")

    results = {
        "apk_path": apk_path,
        "apk_size_bytes": apk_size,
        "asset_checks": {},
        "image_checks": {},
        "dex_checks": {},
        "manifest_checks": {},
        "overall_pass": False
    }

    with zipfile.ZipFile(apk_path, "r") as z:
        names = z.namelist()
        print(f"[+] Total entries in APK archive: {len(names)}")

        # 1. Styles JSON Check
        assert "assets/styles.json" in names, "assets/styles.json missing from APK"
        styles_raw = z.read("assets/styles.json").decode("utf-8")
        styles_data = json.loads(styles_raw)
        styles_list = styles_data.get("styles", styles_data.get("images", []))
        assert len(styles_list) == 10, f"Expected 10 styles, got {len(styles_list)}"

        for s in styles_list:
            assert "id" in s, f"Style missing id: {s}"
            assert "filename" in s, f"Style missing filename: {s}"
            assert "title" in s, f"Style missing title: {s}"
            assert "category_id" in s, f"Style missing category_id: {s}"
            assert "traits" in s, f"Style missing traits: {s}"
            traits = s["traits"]
            for trait in ["era", "ornamentation", "structural_honesty", "geometric_order", "material_warmth"]:
                assert trait in traits, f"Missing trait {trait} in {s['filename']}"
                val = float(traits[trait])
                assert 0.0 <= val <= 10.0, f"Trait {trait} value {val} out of bounds [0, 10]"

        results["asset_checks"]["styles_json"] = {
            "entry": "assets/styles.json",
            "items_count": len(styles_list),
            "valid": True
        }
        print(f"[OK] styles.json validated successfully ({len(styles_list)} styles, all traits in [0.0, 10.0])")

        # 2. Mock Images Check
        expected_images = [
            "arch_01_classical.png",
            "arch_02_gothic.png",
            "arch_03_renaissance.png",
            "arch_04_baroque.png",
            "arch_05_art_deco.png",
            "arch_06_bauhaus.png",
            "arch_07_brutalism.png",
            "arch_08_hightech.png",
            "arch_09_deconstructivism.png",
            "arch_10_parametric.png"
        ]

        for img_name in expected_images:
            asset_path = f"assets/{img_name}"
            assert asset_path in names, f"{asset_path} not found in APK"
            img_bytes = z.read(asset_path)
            img = Image.open(io.BytesIO(img_bytes))
            assert img.format == "PNG", f"{img_name} format is {img.format}, expected PNG"
            assert img.size == (800, 600), f"{img_name} size is {img.size}, expected (800, 600)"
            assert img.mode in ("RGB", "RGBA"), f"{img_name} mode is {img.mode}"
            results["image_checks"][img_name] = {
                "format": img.format,
                "size": list(img.size),
                "mode": img.mode,
                "bytes": len(img_bytes),
                "valid": True
            }
            print(f"[OK] Image verified: {img_name} ({img.format}, {img.size[0]}x{img.size[1]}, {len(img_bytes)} bytes)")

        # 3. DEX Classes Check
        dex_files = [n for n in names if n.endswith(".dex")]
        print(f"[+] DEX archives detected: {len(dex_files)} ({', '.join(dex_files)})")
        combined_dex = b"".join([z.read(d) for d in dex_files])

        required_symbols = [
            b"Lcom/architecture/profiling/MainActivity;",
            b"Lcom/architecture/profiling/domain/engine/ArchetypeEngine;",
            b"Lcom/architecture/profiling/domain/engine/ConfidenceEngine;",
            b"Lcom/architecture/profiling/domain/engine/PairingScheduler;",
            b"Lcom/architecture/profiling/domain/engine/PreferenceScorer;",
            b"Lcom/architecture/profiling/domain/engine/TournamentStateMachine;",
            b"Lcom/architecture/profiling/domain/engine/TransitivityCalculator;",
            b"Lcom/architecture/profiling/domain/model/Style;",
            b"Lcom/architecture/profiling/domain/model/Category;",
            b"Lcom/architecture/profiling/domain/model/TraitRadar;",
            b"Lcom/architecture/profiling/data/repository/AssetStyleRepository;",
            b"Lcom/architecture/profiling/data/image/AssetImageLoader;",
            b"Lcom/architecture/profiling/ui/components/RadarChartKt;",
            b"Lcom/architecture/profiling/ui/components/StyleCardKt;",
            b"Lcom/architecture/profiling/ui/viewmodel/TournamentViewModel;",
            b"Lcom/architecture/profiling/ui/screens/WelcomeScreenKt;",
            b"Lcom/architecture/profiling/ui/screens/TournamentScreenKt;",
            b"Lcom/architecture/profiling/ui/screens/ResultsScreenKt;",
            b"Lcom/architecture/profiling/ui/screens/HistoryScreenKt;"
        ]

        dex_symbol_results = {}
        for sym in required_symbols:
            present = sym in combined_dex
            sym_name = sym.decode("utf-8")
            dex_symbol_results[sym_name] = present
            assert present, f"Required DEX symbol missing: {sym_name}"
            print(f"[OK] DEX symbol verified: {sym_name}")

        app_classes = sorted(set(re.findall(rb"Lcom/architecture/profiling/[a-zA-Z0-9_$/]+;", combined_dex)))
        results["dex_checks"] = {
            "dex_files_count": len(dex_files),
            "total_app_classes": len(app_classes),
            "required_symbols": dex_symbol_results
        }
        print(f"[+] Total com.architecture.profiling classes in DEX: {len(app_classes)}")

        # 4. AndroidManifest.xml Binary XML Check
        assert "AndroidManifest.xml" in names, "AndroidManifest.xml missing from APK"
        manifest_raw = z.read("AndroidManifest.xml")
        magic, total_size = struct.unpack("<II", manifest_raw[:8])
        assert magic == 0x00080003, f"Invalid AXML magic: {hex(magic)}"

        chunk_type, chunk_size = struct.unpack("<II", manifest_raw[8:16])
        assert chunk_type == 0x001c0001, "Invalid STRING_POOL chunk type"

        string_count, style_count, flags, strings_start, styles_start = struct.unpack("<IIIII", manifest_raw[16:36])
        is_utf8 = bool(flags & (1 << 8))
        offsets = [struct.unpack("<I", manifest_raw[36 + i * 4 : 40 + i * 4])[0] for i in range(string_count)]
        strings_base = 8 + strings_start

        manifest_strings = []
        for off in offsets:
            pos = strings_base + off
            if is_utf8:
                l1 = manifest_raw[pos]
                pos += 1
                if l1 & 0x80:
                    pos += 1
                l2 = manifest_raw[pos]
                pos += 1
                if l2 & 0x80:
                    pos += 1
                s = manifest_raw[pos : pos + l2].decode("utf-8", errors="replace")
            else:
                u16len = struct.unpack("<H", manifest_raw[pos : pos + 2])[0]
                pos += 2
                s = manifest_raw[pos : pos + u16len * 2].decode("utf-16le", errors="replace")
            manifest_strings.append(s)

        required_manifest_strings = [
            "com.architecture.profiling",
            "com.architecture.profiling.MainActivity",
            "android.intent.action.MAIN",
            "android.intent.category.LAUNCHER",
            "application",
            "activity"
        ]

        manifest_string_results = {}
        for req in required_manifest_strings:
            found = req in manifest_strings
            manifest_string_results[req] = found
            assert found, f"Required manifest string missing: {req}"
            print(f"[OK] Manifest declaration verified: {req}")

        results["manifest_checks"] = {
            "magic": hex(magic),
            "size_bytes": len(manifest_raw),
            "string_count": string_count,
            "verified_tokens": manifest_string_results
        }

    results["overall_pass"] = True
    print("\n[SUCCESS] ALL EMPIRICAL APK CHECKS PASSED WITH ZERO ERRORS!")
    return results

if __name__ == "__main__":
    default_apk = os.path.join("android", "app", "build", "outputs", "apk", "debug", "app-debug.apk")
    apk_file = sys.argv[1] if len(sys.argv) > 1 else default_apk
    res = verify_apk(apk_file)
    sys.exit(0 if res["overall_pass"] else 1)
