# Referencing the base image defined in your devcontainer.json
ARG BASE_IMAGE="mcr.microsoft.com/devcontainers/cpp:ubuntu-24.04"
FROM ${BASE_IMAGE}

# LibTorch Installation Arguments
ARG LIBTORCH_VERSION="2.3.1"
ARG LIBTORCH_FLAVOR="cpu"
ARG LIBTORCH_FILENAME="libtorch-cxx11-abi-shared-with-deps-${LIBTORCH_VERSION}%2B${LIBTORCH_FLAVOR}.zip"
ARG LIBTORCH_URL="https://download.pytorch.org/libtorch/${LIBTORCH_FLAVOR}/${LIBTORCH_FILENAME}"

# Install System Dependencies and LibTorch
RUN set -ex; \
    sudo apt-get update && sudo apt-get install -y --no-install-recommends \
        autoconf autoconf-archive automake libtool wget unzip; \
    wget -q "${LIBTORCH_URL}" -O /tmp/libtorch.zip; \
    mkdir -p /usr/local/libtorch; \
    unzip -q /tmp/libtorch.zip -d /usr/local/libtorch; \
    rm /tmp/libtorch.zip; \
    sudo apt-get clean && rm -rf /var/lib/apt/lists/*

# Environment Variables (for CMake)
ENV Torch_DIR=/usr/local/libtorch/libtorch/share/cmake/Torch
ENV LD_LIBRARY_PATH=/usr/local/libtorch/libtorch/lib:$LD_LIBRARY_PATH
