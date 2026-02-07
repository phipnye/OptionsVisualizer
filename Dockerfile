FROM ubuntu:24.04
LABEL maintainer="Philip Nye <phipnye@proton.me>"

# Install build tools and dependencies
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    curl \
    zip \
    unzip \
    tar \
    pkg-config \
    python3-dev \
    python3-pip \
    python3-venv && \
    rm -rf /var/lib/apt/lists/*

# Install vcpkg
WORKDIR /app
RUN git clone https://github.com/microsoft/vcpkg.git && \
    ./vcpkg/bootstrap-vcpkg.sh

# Copy source code and config files
COPY src/ ./src/
COPY include/ ./include/
COPY python/ ./python/
COPY CMakeLists.txt vcpkg.json pyproject.toml ./

# Build and install the project into virtual environment
RUN python3 -m venv ./python/.venv && \
    ./python/.venv/bin/python3 -m pip install --no-cache-dir --upgrade pip setuptools wheel && \
    ./python/.venv/bin/python3 -m pip install --no-cache-dir .


# Run the app
EXPOSE 8050
CMD ["./python/.venv/bin/python3", "python/src/main.py"]
