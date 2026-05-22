import subprocess
import time
import os
import sys

def run_bench(mesh_path, threads):
    cmd = ["./bench/ftetwild_bench", mesh_path, str(threads)]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, check=True)
        for line in result.stdout.split('\n'):
            if "TIME:" in line:
                return float(line.split(":")[1].strip().replace('s', ''))
    except Exception as e:
        print(f"Error running with {threads} threads: {e}")
        return None
    return None

def main():
    mesh = "../tests/bunny.off"
    if len(sys.argv) > 1:
        mesh = sys.argv[1]
    
    if not os.path.exists(mesh):
        print(f"Mesh not found: {mesh}")
        return

    # Thread counts to test (powers of 2 up to 16)
    thread_counts = [1, 2, 4, 8, 12, 16]
    
    print(f"{'Threads':<10} | {'Time (s)':<10} | {'Speedup':<10} | {'Efficiency':<10}")
    print("-" * 50)
    
    base_time = None
    
    for t in thread_counts:
        t_time = run_bench(mesh, t)
        if t_time is None:
            continue
            
        if base_time is None:
            base_time = t_time
            speedup = 1.0
            efficiency = 1.0
        else:
            speedup = base_time / t_time
            efficiency = speedup / t
            
        print(f"{t:<10} | {t_time:<10.4f} | {speedup:<10.2f} | {efficiency:<10.2%}")

if __name__ == "__main__":
    main()
