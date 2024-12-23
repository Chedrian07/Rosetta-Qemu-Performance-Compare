#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import re
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns

###############################################################################
# 전역 설정
###############################################################################
sns.set(style="whitegrid")

# string_manipulation 제거됨
ENVIRONMENTS = ["qemu_log", "rosetta_log", "native_log"]
BENCHMARKS = [
    "arith_benchmark",
    "array_sorting",
    "file_io_benchmark",
    "hashing_benchmark",
    "matrix_multiplication",
    "memory_allocation",
    "multi_threaded_computation",
    "parallel_computation",
    "prime_numbers",
    "recursive_fibonacci"
]

###############################################################################
# 도우미 함수
###############################################################################
def cap_value(val, lower=0.0, upper=100.0):
    """val이 lower~upper 범위를 벗어나면 범위 내로 보정."""
    if pd.isna(val):
        return val
    return max(lower, min(upper, val))

def parse_sar_log(sar_file):
    """
    sar.log에서 CPU_User%, CPU_System%, CPU_Idle% 추출.
    - 'Average: all  user%  nice%  system%  iowait%  steal%  idle%'
    """
    if not os.path.exists(sar_file) or os.path.getsize(sar_file) == 0:
        return None, None, None

    with open(sar_file, 'r') as f:
        lines = f.readlines()

    avg_line = None
    for line in reversed(lines):
        line_stripped = line.strip()
        if line_stripped.startswith("Average:") and ("all" in line_stripped):
            avg_line = line_stripped
            break
    if not avg_line:
        return None, None, None

    parts = avg_line.split()
    # 예: ["Average:", "all", "10.07", "0.00", "0.10", "0.00", "0.00", "89.84"]
    if len(parts) < 8:
        return None, None, None

    try:
        cpu_user = float(parts[2])
        cpu_system = float(parts[4])
        cpu_idle = float(parts[7])
    except ValueError:
        return None, None, None

    return cpu_user, cpu_system, cpu_idle

def parse_time_log(time_file):
    """
    time.log에서 실행 시간, 세그폴트 여부 파악
    - 'took XX seconds' 정규식
    - 'Segmentation fault' or 'core dumped' → Failed
    """
    if not os.path.exists(time_file) or os.path.getsize(time_file) == 0:
        return None, "No Data"

    with open(time_file, 'r') as f:
        content = f.read()

    match = re.search(r"took\s+([\d\.]+)\s+seconds", content)
    time_taken = None
    if match:
        try:
            time_taken = float(match.group(1))
        except ValueError:
            time_taken = None

    if ("Segmentation fault" in content) or ("core dumped" in content):
        status = "Failed"
    else:
        status = "Success"

    return time_taken, status

def parse_fio_log(fio_file):
    """
    fio.log에서 Read BW, Write BW 추출 (MiB/s)
    - read: IOPS=..., BW=XXX MiB/s
    - write: IOPS=..., BW=YYY MiB/s
    """
    if not os.path.exists(fio_file) or os.path.getsize(fio_file) == 0:
        return None, None

    with open(fio_file, 'r') as f:
        content = f.read()

    read_pattern = re.compile(r'read:\s+IOPS=\d+.*?BW=(\d+\.\d+)\s?(?:MiB/s|MB/s)')
    write_pattern = re.compile(r'write:\s+IOPS=\d+.*?BW=(\d+\.\d+)\s?(?:MiB/s|MB/s)')

    read_bw, write_bw = None, None

    rm = read_pattern.search(content)
    wm = write_pattern.search(content)

    if rm:
        try:
            read_bw = float(rm.group(1))
        except ValueError:
            read_bw = None
    if wm:
        try:
            write_bw = float(wm.group(1))
        except ValueError:
            write_bw = None

    return read_bw, write_bw

def parse_sysbench_log(sysbench_file):
    """sysbench_cpu.log에서 events per second 추출"""
    if not os.path.exists(sysbench_file) or os.path.getsize(sysbench_file) == 0:
        return None

    with open(sysbench_file, 'r') as f:
        content = f.read()

    match = re.search(r"events per second:\s+([\d\.]+)", content)
    if match:
        try:
            return float(match.group(1))
        except ValueError:
            return None
    return None

###############################################################################
# 메인 로직
###############################################################################
def main():
    records = []
    fio_env_data = {}
    sysbench_env_data = {}

    #---------------------------------------------------------------------------
    # 1) 로그 파싱
    #---------------------------------------------------------------------------
    for env_dir in ENVIRONMENTS:
        env_name = env_dir.replace("_log", "")  # 예: "qemu", "rosetta", "native"

        # FIO, Sysbench 로그
        fio_file = os.path.join(env_dir, "fio.log")
        read_bw, write_bw = parse_fio_log(fio_file)
        fio_env_data[env_name] = {"FIO_Read_MBps": read_bw, "FIO_Write_MBps": write_bw}

        sysbench_file = os.path.join(env_dir, "sysbench_cpu.log")
        sysbench_val = parse_sysbench_log(sysbench_file)
        sysbench_env_data[env_name] = {"Sysbench_CPU_Events_Per_Sec": sysbench_val}

        # 개별 벤치마크들
        for bench in BENCHMARKS:
            bench_dir = os.path.join(env_dir, bench)
            sar_file = os.path.join(bench_dir, "sar.log")
            time_file = os.path.join(bench_dir, "time.log")

            cpu_user, cpu_system, cpu_idle = parse_sar_log(sar_file)
            time_taken, status = parse_time_log(time_file)

            rec = {
                "Environment": env_name,
                "Benchmark": bench,
                "CPU_User%": cpu_user,
                "CPU_System%": cpu_system,
                "CPU_Idle%": cpu_idle,
                "Time_Taken_s": time_taken,
                "Status": status
            }
            records.append(rec)

    df = pd.DataFrame(records)

    #---------------------------------------------------------------------------
    # 2) FIO / Sysbench 병합
    #---------------------------------------------------------------------------
    fio_df = pd.DataFrame([
        {
            "Environment": k,
            "FIO_Read_MBps": v["FIO_Read_MBps"],
            "FIO_Write_MBps": v["FIO_Write_MBps"]
        }
        for k, v in fio_env_data.items()
    ])
    sysbench_df = pd.DataFrame([
        {
            "Environment": k,
            "Sysbench_CPU_Events_Per_Sec": v["Sysbench_CPU_Events_Per_Sec"]
        }
        for k, v in sysbench_env_data.items()
    ])

    df_merged = df.merge(fio_df, on="Environment", how="left")
    df_merged = df_merged.merge(sysbench_df, on="Environment", how="left")

    # CPU Idle% 등 보정
    for col in ["CPU_User%", "CPU_System%", "CPU_Idle%"]:
        if col in df_merged.columns:
            df_merged[col] = pd.to_numeric(df_merged[col], errors="coerce")
            df_merged[col] = df_merged[col].apply(lambda x: cap_value(x, 0.0, 100.0))

    # 결과 CSV
    output_dir = "benchmark_visualizations"
    os.makedirs(output_dir, exist_ok=True)

    csv_path = os.path.join(output_dir, "benchmark_summary_final.csv")
    df_merged.to_csv(csv_path, index=False)
    print(f"[INFO] CSV 생성 완료 -> '{csv_path}'")

    #---------------------------------------------------------------------------
    # 3) 개별 메트릭을 별도 차트로 시각화 (Heatmap 대신)
    #    - CPU 관련 (User, System, Idle)
    #    - Time_Taken_s
    #    - FIO (Read/Write)
    #    - Sysbench CPU
    #---------------------------------------------------------------------------
    # (A) Time_Taken_s
    if "Time_Taken_s" in df_merged.columns:
        df_time = df_merged.dropna(subset=["Time_Taken_s"])
        if not df_time.empty:
            plt.figure(figsize=(14, 7))
            sns.barplot(data=df_time, x="Benchmark", y="Time_Taken_s", hue="Environment")
            plt.title("Benchmark Time Comparison Across Environments")
            plt.xlabel("Benchmark")
            plt.ylabel("Time (s)")
            plt.xticks(rotation=45, ha='right')
            plt.legend(title='Environment')
            plt.tight_layout()
            plt.savefig(os.path.join(output_dir, "benchmark_time_comparison.png"))
            plt.show()

    # (B1) CPU_User%
    if "CPU_User%" in df_merged.columns:
        df_user = df_merged.dropna(subset=["CPU_User%"])
        if not df_user.empty:
            plt.figure(figsize=(14, 7))
            sns.barplot(data=df_user, x="Benchmark", y="CPU_User%", hue="Environment")
            plt.title("CPU User Percentage Comparison")
            plt.xlabel("Benchmark")
            plt.ylabel("CPU User (%)")
            plt.xticks(rotation=45, ha='right')
            plt.legend(title='Environment')
            plt.tight_layout()
            plt.savefig(os.path.join(output_dir, "cpu_user_comparison.png"))
            plt.show()

    # (B2) CPU_System%
    if "CPU_System%" in df_merged.columns:
        df_sys = df_merged.dropna(subset=["CPU_System%"])
        if not df_sys.empty:
            plt.figure(figsize=(14, 7))
            sns.barplot(data=df_sys, x="Benchmark", y="CPU_System%", hue="Environment")
            plt.title("CPU System Percentage Comparison")
            plt.xlabel("Benchmark")
            plt.ylabel("CPU System (%)")
            plt.xticks(rotation=45, ha='right')
            plt.legend(title='Environment')
            plt.tight_layout()
            plt.savefig(os.path.join(output_dir, "cpu_system_comparison.png"))
            plt.show()

    # (B3) CPU_Idle%
    if "CPU_Idle%" in df_merged.columns:
        df_idle = df_merged.dropna(subset=["CPU_Idle%"])
        if not df_idle.empty:
            plt.figure(figsize=(14, 7))
            sns.barplot(data=df_idle, x="Benchmark", y="CPU_Idle%", hue="Environment")
            plt.title("CPU Idle Percentage Comparison")
            plt.xlabel("Benchmark")
            plt.ylabel("CPU Idle (%)")
            plt.xticks(rotation=45, ha='right')
            plt.legend(title='Environment')
            plt.tight_layout()
            plt.savefig(os.path.join(output_dir, "cpu_idle_comparison.png"))
            plt.show()

    # (C) FIO Read/Write
    if "FIO_Read_MBps" in df_merged.columns and "FIO_Write_MBps" in df_merged.columns:
        fio_cols = ["Environment", "FIO_Read_MBps", "FIO_Write_MBps"]
        df_fio = df_merged[fio_cols].drop_duplicates().dropna(subset=["Environment"])
        df_fio_melt = df_fio.melt(
            id_vars="Environment",
            value_vars=["FIO_Read_MBps", "FIO_Write_MBps"],
            var_name="IO_Type", value_name="Bandwidth"
        ).dropna(subset=["Bandwidth"])
        if not df_fio_melt.empty:
            plt.figure(figsize=(10, 6))
            sns.barplot(data=df_fio_melt, x="IO_Type", y="Bandwidth", hue="Environment")
            plt.title("FIO Read/Write Bandwidth Comparison")
            plt.xlabel("I/O Type")
            plt.ylabel("Bandwidth (MB/s)")
            plt.legend(title="Environment")
            plt.tight_layout()
            plt.savefig(os.path.join(output_dir, "fio_bandwidth_comparison.png"))
            plt.show()

    # (D) Sysbench CPU
    if "Sysbench_CPU_Events_Per_Sec" in df_merged.columns:
        sb_cols = ["Environment", "Sysbench_CPU_Events_Per_Sec"]
        df_sb = df_merged[sb_cols].drop_duplicates().dropna(subset=["Sysbench_CPU_Events_Per_Sec"])
        if not df_sb.empty:
            plt.figure(figsize=(8, 6))
            sns.barplot(data=df_sb, x="Environment", y="Sysbench_CPU_Events_Per_Sec")
            plt.title("Sysbench CPU Events Per Second Comparison")
            plt.xlabel("Environment")
            plt.ylabel("Events Per Second")
            plt.tight_layout()
            plt.savefig(os.path.join(output_dir, "sysbench_cpu_events_comparison.png"))
            plt.show()

    # (E) Status
    if "Status" in df_merged.columns:
        plt.figure(figsize=(14, 7))
        sns.countplot(data=df_merged, x="Benchmark", hue="Status")
        plt.title("Benchmark Status Comparison")
        plt.xlabel("Benchmark")
        plt.ylabel("Count")
        plt.xticks(rotation=45, ha='right')
        plt.legend(title="Status")
        plt.tight_layout()
        plt.savefig(os.path.join(output_dir, "benchmark_status_comparison.png"))
        plt.show()

    print("[INFO] 모든 단계를 완료했습니다.")

###############################################################################
# 실행부
###############################################################################
if __name__ == "__main__":
    main()