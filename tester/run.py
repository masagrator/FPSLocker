#!/usr/bin/env python3
"""Assemble and build the FPSLocker patch validator.

The tester links two codebases that both live in a namespace called LOCK:

  * FPSLocker's  source/Lock.cpp  - compiles a .yaml patch into the binary form
  * SaltyNX's    lock.cpp         - loads and applies that binary at runtime

SaltyNX's lock.cpp/lock.hpp are self-contained: the host sandbox the validator
needs lives inside them behind -DHOST_BUILD. Nothing here has to be kept in sync
by hand - editing lock.cpp in SaltyNX is enough.

Their filenames differ only by case, so SaltyNX's copies are placed in a
source/saltynx/ subdirectory. That keeps them from overwriting FPSLocker's
Lock.cpp / Lock.hpp on a case-insensitive filesystem, and makes the include in
main.cpp unambiguous ("saltynx/lock.hpp").

SaltyNX is located in this order:
  1. $SALTYNX_PATH
  2. a sibling checkout at ../../SaltyNX
  3. a shallow clone of $SALTYNX_REPO (branch $SALTYNX_REF)
"""

import multiprocessing
import os
import shutil
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
os.chdir(HERE)

FPSLOCKER_SRC = os.path.join("..", "source")
SALTYNX_REPO = os.environ.get("SALTYNX_REPO", "https://github.com/masagrator/SaltyNX.git")
SALTYNX_REF = os.environ.get("SALTYNX_REF", "")
SALTYNX_SUBDIR = os.path.join("saltysd_core", "source")

# Everything SaltyNX contributes, relative to saltysd_core/source.
SALTYNX_FILES = ["lock.cpp", "lock.hpp"]
SALTYNX_DIRS = ["tinyexpr"]


def fail(msg):
    print(f"x {msg}")
    sys.exit(1)


def ok(msg):
    print(f"- {msg}")


def locate_saltynx():
    explicit = os.environ.get("SALTYNX_PATH")
    if explicit:
        if not os.path.isdir(os.path.join(explicit, SALTYNX_SUBDIR)):
            fail(f"SALTYNX_PATH={explicit} does not look like a SaltyNX checkout")
        ok(f"using SaltyNX from SALTYNX_PATH: {explicit}")
        return explicit

    sibling = os.path.join("..", "..", "SaltyNX")
    if os.path.isdir(os.path.join(sibling, SALTYNX_SUBDIR)):
        ok(f"using sibling SaltyNX checkout: {os.path.abspath(sibling)}")
        return sibling

    dest = os.path.join(HERE, "SaltyNX")
    if os.path.isdir(os.path.join(dest, SALTYNX_SUBDIR)):
        ok("using previously cloned SaltyNX")
        return dest

    cmd = ["git", "clone", "--depth", "1"]
    if SALTYNX_REF:
        cmd += ["--branch", SALTYNX_REF]
    cmd += [SALTYNX_REPO, dest]
    print(f"Cloning SaltyNX ({SALTYNX_REF or 'default branch'})...")
    if subprocess.run(cmd).returncode != 0:
        fail("could not clone SaltyNX")
    ok("SaltyNX cloned")
    return dest


def copy_tree(src, dst):
    if not os.path.isdir(src):
        fail(f"missing directory: {src}")
    shutil.copytree(src, dst, dirs_exist_ok=True)
    ok(f"{os.path.basename(src)}/ -> {dst}")


def copy_file(src, dst_dir):
    if not os.path.isfile(src):
        fail(f"missing file: {src}")
    os.makedirs(dst_dir, exist_ok=True)
    shutil.copy(src, dst_dir)
    ok(f"{os.path.basename(src)} -> {dst_dir}")


def main():
    subprocess.run(["make", "clean"], check=True)

    print("\nCopying FPSLocker sources...")
    for d in ("c4", "rapidyaml", "asmjit"):
        copy_tree(os.path.join(FPSLOCKER_SRC, d), os.path.join("source", d))
    for f in ("asmA64.cpp", "asmA64.hpp", "Lock.cpp", "Lock.hpp"):
        copy_file(os.path.join(FPSLOCKER_SRC, f), "source")

    print("\nCopying SaltyNX sources...")
    saltynx = locate_saltynx()
    src_root = os.path.join(saltynx, SALTYNX_SUBDIR)
    dst_root = os.path.join("source", "saltynx")
    os.makedirs(dst_root, exist_ok=True)
    for f in SALTYNX_FILES:
        copy_file(os.path.join(src_root, f), dst_root)
    for d in SALTYNX_DIRS:
        copy_tree(os.path.join(src_root, d), os.path.join(dst_root, d))

    # Guard against the case-collision this layout exists to prevent.
    for name in ("Lock.cpp", "Lock.hpp"):
        path = os.path.join("source", name)
        if not os.path.isfile(path):
            fail(f"{path} disappeared - SaltyNX's lock.* overwrote FPSLocker's Lock.*")

    jobs = multiprocessing.cpu_count()
    print(f"\nRunning make with {jobs} jobs...")
    subprocess.run(["make", f"-j{jobs}"], check=True)
    print("- build complete: my_program")


if __name__ == "__main__":
    main()
