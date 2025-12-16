# Referencing the base image defined in your devcontainer.json
ARG BASE_IMAGE="mcr.microsoft.com/devcontainers/cpp:ubuntu-24.04"
FROM ${BASE_IMAGE}

# Install System Dependencies
RUN set -ex; \
    sudo apt-get update && sudo apt-get install -y --no-install-recommends \
        autoconf autoconf-archive automake libtool; \
    sudo apt-get clean && rm -rf /var/lib/apt/lists/*
