#!/usr/bin/env bash

set -e

cd "$(mktemp -d --tmpdir vulkan-XXXXXX)"

VULKAN_SDK_VERSION="$(curl -s https://vulkan.lunarg.com/sdk/latest/linux.txt)"
echo "$VULKAN_SDK_VERSION"
VULKAN_ARCHIVE_NAME="vulkansdk-linux-x86_64-${VULKAN_SDK_VERSION}.tar.xz"

# get the latest SDK
curl -s "https://sdk.lunarg.com/sdk/download/${VULKAN_SDK_VERSION}/linux/vulkan_sdk.tar.xz" -o "${VULKAN_ARCHIVE_NAME}"

# checksum
curl -s "https://sdk.lunarg.com/sdk/sha/${VULKAN_SDK_VERSION}/linux/vulkan_sdk.tar.xz.txt" | sha256sum -c

mkdir -p /opt/vulkan
tar xJf "${VULKAN_ARCHIVE_NAME}" -C /opt/vulkan
ln -s /opt/vulkan/"${VULKAN_SDK_VERSION}" /opt/vulkan/default