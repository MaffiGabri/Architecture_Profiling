#!/usr/bin/env python3
"""
scripts/validate_metadata.py

Architecture Profiling Metadata & Asset Validation Script
Validates shared/data/styles.json against shared/data/styles.schema.json
and validates all referenced 800x600 PNG images in shared/images/.

Features:
- Dual-mode JSON Schema Draft-07 validation:
    * Uses official `jsonschema` library if installed
    * Falls back to a robust built-in pure-Python Draft-07 schema validator if not installed
- Strict semantic and referential integrity checks (ID uniqueness 1..10, category foreign keys, category balance)
- Comprehensive image validation:
    * File existence in shared/images/
    * Valid PNG magic bytes (\\x89PNG\\r\\n\\x1a\\n)
    * File size > 10 KB (10,240 bytes)
    * Pillow image decoding with full stream decompression (img.load())
    * Exactly 800x600 px dimensions
    * Color mode strictly RGB or RGBA
    * Fallback pure-Python IHDR binary chunk parser
- Clean, informative CLI reporting with color-coded status badges and summary table
- Deterministic exit codes: 0 on success, non-zero on failure
"""

import argparse
import json
import os
import re
import struct
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Dict, List, Optional, Set, Tuple

# Try importing Pillow
try:
    from PIL import Image
    HAVE_PILLOW = True
except ImportError:
    HAVE_PILLOW = False

# Try importing jsonschema
try:
    import jsonschema
    HAVE_JSONSCHEMA = True
except ImportError:
    HAVE_JSONSCHEMA = False

# Ensure Windows terminal doesn't crash on Unicode characters
if hasattr(sys.stdout, "reconfigure"):
    try:
        sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    except Exception:
        pass


class Colors:
    """Terminal colors with Windows and non-TTY fallback."""
    RESET = "\033[0m"
    BOLD = "\033[1m"
    GREEN = "\033[32m"
    RED = "\033[31m"
    YELLOW = "\033[33m"
    CYAN = "\033[36m"
    BLUE = "\033[34m"
    MAGENTA = "\033[35m"

    @classmethod
    def disable(cls):
        cls.RESET = ""
        cls.BOLD = ""
        cls.GREEN = ""
        cls.RED = ""
        cls.YELLOW = ""
        cls.CYAN = ""
        cls.BLUE = ""
        cls.MAGENTA = ""


# Symbols that work on all platforms
SYM_PASS = "[PASS]"
SYM_FAIL = "[FAIL]"
SYM_WARN = "[WARN]"
SYM_INFO = "[INFO]"


# Auto-configure colors for Windows cmd / non-TTY
if not sys.stdout.isatty() or (sys.platform == "win32" and "WT_SESSION" not in os.environ and "TERM" not in os.environ):
    try:
        # Enable VT100 on Windows 10+
        import ctypes
        kernel32 = ctypes.windll.kernel32
        kernel32.SetConsoleMode(kernel32.GetStdHandle(-11), 7)
    except Exception:
        # If VT100 cannot be enabled, strip colors
        if sys.platform == "win32" and "ANSICON" not in os.environ:
            Colors.disable()


@dataclass
class ValidationIssue:
    severity: str  # "ERROR", "WARNING", "INFO"
    category: str  # "SCHEMA", "INTEGRITY", "IMAGE", "FILESYSTEM"
    location: str
    message: str


class PurePythonDraft7Validator:
    """
    Pure-Python validator implementing the Draft-07 schema subset
    required to validate styles.json without any third-party dependencies.
    """

    def __init__(self, schema: Dict[str, Any]):
        self.schema = schema
        self.definitions = schema.get("definitions", {})

    def resolve_ref(self, ref: str) -> Dict[str, Any]:
        """Resolves internal #/definitions/<name> references."""
        if ref.startswith("#/definitions/"):
            def_name = ref.split("/")[-1]
            if def_name in self.definitions:
                return self.definitions[def_name]
        raise ValueError(f"Unsupported or unresolved schema $ref: {ref}")

    def validate(self, instance: Any, sub_schema: Optional[Dict[str, Any]] = None, path: str = "root") -> List[ValidationIssue]:
        issues: List[ValidationIssue] = []
        if sub_schema is None:
            sub_schema = self.schema

        # Handle $ref
        if "$ref" in sub_schema:
            sub_schema = self.resolve_ref(sub_schema["$ref"])

        # Check type
        expected_type = sub_schema.get("type")
        if expected_type:
            if not self._check_type(instance, expected_type):
                actual_type = type(instance).__name__
                issues.append(ValidationIssue(
                    severity="ERROR",
                    category="SCHEMA",
                    location=path,
                    message=f"Expected type '{expected_type}', got '{actual_type}'"
                ))
                return issues  # Cannot validate further if type is wrong

        # Object validation
        if isinstance(instance, dict):
            # Required fields
            required_keys = sub_schema.get("required", [])
            for req in required_keys:
                if req not in instance:
                    issues.append(ValidationIssue(
                        severity="ERROR",
                        category="SCHEMA",
                        location=f"{path}.{req}",
                        message=f"Missing required property '{req}'"
                    ))

            # Additional properties check
            additional_props = sub_schema.get("additionalProperties", True)
            declared_props = sub_schema.get("properties", {})
            if additional_props is False:
                for k in instance:
                    # Ignore $schema if present at root
                    if path == "root" and k == "$schema":
                        continue
                    if k not in declared_props:
                        issues.append(ValidationIssue(
                            severity="ERROR",
                            category="SCHEMA",
                            location=f"{path}.{k}",
                            message=f"Unexpected property '{k}' (additionalProperties: false)"
                        ))

            # Validate child properties
            for prop_name, prop_schema in declared_props.items():
                if prop_name in instance:
                    issues.extend(self.validate(
                        instance=instance[prop_name],
                        sub_schema=prop_schema,
                        path=f"{path}.{prop_name}"
                    ))

        # Array validation
        elif isinstance(instance, list):
            min_items = sub_schema.get("minItems")
            if min_items is not None and len(instance) < min_items:
                issues.append(ValidationIssue(
                    severity="ERROR",
                    category="SCHEMA",
                    location=path,
                    message=f"Array length {len(instance)} is less than minItems {min_items}"
                ))

            max_items = sub_schema.get("maxItems")
            if max_items is not None and len(instance) > max_items:
                issues.append(ValidationIssue(
                    severity="ERROR",
                    category="SCHEMA",
                    location=path,
                    message=f"Array length {len(instance)} is greater than maxItems {max_items}"
                ))

            item_schema = sub_schema.get("items")
            if item_schema:
                for idx, item in enumerate(instance):
                    issues.extend(self.validate(
                        instance=item,
                        sub_schema=item_schema,
                        path=f"{path}[{idx}]"
                    ))

        # String validation
        elif isinstance(instance, str):
            min_len = sub_schema.get("minLength")
            if min_len is not None and len(instance) < min_len:
                issues.append(ValidationIssue(
                    severity="ERROR",
                    category="SCHEMA",
                    location=path,
                    message=f"String length {len(instance)} is less than minLength {min_len}"
                ))

            pattern = sub_schema.get("pattern")
            if pattern:
                if not re.search(pattern, instance):
                    issues.append(ValidationIssue(
                        severity="ERROR",
                        category="SCHEMA",
                        location=path,
                        message=f"String '{instance}' does not match regex pattern '{pattern}'"
                    ))

        # Number / Integer validation
        elif isinstance(instance, (int, float)) and not isinstance(instance, bool):
            minimum = sub_schema.get("minimum")
            if minimum is not None and instance < minimum:
                issues.append(ValidationIssue(
                    severity="ERROR",
                    category="SCHEMA",
                    location=path,
                    message=f"Value {instance} is less than minimum {minimum}"
                ))

            maximum = sub_schema.get("maximum")
            if maximum is not None and instance > maximum:
                issues.append(ValidationIssue(
                    severity="ERROR",
                    category="SCHEMA",
                    location=path,
                    message=f"Value {instance} is greater than maximum {maximum}"
                ))

        return issues

    def _check_type(self, value: Any, expected: str) -> bool:
        if expected == "object":
            return isinstance(value, dict)
        if expected == "array":
            return isinstance(value, list)
        if expected == "string":
            return isinstance(value, str)
        if expected == "integer":
            return isinstance(value, int) and not isinstance(value, bool)
        if expected == "number":
            return isinstance(value, (int, float)) and not isinstance(value, bool)
        if expected == "boolean":
            return isinstance(value, bool)
        if expected == "null":
            return value is None
        return True


class MetadataValidator:
    """Orchestrates schema, data integrity, and asset checks."""

    def __init__(
        self,
        styles_json_path: Path,
        schema_json_path: Path,
        images_dir: Path,
        strict: bool = True,
        verbose: bool = False
    ):
        self.styles_json_path = styles_json_path.resolve()
        self.schema_json_path = schema_json_path.resolve()
        self.images_dir = images_dir.resolve()
        self.strict = strict
        self.verbose = verbose
        self.issues: List[ValidationIssue] = []

    def run_all_checks(self) -> bool:
        """Runs the complete test suite. Returns True if all checks pass."""
        print(f"{Colors.BOLD}{Colors.CYAN}================================================================{Colors.RESET}")
        print(f"{Colors.BOLD}{Colors.CYAN}    Architecture Profiling — Metadata & Asset Validator       {Colors.RESET}")
        print(f"{Colors.BOLD}{Colors.CYAN}================================================================{Colors.RESET}\\n")

        # 1. File existence
        if not self._check_file_existence():
            self._print_summary()
            return False

        # 2. Parse JSON
        schema_obj = self._parse_json(self.schema_json_path, "styles.schema.json")
        data_obj = self._parse_json(self.styles_json_path, "styles.json")

        if schema_obj is None or data_obj is None:
            self._print_summary()
            return False

        # 3. Schema validation
        self._validate_schema(data_obj, schema_obj)

        # 4. Semantic referential integrity
        self._validate_integrity(data_obj)

        # 5. Image assets validation
        self._validate_images(data_obj)

        # Print final report
        self._print_summary()

        has_errors = any(issue.severity == "ERROR" for issue in self.issues)
        has_warnings = any(issue.severity == "WARNING" for issue in self.issues)
        if has_errors:
            return False
        if self.strict and has_warnings:
            return False
        return True

    def _check_file_existence(self) -> bool:
        print(f"{Colors.BOLD}[1/4] Checking Essential File Existence...{Colors.RESET}")
        all_found = True

        files_to_check = [
            ("styles.schema.json", self.schema_json_path, "file"),
            ("styles.json", self.styles_json_path, "file"),
            ("images directory", self.images_dir, "dir"),
        ]

        for name, path, kind in files_to_check:
            exists = path.is_file() if kind == "file" else path.is_dir()
            if exists:
                print(f"  {Colors.GREEN}{SYM_PASS} FOUND{Colors.RESET} {name}: {path}")
            else:
                print(f"  {Colors.RED}{SYM_FAIL} MISSING{Colors.RESET} {name}: {path}")
                self.issues.append(ValidationIssue(
                    severity="ERROR",
                    category="FILESYSTEM",
                    location=str(path),
                    message=f"Required {kind} does not exist: {path}"
                ))
                all_found = False

        print()
        return all_found

    def _parse_json(self, path: Path, label: str) -> Optional[Dict[str, Any]]:
        try:
            with open(path, "r", encoding="utf-8") as f:
                return json.load(f)
        except json.JSONDecodeError as err:
            self.issues.append(ValidationIssue(
                severity="ERROR",
                category="SCHEMA",
                location=f"{path}:{err.lineno}:{err.colno}",
                message=f"JSON syntax error in {label}: {err.msg}"
            ))
            return None
        except Exception as err:
            self.issues.append(ValidationIssue(
                severity="ERROR",
                category="FILESYSTEM",
                location=str(path),
                message=f"Failed to read {label}: {err}"
            ))
            return None

    def _validate_schema(self, data: Dict[str, Any], schema: Dict[str, Any]):
        print(f"{Colors.BOLD}[2/4] Validating JSON Schema Structure...{Colors.RESET}")
        
        if HAVE_JSONSCHEMA:
            print(f"  {Colors.BLUE}{SYM_INFO} Using 'jsonschema' library (Draft-07 validator){Colors.RESET}")
            try:
                validator_cls = jsonschema.Draft7Validator
                validator_cls.check_schema(schema)
                validator = validator_cls(schema)
                
                schema_errors = list(validator.iter_errors(data))
                if not schema_errors:
                    print(f"  {Colors.GREEN}{SYM_PASS} styles.json perfectly conforms to Draft-07 schema{Colors.RESET}")
                else:
                    for err in schema_errors:
                        json_path = "root." + ".".join(str(p) for p in err.path) if err.path else "root"
                        self.issues.append(ValidationIssue(
                            severity="ERROR",
                            category="SCHEMA",
                            location=json_path,
                            message=err.message
                        ))
            except Exception as e:
                self.issues.append(ValidationIssue(
                    severity="ERROR",
                    category="SCHEMA",
                    location=str(self.schema_json_path),
                    message=f"jsonschema engine exception: {e}"
                ))
        else:
            print(f"  {Colors.YELLOW}{SYM_INFO} 'jsonschema' not installed; using built-in Pure-Python Draft-07 validator{Colors.RESET}")
            validator = PurePythonDraft7Validator(schema)
            issues = validator.validate(data)
            self.issues.extend(issues)
            if not issues:
                print(f"  {Colors.GREEN}{SYM_PASS} styles.json passed built-in Draft-07 schema validation{Colors.RESET}")

        print()

    def _validate_integrity(self, data: Dict[str, Any]):
        print(f"{Colors.BOLD}[3/4] Validating Domain & Referential Integrity...{Colors.RESET}")

        categories = data.get("categories", [])
        styles = data.get("styles") or data.get("images") or []

        # Check Category count
        if len(categories) != 5:
            self.issues.append(ValidationIssue(
                severity="ERROR",
                category="INTEGRITY",
                location="categories",
                message=f"Expected exactly 5 categories for balanced tournament, found {len(categories)}"
            ))

        cat_ids: Set[str] = set()
        for idx, cat in enumerate(categories):
            cid = cat.get("id")
            if cid:
                if cid in cat_ids:
                    self.issues.append(ValidationIssue(
                        severity="ERROR",
                        category="INTEGRITY",
                        location=f"categories[{idx}].id",
                        message=f"Duplicate category ID '{cid}'"
                    ))
                cat_ids.add(cid)

        # Check Style count
        if len(styles) != 10:
            self.issues.append(ValidationIssue(
                severity="ERROR",
                category="INTEGRITY",
                location="styles",
                message=f"Expected exactly 10 architectural styles, found {len(styles)}"
            ))

        style_ids: Set[int] = set()
        filenames: Set[str] = set()
        category_distribution: Dict[str, int] = {cid: 0 for cid in cat_ids}

        for idx, s in enumerate(styles):
            if not isinstance(s, dict):
                self.issues.append(ValidationIssue(
                    severity="ERROR",
                    category="INTEGRITY",
                    location=f"styles[{idx}]",
                    message=f"Style entry must be a dict/object, got {type(s).__name__}"
                ))
                continue
            sid = s.get("id")
            fn = s.get("filename")
            cid = s.get("category_id")

            # Check ID sequence 1..10
            if sid is not None:
                if isinstance(sid, bool) or not isinstance(sid, int):
                    self.issues.append(ValidationIssue(
                        severity="ERROR",
                        category="INTEGRITY",
                        location=f"styles[{idx}].id",
                        message=f"Style ID must be an integer, got {type(sid).__name__}: {sid!r}"
                    ))
                elif not (1 <= sid <= 10):
                    self.issues.append(ValidationIssue(
                        severity="ERROR",
                        category="INTEGRITY",
                        location=f"styles[{idx}].id",
                        message=f"Style ID {sid} out of bounds [1, 10]"
                    ))
                else:
                    if sid in style_ids:
                        self.issues.append(ValidationIssue(
                            severity="ERROR",
                            category="INTEGRITY",
                            location=f"styles[{idx}].id",
                            message=f"Duplicate style ID {sid}"
                        ))
                    style_ids.add(sid)

            # Check filename uniqueness
            if fn:
                if fn in filenames:
                    self.issues.append(ValidationIssue(
                        severity="ERROR",
                        category="INTEGRITY",
                        location=f"styles[{idx}].filename",
                        message=f"Duplicate filename '{fn}'"
                    ))
                filenames.add(fn)

            # Check category foreign key reference
            if cid:
                if cid not in cat_ids:
                    self.issues.append(ValidationIssue(
                        severity="ERROR",
                        category="INTEGRITY",
                        location=f"styles[{idx}].category_id",
                        message=f"Style references non-existent category_id '{cid}'"
                    ))
                else:
                    category_distribution[cid] += 1

            # Validate 5D traits range
            traits = s.get("traits")
            if not isinstance(traits, dict):
                if traits is not None:
                    self.issues.append(ValidationIssue(
                        severity="ERROR",
                        category="INTEGRITY",
                        location=f"styles[{idx}].traits",
                        message=f"Traits property must be a dict/object, got {type(traits).__name__}"
                    ))
                traits = {}
            expected_traits = ["era", "ornamentation", "structural_honesty", "geometric_order", "material_warmth"]
            for trait_name in expected_traits:
                val = traits.get(trait_name)
                if val is not None:
                    if isinstance(val, bool) or not isinstance(val, (int, float)):
                        self.issues.append(ValidationIssue(
                            severity="ERROR",
                            category="INTEGRITY",
                            location=f"styles[{idx}].traits.{trait_name}",
                            message=f"Trait '{trait_name}' must be numeric (int/float), got {type(val).__name__}: {val!r}"
                        ))
                    else:
                        try:
                            num_val = float(val)
                            if not (0.0 <= num_val <= 10.0):
                                self.issues.append(ValidationIssue(
                                    severity="ERROR",
                                    category="INTEGRITY",
                                    location=f"styles[{idx}].traits.{trait_name}",
                                    message=f"Trait '{trait_name}' value {val} is outside [0.0, 10.0]"
                                ))
                        except (TypeError, ValueError) as err:
                            self.issues.append(ValidationIssue(
                                severity="ERROR",
                                category="INTEGRITY",
                                location=f"styles[{idx}].traits.{trait_name}",
                                message=f"Trait '{trait_name}' invalid numeric value: {err}"
                            ))

        # Check category distribution balance
        for cid, count in category_distribution.items():
            if count != 2:
                self.issues.append(ValidationIssue(
                    severity="WARNING" if not self.strict else "ERROR",
                    category="INTEGRITY",
                    location=f"category_balance.{cid}",
                    message=f"Category '{cid}' has {count} styles (expected exactly 2 for balanced tournament)"
                ))

        if not any(i.category == "INTEGRITY" and i.severity == "ERROR" for i in self.issues):
            print(f"  {Colors.GREEN}{SYM_PASS} 10 unique style IDs (1..10), 5 categories (2 items each), referential integrity confirmed{Colors.RESET}")
        print()

    def _validate_images(self, data: Dict[str, Any]):
        print(f"{Colors.BOLD}[4/4] Validating Referenced Image Files (800x600 PNG)...{Colors.RESET}")
        styles = data.get("styles") or data.get("images") or []

        if not styles:
            self.issues.append(ValidationIssue(
                severity="ERROR",
                category="IMAGE",
                location="styles",
                message="No styles/images list found to validate"
            ))
            return

        validated_count = 0
        for idx, s in enumerate(styles):
            if not isinstance(s, dict):
                continue
            sid = s.get("id")
            fn = s.get("filename")
            style_name = s.get("style_en", f"Style #{sid}")
            sid_disp = f"{sid:02d}" if isinstance(sid, int) and not isinstance(sid, bool) else str(sid)

            if not fn:
                self.issues.append(ValidationIssue(
                    severity="ERROR",
                    category="IMAGE",
                    location=f"styles[{sid}]",
                    message="Missing 'filename' property in style metadata"
                ))
                continue

            img_path = self.images_dir / fn
            img_errors = self._inspect_image_file(img_path, fn)

            if not img_errors:
                size_kb = img_path.stat().st_size / 1024.0
                print(f"  {Colors.GREEN}{SYM_PASS} [{sid_disp:>2}] {fn:<28} | 800x600 | {size_kb:6.1f} KB | {style_name}{Colors.RESET}")
                validated_count += 1
            else:
                print(f"  {Colors.RED}{SYM_FAIL} [{sid_disp:>2}] {fn:<28} | {style_name}{Colors.RESET}")
                for err in img_errors:
                    print(f"         {Colors.RED}--> {err}{Colors.RESET}")
                    self.issues.append(ValidationIssue(
                        severity="ERROR",
                        category="IMAGE",
                        location=fn,
                        message=err
                    ))

        # Check for unreferenced / orphan files in shared/images/
        if self.images_dir.is_dir():
            disk_pngs = set(p.name for p in self.images_dir.glob("*.png"))
            referenced_pngs = set(s.get("filename") for s in styles if isinstance(s, dict) and s.get("filename"))
            orphans = disk_pngs - referenced_pngs
            if orphans:
                for orphan in orphans:
                    self.issues.append(ValidationIssue(
                        severity="WARNING",
                        category="IMAGE",
                        location=orphan,
                        message=f"Orphaned PNG found in shared/images/ not referenced in JSON: {orphan}"
                    ))

        print()

    def _inspect_image_file(self, path: Path, filename: str) -> List[str]:
        errors: List[str] = []

        # 1. Existence
        if not path.is_file():
            return [f"File does not exist on disk: {path}"]

        # 2. File size threshold (> 10 KB = 10,240 bytes)
        MIN_BYTES = 10 * 1024
        file_size = path.stat().st_size
        if file_size <= MIN_BYTES:
            errors.append(f"File size {file_size} bytes ({file_size / 1024:.1f} KB) is <= 10 KB minimum threshold")

        # 3. Binary PNG magic bytes check
        try:
            with open(path, "rb") as f:
                header = f.read(8)
            if header != b"\x89PNG\r\n\x1a\n":
                errors.append(f"Invalid PNG magic bytes: {header!r} (expected b'\\x89PNG\\r\\n\\x1a\\n')")
                return errors
        except Exception as e:
            errors.append(f"Failed to read image binary header: {e}")
            return errors

        # 4. Image inspection via Pillow or fallback parser
        if HAVE_PILLOW:
            try:
                with Image.open(path) as img:
                    # Verify format
                    if img.format != "PNG":
                        errors.append(f"Pillow format '{img.format}' != 'PNG'")

                    # Verify exact dimensions
                    if img.size != (800, 600):
                        errors.append(f"Dimensions {img.size[0]}x{img.size[1]} != required 800x600 px")

                    # Verify mode
                    if img.mode not in ("RGB", "RGBA"):
                        errors.append(f"Color mode '{img.mode}' is not RGB or RGBA")

                    # Force decompression of image stream to verify uncorrupted pixel data
                    img.load()
            except Exception as e:
                errors.append(f"Pillow decoding error: {e}")
        else:
            # Fallback pure-Python IHDR parser
            try:
                with open(path, "rb") as f:
                    data = f.read(30)
                if len(data) >= 26:
                    w, h, bit_depth, color_type = struct.unpack(">IIBB", data[16:26])
                    if (w, h) != (800, 600):
                        errors.append(f"PNG IHDR dimensions {w}x{h} != required 800x600 px")
                    # Color type: 2 = Truecolor (RGB), 6 = Truecolor with Alpha (RGBA)
                    if color_type not in (2, 6):
                        errors.append(f"PNG IHDR color type {color_type} is not Truecolor RGB(2) or RGBA(6)")
                else:
                    errors.append("Truncated PNG header (less than 26 bytes)")
            except Exception as e:
                errors.append(f"Binary header decode error: {e}")

        return errors

    def _print_summary(self):
        print(f"{Colors.BOLD}{Colors.CYAN}================================================================{Colors.RESET}")
        print(f"{Colors.BOLD}{Colors.CYAN}                       VALIDATION SUMMARY                       {Colors.RESET}")
        print(f"{Colors.BOLD}{Colors.CYAN}================================================================{Colors.RESET}")

        errors = [i for i in self.issues if i.severity == "ERROR"]
        warnings = [i for i in self.issues if i.severity == "WARNING"]

        if errors:
            print(f"\\n{Colors.BOLD}{Colors.RED}[FAIL] FAILED -- {len(errors)} ERROR(S) DETECTED:{Colors.RESET}")
            for idx, err in enumerate(errors, 1):
                print(f"  {idx:2d}. [{err.category}] {Colors.BOLD}{err.location}{Colors.RESET}: {err.message}")

        if warnings:
            print(f"\\n{Colors.BOLD}{Colors.YELLOW}[WARN] WARNINGS ({len(warnings)}):{Colors.RESET}")
            for idx, warn in enumerate(warnings, 1):
                print(f"  {idx:2d}. [{warn.category}] {warn.location}: {warn.message}")

        if not errors and not warnings:
            print(f"\\n{Colors.BOLD}{Colors.GREEN}[PASS] SUCCESS: 100% OF VALIDATION CHECKS PASSED CLEANLY!{Colors.RESET}")
            print(f"  * JSON Schema Draft-07: Conforms perfectly")
            print(f"  * Referential Integrity: 10 Styles, 5 Categories, Unique IDs & Filenames")
            print(f"  * Image Assets: All 10 PNGs present, 800x600 px, >10 KB, valid RGB/RGBA")
            print(f"{Colors.BOLD}{Colors.GREEN}Status: PASS (Exit Code 0){Colors.RESET}\\n")
        elif not errors and not self.strict:
            print(f"\n{Colors.BOLD}{Colors.GREEN}[PASS] SUCCESS (WITH WARNINGS): Core requirements passed.{Colors.RESET}")
            print(f"{Colors.BOLD}{Colors.GREEN}Status: PASS (Exit Code 0){Colors.RESET}\n")
        elif not errors and self.strict:
            print(f"\n{Colors.BOLD}{Colors.RED}[FAIL] FAILED: {len(warnings)} warning(s) treated as error(s) under strict mode.{Colors.RESET}")
            print(f"{Colors.BOLD}{Colors.RED}Status: FAIL (Exit Code 1){Colors.RESET}\n")
        else:
            print(f"\n{Colors.BOLD}{Colors.RED}Status: FAIL (Exit Code 1){Colors.RESET}\n")


def parse_args() -> argparse.Namespace:
    # Auto-detect project root
    script_dir = Path(__file__).resolve().parent
    repo_root = script_dir.parent if script_dir.name == "scripts" else script_dir

    parser = argparse.ArgumentParser(
        description="Architecture Profiling Metadata & Asset Validator"
    )
    parser.add_argument(
        "data_pos",
        nargs="?",
        type=Path,
        default=None,
        help="Optional positional path to styles.json"
    )
    parser.add_argument(
        "--root",
        type=Path,
        default=repo_root,
        help="Repository root directory (default: auto-detected)"
    )
    parser.add_argument(
        "--data",
        type=Path,
        default=None,
        help="Path to styles.json (default: <root>/shared/data/styles.json)"
    )
    parser.add_argument(
        "--schema",
        type=Path,
        default=None,
        help="Path to styles.schema.json (default: <root>/shared/data/styles.schema.json)"
    )
    parser.add_argument(
        "--images",
        type=Path,
        default=None,
        help="Directory containing images (default: <root>/shared/images)"
    )
    parser.add_argument(
        "--strict",
        action="store_true",
        default=True,
        help="Treat warnings as errors (default: True)"
    )
    parser.add_argument(
        "--no-strict",
        dest="strict",
        action="store_false",
        help="Allow warnings without failing"
    )
    parser.add_argument(
        "--verbose", "-v",
        action="store_true",
        help="Enable verbose output"
    )

    return parser.parse_args()


def main():
    args = parse_args()
    root = args.root

    data_path = args.data or args.data_pos or (root / "shared" / "data" / "styles.json")
    schema_path = args.schema or (root / "shared" / "data" / "styles.schema.json")
    images_dir = args.images or (root / "shared" / "images")

    validator = MetadataValidator(
        styles_json_path=data_path,
        schema_json_path=schema_path,
        images_dir=images_dir,
        strict=args.strict,
        verbose=args.verbose
    )

    success = validator.run_all_checks()
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
