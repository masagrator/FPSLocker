#!/usr/bin/env python3

import shutil
import subprocess
import os
import multiprocessing

subprocess.run(['make', 'clean'], check=True)

try:
    # Copy folders to /source
    print("Copying c4...")
    shutil.copytree('../source/c4', 'source/c4', dirs_exist_ok=True)
    print("✓ c4 copied")
    
    print("Copying rapidyaml...")
    shutil.copytree('../source/rapidyaml', 'source/rapidyaml', dirs_exist_ok=True)
    print("✓ rapidyaml copied")
    
    print("Copying asmjit...")
    shutil.copytree('../source/asmjit', 'source/asmjit', dirs_exist_ok=True)
    print("✓ asmjit copied")
    
    # Copy individual files to /source
    print("Copying asmA64.cpp...")
    shutil.copy('../source/asmA64.cpp', 'source')
    print("✓ asmA64.cpp copied")
    
    print("Copying asmA64.hpp...")
    shutil.copy('../source/asmA64.hpp', 'source')
    print("✓ asmA64.hpp copied")
    
    print("Copying Lock.cpp...")
    shutil.copy('../source/Lock.cpp', 'source')
    print("✓ Lock.cpp copied")
    
    print("Copying Lock.hpp...")
    shutil.copy('../source/Lock.hpp', 'source')
    print("✓ Lock.hpp copied")
    
except PermissionError as e:
    print(f"✗ Permission denied: {e}")
    print("Try running with sudo: sudo python3 script.py")
except FileNotFoundError as e:
    print(f"✗ File not found: {e}")
except Exception as e:
    print(f"✗ Error: {e}")

# Execute make command with number of jobs equal to physical cores
num_cores = multiprocessing.cpu_count()
print(f"\nRunning make with {num_cores} jobs...")
subprocess.run(['make', f'-j{num_cores}'], check=True)
print("✓ Make completed successfully")