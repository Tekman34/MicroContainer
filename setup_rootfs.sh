#!/bin/bash
set -e
echo "Downloading Alpine Linux minirootfs..."
wget -qO alpine.tar.gz https://dl-cdn.alpinelinux.org/alpine/v3.19/releases/x86_64/alpine-minirootfs-3.19.1-x86_64.tar.gz
echo "Extracting to rootfs/..."
mkdir -p rootfs
tar -xzf alpine.tar.gz -C rootfs
rm alpine.tar.gz
echo "Done!"
