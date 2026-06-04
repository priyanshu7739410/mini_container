# Quick Start & Run Guide
*Commands and steps to compile, execute, and verify your custom container runtime.*

---

## 🚀 1. Compilation and Build

To clean previous builds and compile the container runtime using the optimized C++17 Makefile, run the following commands in your WSL2 shell:

```bash
# Clean up temporary and output files
make clean

# Compile the container runtime binary
make
```
This generates the executable `./mycontainer` in the root of your project directory.

---

## 📂 2. Set Up the Isolated Filesystem

Before running a container, you must prepare the isolated root directory containing the Alpine Linux rootfs (shared libraries and basic binaries):

```bash
# Download and extract the Alpine minirootfs non-interactively
./setup_rootfs.sh
```
This downloads Alpine Linux `3.23.0` and extracts it into the local `./rootfs` folder.

---

## 📦 3. Running the Container

All namespace creations, file mounts, and cgroup resource allocations require superuser permissions.

### Option A: Standard Sudo (Interactive)
```bash
sudo ./mycontainer run /bin/sh
```

### Option B: WSL Root User (Direct)
If you want to run the container directly without password prompts from a Windows-based pipeline or external task runners:
```bash
wsl -d Ubuntu -u root sh -c "cd /mnt/d/containers-from-scratch && ./mycontainer run /bin/sh"
```

---

## 🔍 4. Verification Checklist inside the Container

Once inside the container shell (`/bin/sh`), run these commands to verify that the kernel isolation borders are fully active:

### 1. Hostname Isolation (UTS Namespace)
```bash
hostname
```
* **Expected Output:** `mini-container` (or your custom hostname). The host's original hostname is fully hidden.

### 2. PID Space Isolation (PID Namespace & procfs)
```bash
ps aux
```
* **Expected Output:**
  ```text
  PID   USER     TIME  COMMAND
      1 root      0:00 /bin/sh
      2 root      0:00 ps aux
  ```
  Only the container's shell and the `ps` command are visible. All other host processes are completely hidden.

### 3. Filesystem Isolation (Chroot Jail)
```bash
ls -la /
```
* **Expected Output:** Shows Alpine Linux's light system structure (including `/bin`, `/lib`, `/proc`, etc.). You cannot browse or see any files outside `./rootfs` on the host machine.

---

## 🛠️ 5. Setting Hardware Resource Limits (Cgroups v2)

Use the CLI flags to restrict physical resource consumption.

### Restricting Memory Limit (e.g. 50 Megabytes)
```bash
sudo ./mycontainer run --memory 50M --hostname sandbox /bin/sh
```
* The kernel writes `52428800` bytes to `/sys/fs/cgroup/mycontainer-<PID>/memory.max`.
* If a process inside the container allocates more than 50MB, the Out-Of-Memory (OOM) killer will terminate it immediately.

### Restricting CPU Usage (e.g. 10% of 1 CPU Core)
```bash
sudo ./mycontainer run --cpu "10000 100000" --hostname sandbox /bin/sh
```
* The quota is set to `10000` µs per `100000` µs scheduler period.
* Forces the scheduler to throttle the container's CPU shares once it hits 10% usage of a core.

### Combining Limits
```bash
sudo ./mycontainer run --memory 100M --cpu "20000 100000" --hostname production /bin/sh
```

### list running containers
```bash
find /sys/fs/cgroup -maxdepth 1 -type d | grep mycontainer
```
