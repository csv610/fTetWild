import subprocess
import os
import sys
import time

def run_bench(bin_path, mesh_path):
    cmd = [bin_path, "-i", mesh_path, "-o", "tmp.msh", "--is-quiet"]
    try:
        start_time = time.time()
        subprocess.run(cmd, capture_output=True, text=True, check=True)
        return time.time() - start_time
    except Exception as e:
        print(f"Error processing {mesh_path}: {e}")
        return None

def get_tri_count(mesh_path):
    count = 0
    with open(mesh_path, 'r') as f:
        lines = f.readlines()
        if mesh_path.endswith('.off'):
            # OFF format: second line has counts
            header = lines[1].split()
            if len(header) >= 2:
                return int(header[1])
    return count

def main():
    bin_path = "./FloatTetwild_bin"
    base_mesh = "../tests/bunny.off"
    
    if not os.path.exists(bin_path):
        print(f"Binary not found: {bin_path}")
        return

    # To simulate increasing triangles, we'll use different envelope sizes 
    # and ideal edge lengths which force more refinement/splitting.
    # Alternatively, if we had multiple meshes we'd use them.
    # For now, let's vary the '--lr' parameter to increase output complexity
    # as a proxy for input complexity, OR vary the envelope.
    
    edge_lengths = [0.1, 0.05, 0.025, 0.0125, 0.008]
    
    print(f"{'Target L':<10} | {'Time (s)':<10} | {'Estimated Complexity'}")
    print("-" * 50)
    
    for l in edge_lengths:
        cmd = [bin_path, "-i", base_mesh, "-o", "tmp.msh", "-l", str(l), "--is-quiet"]
        start = time.time()
        result = subprocess.run(cmd, capture_output=True, text=True)
        duration = time.time() - start
        
        # Extract triangle/tetra counts from output if possible
        tets = 0
        for line in result.stdout.split('\n'):
            if "#t =" in line:
                tets = int(line.split("=")[1].strip())
        
        print(f"{l:<10} | {duration:<10.2f} | Output Tets: {tets}")

if __name__ == "__main__":
    main()
