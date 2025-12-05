# Parallel Password Cracker - SHA256 Brute Force

A comparative study of parallel computing approaches for SHA256 password cracking using Serial, OpenMP, MPI, and CUDA implementations.

## 📋 Project Structure

```
Assignment_03/
├── Serial/          # Sequential baseline implementation
├── OpenMP/          # Shared-memory parallelization
├── MPI/             # Distributed-memory parallelization
├── CUDA/            # GPU-accelerated implementation
├── Data/            # Performance results and comparison graphs
└── Screenshots/     # Execution screenshots
```

## 🔧 Prerequisites

### macOS Setup
```bash
# Install OpenSSL
brew install openssl@3

# Install OpenMP
brew install libomp

# Install MPI
brew install open-mpi

# Install Python dependencies
pip3 install matplotlib
```

### CUDA Setup
- NVIDIA GPU with CUDA support
- CUDA Toolkit installed
- Jupyter Notebook for running CUDA implementation

## 🚀 Quick Start

### 1. Serial Implementation
```bash
cd Serial
make
./serial_cracker
# Enter password when prompted (max 4 chars)
```

### 2. OpenMP Implementation
```bash
cd OpenMP
make
./omp_cracker 4  # 4 threads
# Enter password when prompted
```

### 3. MPI Implementation
```bash
cd MPI
mpicc -I/opt/homebrew/opt/openssl@3/include -o mpi_cracker mpi_cracker.c \
      -L/opt/homebrew/opt/openssl@3/lib -lssl -lcrypto
mpirun -n 4 ./mpi_cracker  # 4 processes
# Enter password when prompted
```

### 4. CUDA Implementation
```bash
cd CUDA
jupyter notebook cuda_cracker.ipynb
# Run cells in notebook
```

## 📊 Performance Testing

### Collect Serial Baseline
```bash
cd Serial
python3 collect_serial_data.py
```

### Collect OpenMP Data
```bash
cd OpenMP
python3 collect_omp_data.py
```

### Collect MPI Data
```bash
cd MPI
python3 collect_mpi_data.py
```

### Generate Comparison Graph
```bash
cd Data
python3 generate_comparison_graph.py
```

## 🔑 Features

- **Character Set**: a-z, A-Z, 0-9 (62 characters)
- **Max Password Length**: 4 characters
- **Hash Algorithm**: SHA256
- **Search Space**: Up to 62^4 = 14,776,336 combinations

## ⚙️ Implementation Details

### Serial
- Recursive brute force
- Single-threaded baseline

### OpenMP
- Shared-memory parallelization
- Guided scheduling for load balancing
- Atomic flag for early termination

### MPI
- Distributed-memory parallelization
- Round-robin work distribution
- Collective communication for result gathering

### CUDA
- GPU-accelerated parallel processing
- Massive parallelism with thousands of threads
- Optimized for NVIDIA GPUs

## 📈 Performance Metrics

All implementations measure:
- **Execution Time**: Time to crack password
- **Speedup**: Serial time / Parallel time
- **Efficiency**: Speedup / Number of processors

## 🛠️ Compilation Flags

- **Optimization**: `-O2` or `-O3`
- **OpenMP**: `-Xclang -fopenmp -lomp`
- **OpenSSL**: `-I<path>/include -L<path>/lib -lssl -lcrypto`

## 📝 Notes

- Test password "9999" used for benchmarking
- Results may vary based on hardware
- OpenMP performance depends on CPU cores
- MPI can run on single or multiple machines
- CUDA requires NVIDIA GPU

## 👤 Author

Student ID: IT23230910
Student Name : W.P.C.L.Pathirana
Course: Parallel Computing - Y3S1
Institution: SLIIT

## 📄 License

Academic project for educational purposes.
