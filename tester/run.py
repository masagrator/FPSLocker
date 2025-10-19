#!/usr/bin/env python3

import shutil
import subprocess
import os
import multiprocessing

subprocess.run(['make', 'clean'], check=True)

# Copy folders to /source
shutil.copytree('../source/c4', '/source/c4', dirs_exist_ok=True)
shutil.copytree('../source/rapidyaml', '/source/rapidyaml', dirs_exist_ok=True)
shutil.copytree('../source/asmjit', '/source/asmjit', dirs_exist_ok=True)

# Copy individual files to /source
shutil.copy('../source/asmA64.cpp', '/source')
shutil.copy('../source/asmA64.hpp', '/source')
shutil.copy('../source/Lock.cpp', '/source')
shutil.copy('../source/Lock.hpp', '/source')

# Execute make command
num_cores = multiprocessing.cpu_count()
subprocess.run(['make', f'-j{num_cores}'], check=True)