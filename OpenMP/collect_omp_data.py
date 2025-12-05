import subprocess
import re
import csv
import sys
import matplotlib.pyplot as plt

# --- CONFIGURATION ---
PASSWORD = "9999"      # The hard password
THREAD_COUNTS = [1, 2, 4, 8, 16]
OUTPUT_CSV = "omp_results.csv"
SERIAL_TIME = 17.87    # Your specific serial time

def run_command(command, input_str=None):
    """Runs a shell command and returns output."""
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
    print(f"🚀 Starting OpenMP Data Collection for password: '{PASSWORD}'")
    print("-" * 60)

    # 1. Compile (Mac-specific flags included)
    print("🔨 Compiling omp_cracker.c...")
    # We add -O3 for optimization to ensure fair comparison
    compile_cmd = (
        "gcc -Xpreprocessor -fopenmp omp_cracker.c -o omp_cracker "
        "-I$(brew --prefix openssl)/include -I$(brew --prefix libomp)/include "
        "-L$(brew --prefix openssl)/lib -L$(brew --prefix libomp)/lib "
        "-lssl -lcrypto -lomp -O3"
    )
    
    out, err, code = run_command(compile_cmd)
    
    if code != 0:
        print("❌ Compilation Failed!")
        print(err)
        return

    results = []

    # 2. Run Tests
    for threads in THREAD_COUNTS:
        print(f"🔹 Running with {threads} threads...", end="", flush=True)
        
        # OpenMP takes thread count as first argument
        cmd = f"./omp_cracker {threads}"
        
        # Send password followed by newline
        out, err, code = run_command(cmd, input_str=f"{PASSWORD}\n")
        
        # Parse Execution Time
        if out is None:
            print(" ❌ Failed to run command")
            continue
        match = re.search(r"Execution Time:\s*([0-9\.]+)\s*seconds", out)
        if match:
            time_taken = float(match.group(1))
            speedup = SERIAL_TIME / time_taken
            print(f" ✅ Time: {time_taken:.4f}s | Speedup: {speedup:.2f}x")
            results.append({
                "Threads": threads,
                "Time": time_taken,
                "Speedup": speedup
            })
        else:
            print(" ❌ Failed to parse time")
            if "Password not found" in out:
                print("   (Password not found)")
            else:
                # Print a bit of stderr to help debug
                print(f"   Error output: {err.strip()[:200]}...")

    # 3. Save to CSV
    with open(OUTPUT_CSV, "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=["Threads", "Time", "Speedup"])
        writer.writeheader()
        writer.writerows(results)
    
    print("-" * 60)
    print(f"💾 Results saved to {OUTPUT_CSV}")

    # 4. Generate Graphs
    if len(results) > 0:
        x = [r["Threads"] for r in results]
        y_time = [r["Time"] for r in results]
        y_speedup = [r["Speedup"] for r in results]

        plt.figure(figsize=(12, 5))

        # Graph 1: Execution Time
        plt.subplot(1, 2, 1)
        plt.plot(x, y_time, marker='o', color='red')
        plt.title(f"OpenMP Execution Time ('{PASSWORD}')")
        plt.xlabel("Number of Threads")
        plt.ylabel("Time (seconds)")
        plt.grid(True)
        # Ensure x-axis only shows the specific thread counts we tested
        plt.xticks(THREAD_COUNTS)
        for i, txt in enumerate(y_time):
            plt.annotate(f"{txt:.2f}s", (x[i], y_time[i]), textcoords="offset points", xytext=(0,10), ha='center')

        # Graph 2: Speedup
        plt.subplot(1, 2, 2)
        plt.plot(x, y_speedup, marker='o', color='purple')
        plt.title(f"OpenMP Speedup (vs Serial {SERIAL_TIME}s)")
        plt.xlabel("Number of Threads")
        plt.ylabel("Speedup Factor")
        plt.grid(True)
        plt.xticks(THREAD_COUNTS)
        for i, txt in enumerate(y_speedup):
            plt.annotate(f"{txt:.1f}x", (x[i], y_speedup[i]), textcoords="offset points", xytext=(0,10), ha='center')

        plt.tight_layout()
        plt.savefig("omp_performance_graphs.png")
        print("📊 Graphs saved to omp_performance_graphs.png")
        plt.show()

if __name__ == "__main__":
    main()