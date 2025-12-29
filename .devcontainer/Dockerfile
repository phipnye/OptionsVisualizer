FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# Install system tools
RUN apt-get update && apt-get install -y \
    build-essential \
    gcc \
    g++ \
    cmake \
    ninja-build \
    gdb \
    git \
    curl \
    zip \
    unzip \
    tar \
    pkg-config \
    autoconf \
    automake \
    autoconf-archive \
    python3-dev \
    python3.12-venv \
    && rm -rf /var/lib/apt/lists/*

# Install vcpkg into /opt/vcpkg
RUN git clone https://github.com/microsoft/vcpkg.git /opt/vcpkg && \
    /opt/vcpkg/bootstrap-vcpkg.sh && \
    chmod -R 777 /opt/vcpkg

# Set Environment Variables so CLion and CMake can find vcpkg
ENV VCPKG_ROOT=/opt/vcpkg

WORKDIR /home/project
