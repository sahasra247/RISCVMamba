#!/usr/bin/env python3
"""
Automated Renode Headless Demo Runner
======================================
Automates running the Xmamba CFU demo inside Renode simulation:
  1. Launches Renode simulation of the target CFU Playground project.
  2. Sends interactive menu triggers ('3' -> 't' -> 's').
  3. Captures UART output logs to results/uart_renode.log.
  4. Parses results and checks for PASS markers.
  5. Feeds log into demo_dashboard.py for performance display.

Usage:
  python scripts/run_renode_demo.py --proj-dir ~/CFU-Playground/proj/xmamba
"""

import argparse
import os
from pathlib import Path
import subprocess
import sys
import time

def run_headless_renode(proj_dir: Path, output_log: Path, timeout_sec: int = 120) -> bool:
    print(f"[Renode Demo] Launching Renode in project: {proj_dir}")
    print(f"[Renode Demo] Logging output to: {output_log}")

    output_log.parent.mkdir(parents=True, exist_ok=True)

    expect_script = proj_dir / "build" / "interact.expect"
    env = os.environ.copy()
    env["TARGET"] = "digilent_arty"

    try:
        if expect_script.exists():
            cmd = [str(expect_script), "r", "3", "t", "s"]
            proc = subprocess.Popen(
                cmd,
                cwd=str(proj_dir),
                env=env,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True
            )
            print("[Renode Demo] Running automated menu interaction via interact.expect...")
            stdout_data, _ = proc.communicate(timeout=timeout_sec)
            decoded = stdout_data
        else:
            cmd = ["make", "renode"]
            keystrokes = b"3\nt\ns\n"
            proc = subprocess.Popen(
                cmd,
                cwd=str(proj_dir),
                env=env,
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=False
            )
            print("[Renode Demo] Waiting for Renode initialization...")
            time.sleep(15.0)
            print("[Renode Demo] Sending menu keys ('3', 't', 's')...")
            proc.stdin.write(keystrokes)
            proc.stdin.flush()
            stdout_bytes, _ = proc.communicate(timeout=timeout_sec)
            decoded = stdout_bytes.decode("utf-8", errors="replace")

        output_log.write_text(decoded, encoding="utf-8")

        pass_single = "PASS XMAMBA SINGLE TOKEN" in decoded
        pass_seq = "PASS XMAMBA SEQUENCE AND RESET" in decoded

        if pass_single and pass_seq:
            print("[Renode Demo] SUCCESS: Both Single Token and Sequence tests passed!")
            return True
        elif pass_single:
            print("[Renode Demo] SUCCESS: Single Token test passed!")
            return True
        else:
            print("[Renode Demo] WARNING: PASS markers not detected in output.")
            return False

    except subprocess.TimeoutExpired:
        print("[Renode Demo] Timeout reached, terminating process...")
        proc.kill()
        return False
    except Exception as e:
        print(f"[Renode Demo] Error running Renode: {e}")
        return False

def main():
    parser = argparse.ArgumentParser(description="Automated Renode Demo Runner")
    parser.add_argument("--proj-dir", type=Path, default=None, help="Path to CFU Playground project directory")
    parser.add_argument("--log-out", type=Path, default=Path(__file__).resolve().parent.parent / "results" / "uart_renode.log")
    parser.add_argument("--dashboard", action="store_true", default=True, help="Automatically open dashboard after run")
    args = parser.parse_args()

    pkg_root = Path(__file__).resolve().parent.parent

    if not args.proj_dir or not args.proj_dir.exists():
        print(f"[Notice] Project directory not specified or does not exist.")
        print(f"To run directly on an installed CFU Playground environment:")
        print(f"  python scripts/run_renode_demo.py --proj-dir ~/CFU-Playground/proj/xmamba")
        print(f"\nFalling back to offline demo dashboard demonstration...")
        cmd = [sys.executable, str(pkg_root / "demo" / "demo_dashboard.py"), "--demo-mock"]
        subprocess.run(cmd)
        return

    success = run_headless_renode(args.proj_dir, args.log_out)
    if args.dashboard and args.log_out.exists():
        cmd = [sys.executable, str(pkg_root / "demo" / "demo_dashboard.py"), "--log-file", str(args.log_out)]
        subprocess.run(cmd)

if __name__ == "__main__":
    main()
