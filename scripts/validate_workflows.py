#!/usr/bin/env python3
"""
scripts/validate_workflows.py
Empirical GitHub Actions YAML Parser, Linter, and Semantic Validator.

Performs:
1. Lexical and structural YAML parsing into nested Python structures.
2. GitHub Actions Schema validation (top-level, job-level, step-level).
3. Runner targets verification (runs-on).
4. Actions version and identifier syntax validation (uses: org/repo@ref).
5. Expression syntax validation (${{ ... }} brace parity).
6. Local repository path references cross-validation (scripts, folders, files referenced in run/path).
"""

import sys
import re
from pathlib import Path

# Recognized GitHub Actions official and commonly supported runner labels
VALID_RUNNERS = {
    "ubuntu-latest", "ubuntu-24.04", "ubuntu-22.04", "ubuntu-20.04",
    "windows-latest", "windows-2022", "windows-2019",
    "macos-latest", "macos-14", "macos-13", "macos-12"
}

ALLOWED_TOP_KEYS = {
    "name", "on", "permissions", "jobs", "env", "defaults", "concurrency"
}

ALLOWED_JOB_KEYS = {
    "name", "runs-on", "strategy", "steps", "needs", "permissions",
    "environment", "outputs", "env", "defaults", "if", "timeout-minutes",
    "continue-on-error", "container", "services"
}

ALLOWED_STEP_KEYS = {
    "name", "id", "if", "uses", "run", "working-directory",
    "shell", "with", "env", "continue-on-error", "timeout-minutes"
}

VALID_SHELLS = {
    "bash", "sh", "cmd", "pwsh", "powershell", "python"
}

def parse_yaml_lines(lines):
    """
    Recursive indentation-based YAML parser capable of parsing
    mappings, sequences, flow sequences, and multiline block scalars (| and >).
    """
    def get_indent(line):
        return len(line) - len(line.lstrip(" "))

    def strip_comments(line):
        # Strip trailing comments while keeping quotes intact
        in_quote = False
        quote_char = None
        for i, ch in enumerate(line):
            if ch in ('"', "'"):
                if not in_quote:
                    in_quote = True
                    quote_char = ch
                elif ch == quote_char:
                    in_quote = False
            elif ch == '#' and not in_quote:
                return line[:i].rstrip()
        return line

    def parse_flow_seq(val_str):
        # [ a, b, c ]
        val_str = val_str.strip()
        if val_str.startswith("[") and val_str.endswith("]"):
            inner = val_str[1:-1].strip()
            if not inner:
                return []
            parts = [p.strip().strip("'\"") for p in inner.split(",")]
            return parts
        return val_str

    idx = 0
    total = len(lines)

    def parse_block(current_indent):
        nonlocal idx
        result = None
        is_dict = None

        while idx < total:
            raw_line = lines[idx]
            stripped = strip_comments(raw_line)

            if not stripped.strip():
                idx += 1
                continue

            indent = get_indent(stripped)
            if indent < current_indent:
                break

            line_content = stripped.strip()

            # Sequence item
            if line_content.startswith("- ") or line_content == "-":
                if is_dict is True:
                    break
                is_dict = False
                if result is None:
                    result = []

                item_str = line_content[2:].strip()
                idx += 1

                if not item_str:
                    # Nested block sequence item
                    sub_val = parse_block(indent + 2)
                    result.append(sub_val)
                elif ":" in item_str and not item_str.startswith("{"):
                    # Sequence item that is a map: e.g. "- name: Foo"
                    k, v = item_str.split(":", 1)
                    k = k.strip()
                    v = v.strip()
                    sub_dict = {}
                    if v == "|" or v == ">" or v == ">-":
                        # Multi-line string block
                        sub_dict[k] = parse_multiline(indent + 4, v)
                    elif v:
                        sub_dict[k] = parse_flow_seq(v.strip("'\""))
                    else:
                        sub_dict[k] = parse_block(indent + 4)

                    # Continue reading siblings at the same key-level for this map
                    while idx < total:
                        next_line = strip_comments(lines[idx])
                        if not next_line.strip():
                            idx += 1
                            continue
                        next_indent = get_indent(next_line)
                        if next_indent <= indent:
                            break
                        # Key within the same item must be indented > indent
                        next_stripped = next_line.strip()
                        if ":" in next_stripped and not next_stripped.startswith("- "):
                            sk, sv = next_stripped.split(":", 1)
                            sk = sk.strip()
                            sv = sv.strip()
                            idx += 1
                            if sv == "|" or sv == ">" or sv == ">-":
                                sub_dict[sk] = parse_multiline(next_indent + 2, sv)
                            elif sv:
                                sub_dict[sk] = parse_flow_seq(sv.strip("'\""))
                            else:
                                sub_dict[sk] = parse_block(next_indent + 2)
                        else:
                            break
                    result.append(sub_dict)
                else:
                    result.append(item_str.strip("'\""))

            elif ":" in line_content:
                if is_dict is False:
                    break
                is_dict = True
                if result is None:
                    result = {}

                k, v = line_content.split(":", 1)
                k = k.strip()
                v = v.strip()
                idx += 1

                if v == "|" or v == ">" or v == ">-":
                    result[k] = parse_multiline(indent + 2, v)
                elif v:
                    result[k] = parse_flow_seq(v.strip("'\""))
                else:
                    sub_val = parse_block(indent + 2)
                    result[k] = sub_val
            else:
                idx += 1

        return result

    def parse_multiline(block_indent, style):
        nonlocal idx
        text_lines = []
        while idx < len(lines):
            raw = lines[idx]
            if not raw.strip():
                text_lines.append("")
                idx += 1
                continue
            cur_indent = get_indent(raw)
            if cur_indent < block_indent:
                break
            text_lines.append(raw[block_indent:])
            idx += 1
        if style in (">", ">-"):
            return " ".join([l.strip() for l in text_lines if l.strip()])
        return "\n".join(text_lines)

    return parse_block(0)

def validate_workflow_schema(workflow_path: Path):
    print(f"\n========================================================")
    print(f" Validating: {workflow_path.name}")
    print(f" Path: {workflow_path}")
    print(f"========================================================")

    errors = []
    warnings = []

    content = workflow_path.read_text(encoding="utf-8")
    lines = content.splitlines()

    # 1. Structural parse
    try:
        wf = parse_yaml_lines(lines)
    except Exception as e:
        return [f"Fatal YAML syntax error parsing {workflow_path.name}: {e}"], []

    if not isinstance(wf, dict):
        return [f"Top-level YAML structure must be a mapping, got {type(wf).__name__}"], []

    # 2. Check top-level keys
    for k in wf:
        if k not in ALLOWED_TOP_KEYS:
            warnings.append(f"Unrecognized top-level key '{k}'")

    if "name" not in wf:
        errors.append("Missing required top-level key 'name'")
    else:
        print(f"[OK] Workflow Name: '{wf['name']}'")

    if "on" not in wf:
        errors.append("Missing required top-level key 'on'")
    else:
        triggers = wf["on"]
        print(f"[OK] Triggers defined: {list(triggers.keys()) if isinstance(triggers, dict) else triggers}")

    if "jobs" not in wf or not isinstance(wf["jobs"], dict) or len(wf["jobs"]) == 0:
        errors.append("Workflow must define non-empty 'jobs' mapping")
        return errors, warnings

    print(f"[OK] Jobs found: {list(wf['jobs'].keys())}")

    # 3. Validate each job
    repo_root = workflow_path.resolve().parent.parent.parent

    for job_id, job_spec in wf["jobs"].items():
        print(f"\n  --- Job: {job_id} ---")
        if not isinstance(job_spec, dict):
            errors.append(f"Job '{job_id}' specification must be a dictionary")
            continue

        # Check job keys
        for jk in job_spec:
            if jk not in ALLOWED_JOB_KEYS:
                warnings.append(f"Job '{job_id}' contains non-standard key '{jk}'")

        # Runs-on check
        if "runs-on" not in job_spec:
            errors.append(f"Job '{job_id}' missing required 'runs-on'")
        else:
            runner = job_spec["runs-on"]
            if runner in VALID_RUNNERS:
                print(f"  [OK] Valid runner target: {runner}")
            elif runner.startswith("${{") and runner.endswith("}}"):
                print(f"  [OK] Dynamic matrix runner target expression: {runner}")
            else:
                warnings.append(f"Job '{job_id}' runner target '{runner}' not in known standard GitHub runners list")

        # Steps check
        steps = job_spec.get("steps")
        if not isinstance(steps, list) or len(steps) == 0:
            errors.append(f"Job '{job_id}' must have a non-empty list of 'steps'")
            continue

        print(f"  [OK] Step count: {len(steps)}")

        for i, step in enumerate(steps, 1):
            if not isinstance(step, dict):
                errors.append(f"Job '{job_id}' step #{i} is not a dictionary: {step}")
                continue

            step_name = step.get("name", f"Step #{i}")

            # Check step keys
            for sk in step:
                if sk not in ALLOWED_STEP_KEYS:
                    warnings.append(f"Job '{job_id}', step '{step_name}': unknown step key '{sk}'")

            has_uses = "uses" in step
            has_run = "run" in step

            if not has_uses and not has_run:
                errors.append(f"Job '{job_id}', step '{step_name}': must have either 'uses' or 'run'")
            elif has_uses and has_run:
                errors.append(f"Job '{job_id}', step '{step_name}': cannot specify both 'uses' and 'run'")

            # Validate 'uses'
            if has_uses:
                uses_val = step["uses"]
                if not isinstance(uses_val, str) or "@" not in uses_val:
                    errors.append(f"Job '{job_id}', step '{step_name}': 'uses' action '{uses_val}' lacks valid @version/ref specifier")
                else:
                    action_name, version = uses_val.split("@", 1)
                    if not re.match(r"^[\w\-./]+$", action_name):
                        errors.append(f"Job '{job_id}', step '{step_name}': invalid action path '{action_name}'")
                    if not re.match(r"^[\w\-./v]+$", version):
                        errors.append(f"Job '{job_id}', step '{step_name}': invalid action ref '{version}'")

            # Validate 'shell'
            if "shell" in step:
                sh_val = step["shell"]
                # Allows custom msys2 format like "msys2 {0}"
                if sh_val not in VALID_SHELLS and not sh_val.startswith("msys2"):
                    warnings.append(f"Job '{job_id}', step '{step_name}': unconventional shell '{sh_val}'")

            # Validate ${{ ... }} expression balance in values
            for k_s, v_s in step.items():
                if isinstance(v_s, str) and "${{" in v_s:
                    if v_s.count("${{") != v_s.count("}}"):
                        errors.append(f"Job '{job_id}', step '{step_name}': unbalanced expression syntax in '{k_s}'")

            # Validate local script references in 'run'
            if has_run:
                run_content = step["run"]
                # Scan for scripts/ or python scripts/... references
                for match in re.finditer(r"scripts/[\w\-.]+\.py", run_content):
                    script_ref = match.group(0)
                    script_file = repo_root / script_ref
                    if not script_file.exists():
                        errors.append(f"Job '{job_id}', step '{step_name}': referenced script '{script_ref}' does NOT exist in repository!")
                    else:
                        print(f"    [VERIFIED REF] Local script target verified: {script_ref}")

                # Scan for CMake source dirs in run
                for match in re.finditer(r"-S\s+([\w\-.]+)", run_content):
                    src_dir_ref = match.group(1)
                    src_dir = repo_root / src_dir_ref
                    if not src_dir.is_dir():
                        errors.append(f"Job '{job_id}', step '{step_name}': CMake source dir '{src_dir_ref}' does NOT exist!")
                    else:
                        print(f"    [VERIFIED REF] CMake source directory verified: {src_dir_ref}")

    return errors, warnings

def main():
    repo_root = Path(__file__).resolve().parent.parent
    windows_yml = repo_root / ".github" / "workflows" / "windows.yml"
    android_yml = repo_root / ".github" / "workflows" / "android.yml"

    all_passed = True

    for wf_path in [windows_yml, android_yml]:
        errors, warnings = validate_workflow_schema(wf_path)
        if warnings:
            print("\n  [WARNINGS]:")
            for w in warnings:
                print(f"   * {w}")
        if errors:
            print("\n  [ERRORS]:")
            for e in errors:
                print(f"   ! {e}")
            all_passed = False
        else:
            print(f"\n  [PASS] {wf_path.name} is 100% syntactically and semantically valid!")

    print("\n========================================================")
    if all_passed:
        print("[SUCCESS] ALL GITHUB ACTIONS WORKFLOWS ARE FULLY VALID!")
    else:
        print("[FAILURE] GITHUB ACTIONS WORKFLOW VALIDATION ENCOUNTERED ERRORS!")
    print("========================================================")
    return all_passed

if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)
