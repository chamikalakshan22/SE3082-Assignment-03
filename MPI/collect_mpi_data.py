import subprocess
import re
import csv
import sys
import matplotlib.pyplot as plt

# --- CONFIGURATION ---
PASSWORD = "9999"  # The hard password
PROCESS_COUNTS = [1, 2, 4, 8, 16]
OUTPUT_CSV = "mpi_results.csv"
SERIAL_TIME = 17.87  # Your specific serial time

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
    print(f"🚀 Starting MPI Data Collection for password: '{PASSWORD}'")
    print("-" * 60)

    # 1. Compile
    print("🔨 Compiling mpi_cracker.c...")
    # Note: On Mac, we need to link OpenSSL explicitly
    compile_cmd = "mpicc mpi_cracker.c -o mpi_cracker -I$(brew --prefix openssl)/include -L$(brew --prefix openssl)/lib -lssl -lcrypto"
    out, err, code = run_command(compile_cmd)
    
    if code != 0:
        print("❌ Compilation Failed!")
        print(err)
        return

    results = []

    # 2. Run Tests
    for n in PROCESS_COUNTS:
        print(f"🔹 Running with {n} processes...", end="", flush=True)
        
        # We use --oversubscribe to allow more processes than physical cores
        cmd = f"mpirun --oversubscribe -n {n} ./mpi_cracker"
        
        # Send password followed by newline
        out, err, code = run_command(cmd, input_str=f"{PASSWORD}\n")
        
        # Parse Execution Time
        match = re.search(r"Execution Time:\s*([0-9\.]+)\s*seconds", out) if out else None
        if match:
            time_taken = float(match.group(1))
            speedup = SERIAL_TIME / time_taken
            print(f" ✅ Time: {time_taken:.4f}s | Speedup: {speedup:.2f}x")
            results.append({
                "Processes": n,
                "Time": time_taken,
                "Speedup": speedup
            })
        else:
            print(" ❌ Failed to parse time")
            # Print error if it failed
            if out and "Password not found" in out:
                print("   (Password not found)")
            else:
                print(f"   Error output: {err.strip()[:200]}...")

    # 3. Save to CSV
    with open(OUTPUT_CSV, "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=["Processes", "Time", "Speedup"])
        writer.writeheader()
        writer.writerows(results)
    
    print("-" * 60)
    print(f"💾 Results saved to {OUTPUT_CSV}")

    # 4. Generate Graphs
    if len(results) > 0:
        x = [r["Processes"] for r in results]
        y_time = [r["Time"] for r in results]
        y_speedup = [r["Speedup"] for r in results]

        plt.figure(figsize=(12, 5))

        # Graph 1: Execution Time
        plt.subplot(1, 2, 1)
        plt.plot(x, y_time, marker='o', color='blue')
        plt.title(f"MPI Execution Time ('{PASSWORD}')")
        plt.xlabel("Number of Processes")
        plt.ylabel("Time (seconds)")
        plt.grid(True)
        for i, txt in enumerate(y_time):
            plt.annotate(f"{txt:.2f}s", (x[i], y_time[i]), textcoords="offset points", xytext=(0,10), ha='center')

        # Graph 2: Speedup
        plt.subplot(1, 2, 2)
        plt.plot(x, y_speedup, marker='o', color='green')
        plt.title(f"MPI Speedup (vs Serial {SERIAL_TIME}s)")
        plt.xlabel("Number of Processes")
        plt.ylabel("Speedup Factor")
        plt.grid(True)
        for i, txt in enumerate(y_speedup):
            plt.annotate(f"{txt:.1f}x", (x[i], y_speedup[i]), textcoords="offset points", xytext=(0,10), ha='center')

        plt.tight_layout()
        plt.savefig("mpi_performance_graphs.png")
        print("📊 Graphs saved to mpi_performance_graphs.png")
        plt.show()

if __name__ == "__main__":
    main()