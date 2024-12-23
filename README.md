# Rosetta vs QEMU vs Native Performance Benchmarking

## Overview

This project conducts a comprehensive performance benchmarking analysis comparing **Rosetta**, **QEMU**, and **Native** execution environments on an **Apple Silicon M1 Max** system running **Ubuntu 22.04**. The benchmarks evaluate various computational tasks to assess the efficiency and performance overhead introduced by emulation and translation layers.

## Table of Contents

1. [Project Structure](#project-structure)
2. [Prerequisites](#prerequisites)
3. [Setup and Installation](#setup-and-installation)
4. [Building the Benchmarks](#building-the-benchmarks)
5. [Running the Benchmarks](#running-the-benchmarks)
6. [Visualizing the Results](#visualizing-the-results)
7. [Benchmark Results](#benchmark-results)
8. [Analysis](#analysis)
9. [Conclusion](#conclusion)
10. [Future Work](#future-work)
11. [License](#license)
12. [Contact](#contact)

## Project Structure

Rosetta-Qemu-Performance-Compare/
├── .git/
├── Binary_List
├── Makefile
├── README.md
├── install-package.sh
├── native_log/
├── out/
├── qemu_log/
├── rosetta_log/
├── src/
├── start-test.sh
└── visualize_benchmarks.py

- **.git/**: Git repository metadata.
- **Binary_List**: (Description of the file's purpose if applicable).
- **Makefile**: Defines build targets and compilation instructions.
- **README.md**: This documentation file.
- **install-package.sh**: Script to install necessary packages and dependencies.
- **native_log/**, **qemu_log/**, **rosetta_log/**: Directories containing logs for each environment.
- **out/**: Directory where compiled binaries are placed.
- **src/**: Source code for all benchmark programs.
- **start-test.sh**: Script to initiate benchmark tests across environments.
- **visualize_benchmarks.py**: Python script to parse logs and generate visualizations.

## Prerequisites

Ensure the following software is installed on your system:

- **GCC**: GNU Compiler Collection for compiling C programs.
- **Python 3.6+**: Required to run the visualization script.
- **Python Libraries**:
  - `pandas`
  - `numpy`
  - `matplotlib`
  - `seaborn`

You can install the Python libraries using `pip`:

```bash
pip3 install pandas numpy matplotlib seaborn


Results and Figures

Summary CSV:
	•	The file benchmark_summary_final.csv in the benchmark_visualizations/ folder shows a consolidated table of:
	•	Environment (qemu/rosetta/native)
	•	Benchmark name
	•	CPU usage stats (User%, System%, Idle%)
	•	Execution time (Time_Taken_s)
	•	Status (Success/Failed)
	•	FIO (I/O throughput in MB/s)
	•	Sysbench CPU events per second

Graphs:
	•	In the benchmark_visualizations/ folder, open the following PNG files with any image viewer of your choice:
	•	benchmark_time_comparison.png
Visualizes the elapsed time for each benchmark across all environments.
	•	cpu_user_comparison.png
Shows user-space CPU utilization per benchmark.
	•	cpu_system_comparison.png
Shows kernel/system-level CPU utilization per benchmark.
	•	cpu_idle_comparison.png
Compares idle CPU percentages across benchmarks.
	•	sysbench_cpu_events_comparison.png
Displays CPU events/second from Sysbench tests in each environment.
	•	benchmark_status_comparison.png
Count of success/fail statuses for each benchmark in each environment.

Simply navigate to the benchmark_visualizations/ directory and open the images with your favorite image viewer.


Observations
	1.	Arithmetic-Intensive tasks often incur the highest overhead under QEMU.
	2.	Rosetta performance is close to Native for many tasks, especially smaller ones (e.g., array sorting).
	3.	Sysbench results highlight that QEMU introduces significant overhead in CPU instructions, whereas Rosetta remains closer to native performance.