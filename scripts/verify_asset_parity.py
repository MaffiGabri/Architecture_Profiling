import hashlib
import zipfile
import os
import sys

def get_file_sha256(path):
    with open(path, 'rb') as f:
        return hashlib.sha256(f.read()).hexdigest()

def get_zip_sha256(zip_obj, member_path):
    return hashlib.sha256(zip_obj.read(member_path)).hexdigest()

def main():
    apk_path = 'android/app/build/outputs/apk/debug/app-debug.apk'
    if not os.path.exists(apk_path):
        print(f"ERROR: APK not found at {apk_path}")
        sys.exit(1)

    with zipfile.ZipFile(apk_path, 'r') as z:
        files = [
            ('styles.json', 'shared/data/styles.json', 'build/bin/assets/data/styles.json', 'assets/styles.json')
        ]
        
        image_names = [
            'arch_01_classical.png',
            'arch_02_gothic.png',
            'arch_03_renaissance.png',
            'arch_04_baroque.png',
            'arch_05_art_deco.png',
            'arch_06_bauhaus.png',
            'arch_07_brutalism.png',
            'arch_08_hightech.png',
            'arch_09_deconstructivism.png',
            'arch_10_parametric.png'
        ]
        
        for name in image_names:
            files.append((name, f'shared/images/{name}', f'build/bin/assets/images/{name}', f'assets/{name}'))

        print(f"{'Asset':<30} | {'Shared SHA256':<20} | {'Windows SHA256':<20} | {'APK SHA256':<20} | Parity")
        print("-" * 105)

        all_ok = True
        for name, shared_p, win_p, apk_p in files:
            h_shared = get_file_sha256(shared_p)
            h_win = get_file_sha256(win_p)
            h_apk = get_zip_sha256(z, apk_p)

            match = (h_shared == h_win == h_apk)
            if not match:
                all_ok = False
            status = "MATCH (OK)" if match else "MISMATCH (FAIL)"
            print(f"{name:<30} | {h_shared[:16]}... | {h_win[:16]}... | {h_apk[:16]}... | {status}")
            if not match:
                print(f"  Shared:  {h_shared}")
                print(f"  Windows: {h_win}")
                print(f"  APK:     {h_apk}")

        print("-" * 105)
        if all_ok:
            print("[SUCCESS] All 11 assets (10 PNGs + styles.json) have 100% cryptographic SHA256 parity!")
            sys.exit(0)
        else:
            print("[FAILURE] Cryptographic parity mismatch detected!")
            sys.exit(2)

if __name__ == '__main__':
    main()
