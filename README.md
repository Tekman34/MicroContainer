# Microjail: A Minimal Container Runtime in C

Microjail is a minimal, OCI-inspired container runtime built from scratch in C for Linux. It demonstrates deep operating system literacy, kernel boundary security, and low-level systems programming by leveraging Namespaces, Cgroups v2, and Linux Capabilities.

## Core Architecture

1. **Namespace Virtualization**
   - **UTS Namespace:** Isolates the system hostname.
   - **PID Namespace:** Isolates the process hierarchy (the container runs as PID 1).
   - **Mount Namespace & VFS Chroot:** Isolates the filesystem, jailing the workload inside a minimal Alpine Linux rootfs.

2. **Resource Throttling (Cgroups v2)**
   - Programmatic manipulation of the Linux `/sys/fs/cgroup` virtual filesystem.
   - Enforces a hard limit on the number of processes (mitigating fork-bomb attacks).

3. **Kernel Security & Hardening (Capabilities)**
   - Drops the **Capability Bounding Set** via `prctl` to prevent `execve` from restoring privileges to setuid/root binaries.
   - Strips all thread capabilities via raw `syscall(SYS_capset)`, ensuring the workload runs with UID 0 but possesses zero actual kernel privileges (preventing breakout attacks like arbitrary mounting).

## Building & Running

### Requirements
- Linux kernel with Cgroups v2 enabled
- GCC and Make

### Quick Start
```bash
# 1. Download the minimal Alpine rootfs (requires internet)
./setup_rootfs.sh

# 2. Compile the runtime
make

# 3. Launch the isolated container (Requires sudo for clone namespace flags)
sudo ./microjail
```

## Threat Models Mitigated
* **Fork-Bombs:** Mitigated via Cgroup `pids.max` limits.
* **Filesystem Traversal:** Mitigated via `chroot` and Mount Namespaces.
* **Root Escalation (The execve Trap):** Mitigated by intentionally clearing the capability bounding set prior to capability dropping, preventing the kernel from automatically restoring root privileges upon executing a shell.
