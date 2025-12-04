FROM mcr.microsoft.com/devcontainers/cpp:debian13

ARG REINSTALL_CMAKE_VERSION_FROM_SOURCE="none"

# Optionally install the cmake for vcpkg
COPY reinstall-cmake.sh /tmp/

RUN if [ "${REINSTALL_CMAKE_VERSION_FROM_SOURCE}" != "none" ]; then \
        chmod +x /tmp/reinstall-cmake.sh && /tmp/reinstall-cmake.sh ${REINSTALL_CMAKE_VERSION_FROM_SOURCE}; \
    fi \
    && rm -f /tmp/reinstall-cmake.sh

# [Optional] Uncomment this section to install additional vcpkg ports.
# RUN su vscode -c "${VCPKG_ROOT}/vcpkg install <your-port-name-here>"

COPY get-vulkanSDK.sh /tmp/

RUN chmod +x /tmp/get-vulkanSDK.sh && /tmp/get-vulkanSDK.sh

# [Optional] Uncomment this section to install additional packages.
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install --no-install-recommends  \
    libvulkan1 \
    mesa-vulkan-drivers \
    clang-tools  \
    libglfw3-dev  \
    libglm-dev  \
    libxrandr-dev  \
    libwayland-dev
