import subprocess
import re
import csv
import sys

# --- CONFIGURATION ---
PASSWORD = "9999"      # Must match parallel tests
OUTPUT_CSV = "serial_results.csv"
RUNS = 3               # Run 3 times to get a stable average

def run_command(command, input_str=None):
    try:
        process = subprocess.Popen(
            command,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            shell=True
        )
        stdout, stderr = process.communicate(input=input_str)
        return stdout, stderr, process.returncode
    except Exception as e:
        return None, str(e), -1

def main():
    print(f"🚀 Starting Serial Baseline Collection for: '{PASSWORD}'")
    print("-" * 60)

    # 1. Compile
    print("🔨 Compiling serial_cracker.c...")
    compile_cmd = (
        "gcc serial_cracker.c -o serial_cracker "
        "-I$(brew --prefix openssl)/include "
        "-L$(brew --prefix openssl)/lib "
        "-lssl -lcrypto -O3"
    )
    
    out, err, code = run_command(compile_cmd)
    
    if code != 0:
        print("❌ Compilation Failed!")
        print(err)
        return

    total_time = 0
    results = []

    # 2. Run Tests
    for i in range(1, RUNS + 1):
        print(f"🔹 Run {i}/{RUNS}...", end="", flush=True)
        
        cmd = "./serial_cracker"
        out, err, code = run_command(cmd, input_str=f"{PASSWORD}\n")
        
        if out:
            match = re.search(r"Execution Time:\s*([0-9\.]+)\s*seconds", out)
        else:
            match = None
            
        if match:
            time_taken = float(match.group(1))
            print(f" ✅ Time: {time_taken:.4f}s")
            total_time += time_taken
            results.append({"Run": i, "Time": time_taken})
        else:
            print(" ❌ Failed to parse time")
            print(f"   Error: {err.strip()[:100]}...")

    avg_time = total_time / RUNS
    print("-" * 60)
    print(f"⭐️ AVERAGE SERIAL TIME: {avg_time:.4f} seconds")
    print(f"(Use this value for SERIAL_TIME in your MPI and OpenMP scripts!)")
    print("-" * 60)

    # 3. Save to CSV
    with open(OUTPUT_CSV, "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=["Run", "Time"])
        writer.writeheader()
        writer.writerows(results)
    
    print(f"💾 Results saved to {OUTPUT_CSV}")

if __name__ == "__main__":
    main()