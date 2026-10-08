<div align="center">
  <h1>🛡️ Microjail</h1>
  <p><b>A Minimal, OCI-Inspired Container Runtime built from scratch in C.</b></p>
  
  [![Language](https://img.shields.io/badge/Language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
  [![OS](https://img.shields.io/badge/OS-Linux-yellow.svg)](https://kernel.org)
  [![Security](https://img.shields.io/badge/Security-Namespaces%20%7C%20Cgroups%20%7C%20Capabilities-red.svg)](#)
</div>

---

**Microjail** is a low-level systems engineering project demonstrating deep operating system literacy and kernel boundary security. By leveraging standard Linux system calls, it replicates the core isolation mechanics found in enterprise engines like Docker and Podman.

## 🏗️ Core Architecture

### 1. Namespace Virtualization
*   **UTS Namespace:** Isolates the system hostname.
*   **PID Namespace:** Isolates the process hierarchy (the container workload runs securely as PID 1).
*   **Mount Namespace & VFS Chroot:** Isolates the filesystem, jailing the workload inside a minimal Alpine Linux `rootfs`.

### 2. Resource Throttling (Cgroups v2)
*   Programmatic manipulation of the Linux `/sys/fs/cgroup` virtual filesystem.
*   Enforces a hard limit on the number of processes to mitigate **fork-bomb** attacks.

### 3. Kernel Security & Hardening (Capabilities)
*   Drops the **Capability Bounding Set** via `prctl` to prevent `execve` from restoring privileges to setuid/root binaries.
*   Strips all thread capabilities via raw `syscall(SYS_capset)`, ensuring the workload runs with UID 0 but possesses zero actual kernel privileges (preventing breakout attacks like arbitrary mounting).

---

## 🚀 Quick Start

### Requirements
- Linux kernel with Cgroups v2 enabled
- `gcc` and `make`

### Building the Runtime
```bash
# 1. Download the minimal Alpine rootfs (requires internet)
./setup_rootfs.sh

# 2. Compile the runtime
make

# 3. Launch the isolated container (Requires sudo to create Namespaces)
sudo ./microjail
```

## 🔒 Security Demonstrations
Once inside the container shell, you can verify the isolation:

1. **Verify PID Isolation:** Run `ps`. You will only see the shell and the `ps` command. You are PID 1.
2. **Verify Filesystem Jail:** Run `ls /`. You will only see the Alpine Linux root files, not the host files.
3. **Verify Capability Drops:** Run `hostname HACKED`. The kernel will block the request with `Operation not permitted`.
