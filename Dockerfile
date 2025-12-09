# --- Build
FROM ubuntu:24.04 as builder
LABEL maintainer="Philip Nye <phipnye@proton.me>"

# Install build tools
RUN apt-get update && 
    # Never prompts the user for choices on installation/configuration of packages
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
    build-essential \
    cmake \
    python3-dev \
    python3-pip
    # Clean up APT cache to keep the layer small
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy source code and config files
COPY . /app

# Build executable
RUN cmake . && \
    make

# Install python requirements for dashboard
WORKDIR /app/app
RUN python3 -m pip install --no-cache-dir -r requirements.txt

# --- Run
