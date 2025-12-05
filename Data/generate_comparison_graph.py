import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import os

# --- CONFIGURATION: PATHS TO DATA FILES ---
# Since this script is now INSIDE the 'data' folder, we just use filenames.
SERIAL_CSV = 'serial_results.csv'
OMP_CSV    = 'omp_results.csv'
MPI_CSV    = 'mpi_results.csv'
CUDA_CSV   = 'cuda_performance_results.csv'

def load_data():
    data = []
    
    # 1. Load Serial Baseline
    if os.path.exists(SERIAL_CSV):
        df = pd.read_csv(SERIAL_CSV)
        # Serial file has 'Run' and 'Time'. We take the average.
        avg_time = df['Time'].mean()
        # Add Serial as a "1 Core" entry for the line graph
        data.append({'Technology': 'Serial', 'Cores': 1, 'Time': avg_time})
        print(f"✅ Loaded Serial: {avg_time:.4f}s")
    else:
        print(f"⚠️ Warning: {SERIAL_CSV} not found. Using placeholder 17.8s")
        data.append({'Technology': 'Serial', 'Cores': 1, 'Time': 17.8})

    # 2. Load OpenMP
    if os.path.exists(OMP_CSV):
        df = pd.read_csv(OMP_CSV)
        # OpenMP file has 'Threads' and 'Time'
        for _, row in df.iterrows():
            data.append({'Technology': 'OpenMP', 'Cores': int(row['Threads']), 'Time': row['Time']})
        print(f"✅ Loaded OpenMP data ({len(df)} rows)")
    else:
        print(f"⚠️ Warning: {OMP_CSV} not found.")

    # 3. Load MPI
    if os.path.exists(MPI_CSV):
        df = pd.read_csv(MPI_CSV)
        # MPI file has 'Processes' and 'Time'
        for _, row in df.iterrows():
            data.append({'Technology': 'MPI', 'Cores': int(row['Processes']), 'Time': row['Time']})
        print(f"✅ Loaded MPI data ({len(df)} rows)")
    else:
        print(f"⚠️ Warning: {MPI_CSV} not found.")

    # 4. Load CUDA
    if os.path.exists(CUDA_CSV):
        df = pd.read_csv(CUDA_CSV)
        # CUDA file has 'Execution Time (s)' and 'Password'
        # We want the execution time for the hardest password (e.g., '9999')
        # If '9999' exists, filter for it. Otherwise, find the best time.
        if '9999' in df['Password'].values:
            best_cuda = df[df['Password'] == '9999']['Execution Time (s)'].min()
        else:
            best_cuda = df['Execution Time (s)'].min()
            
        # We plot CUDA as a special entry. Since it uses 1000s of threads, 
        # we assign it a fake 'Cores' X-value (e.g. 18) just to place it on the graph next to 16.
        data.append({'Technology': 'CUDA (GPU)', 'Cores': 18, 'Time': best_cuda}) 
        print(f"✅ Loaded CUDA data: {best_cuda:.4f}s")
    else:
        print(f"⚠️ Warning: {CUDA_CSV} not found.")

    return pd.DataFrame(data)

def plot_comparison(df):
    plt.figure(figsize=(12, 7))
    sns.set_style("whitegrid")
    
    # 1. Plot OpenMP and MPI as Line Charts
    # These show how performance improves as we add CPU cores
    subset = df[df['Technology'].isin(['OpenMP', 'MPI'])]
    sns.lineplot(data=subset, x='Cores', y='Time', hue='Technology', style='Technology', 
                 markers=True, dashes=False, linewidth=3, markersize=10)
    
    # 2. Plot Serial as a horizontal reference line
    serial_time = df[df['Technology'] == 'Serial']['Time'].values[0]
    plt.axhline(y=serial_time, color='gray', linestyle='--', label=f'Serial Baseline ({serial_time:.2f}s)')

    # 3. Plot CUDA as a separate Bar/Point
    if 'CUDA (GPU)' in df['Technology'].values:
        cuda_row = df[df['Technology'] == 'CUDA (GPU)'].iloc[0]
        cuda_time = cuda_row['Time']
        
        # Draw a bar for CUDA
        plt.bar(x=18, height=cuda_time, width=1.5, color='green', alpha=0.7, label='CUDA (GPU)')
        
        # Add label on top of the bar
        plt.text(18, cuda_time + (serial_time * 0.05), f'CUDA\n{cuda_time:.3f}s', 
                 ha='center', va='bottom', fontweight='bold', color='green')

    # Formatting
    plt.title('Final Performance Comparison: CPU (OpenMP/MPI) vs GPU (CUDA)', fontsize=16, fontweight='bold')
    plt.ylabel('Execution Time (seconds) - Lower is Better', fontsize=12)
    plt.xlabel('Number of Processing Units (CPU Cores)', fontsize=12)
    
    # Set X-axis ticks to match our tests (1, 2, 4, 8, 16) + CUDA spot
    plt.xticks([1, 2, 4, 8, 16, 18], ['1', '2', '4', '8', '16', 'GPU'])
    
    plt.legend(title="Technology", fontsize=11)
    plt.tight_layout()
    
    # Save
    output_file = 'master_comparison_graph.png'
    plt.savefig(output_file, dpi=300)
    print(f"\n📊 Graph saved to '{output_file}'")
    plt.show()

if __name__ == "__main__":
    df = load_data()
    if not df.empty:
        plot_comparison(df)
    else:
        print("No data found! Check that CSV files are in the same folder as this script.")