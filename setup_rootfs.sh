#!/bin/bash
set -e

# Configuration
ALPINE_VERSION="3.23.0"
TARBALL="alpine-minirootfs-${ALPINE_VERSION}-x86_64.tar.gz"
URL="https://dl-cdn.alpinelinux.org/alpine/latest-stable/releases/x86_64/${TARBALL}"
ROOTFS_DIR="rootfs"

echo "=== Mini Container Runtime: Rootfs Setup ==="

# Check if rootfs directory already exists and is non-empty
if [ -d "$ROOTFS_DIR" ] && [ "$(ls -A $ROOTFS_DIR)" ]; then
    echo "Rootfs directory '$ROOTFS_DIR' already exists and is not empty. Skipping download."
    exit 0
fi

# Create rootfs directory
mkdir -p "$ROOTFS_DIR"

# Download the Alpine minirootfs if not already present
if [ ! -f "$TARBALL" ]; then
    echo "Downloading Alpine Linux minirootfs (v${ALPINE_VERSION}) from official CDN..."
    curl -L -o "$TARBALL" "$URL"
else
    echo "Found existing minirootfs tarball: $TARBALL"
fi

# Extract the rootfs
echo "Extracting minirootfs to '$ROOTFS_DIR'..."
# Run without sudo to avoid password prompts, using --no-same-owner
tar -xzf "$TARBALL" --no-same-owner -C "$ROOTFS_DIR"

# Cleanup the tarball to keep directory clean
rm -f "$TARBALL"

echo "=== Rootfs setup completed successfully! ==="
echo "Rootfs is located at: $(pwd)/$ROOTFS_DIR"
