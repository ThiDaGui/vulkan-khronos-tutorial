FROM mcr.microsoft.com/devcontainers/cpp:debian13

ARG REINSTALL_CMAKE_VERSION_FROM_SOURCE="none"

# Optionally install the cmake for vcpkg
COPY reinstall-cmake.sh /tmp/

RUN if [ "${REINSTALL_CMAKE_VERSION_FROM_SOURCE}" != "none" ]; then \
        chmod +x /tmp/reinstall-cmake.sh && /tmp/reinstall-cmake.sh ${REINSTALL_CMAKE_VERSION_FROM_SOURCE}; \
    fi \
    && rm -f /tmp/reinstall-cmake.sh

    # [Optional] Uncomment this section to install additional packages.
RUN apt-get update && apt-get upgrade -y && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install --no-install-recommends  \
    mesa-vulkan-drivers \
    autoconf automake libtool m4 autoconf-archive \
    libwayland-dev libxkbcommon-dev libegl1-mesa-dev libx11-dev libxft-dev libxext-dev libibus-1.0-dev \
    libxinerama-dev libxcursor-dev xorg-dev libglu1-mesa-dev \
    python3.13-venv python3-jinja2 \
    libltdl-dev \
    wayland-protocols flex bison libx11-xcb-dev "^libxcb.*-dev" libxkbcommon-x11-dev \
    && apt-get autoremove -y && apt-get clean -y && rm -rf /var/lib/apt/lists/*

RUN cd "${VCPKG_ROOT}" && git pull --ff-only

# [Optional] Uncomment this section to install additional vcpkg ports.
# RUN su vscode -c "${VCPKG_ROOT}/vcpkg install glfw3"
