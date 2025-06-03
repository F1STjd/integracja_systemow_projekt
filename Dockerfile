# Use Ubuntu 24.04 as base image for better C++26 support
FROM ubuntu:24.04

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive
ENV CXX=g++-14
ENV CC=gcc-14

# Install system dependencies
RUN apt-get update && apt-get install -y \
    # Build essentials
    build-essential \
    cmake \
    ninja-build \
    git \
    pkg-config \
    # Python for cpp-httplib CMake configuration
    python3 \
    python3-pip \
    # C++ compiler with C++26 support
    gcc-14 \
    g++-14 \
    # SQLite3 development libraries
    libsqlite3-dev \
    sqlite3 \
    # Additional utilities
    curl \
    wget \
    && rm -rf /var/lib/apt/lists/*

# Set GCC-14 as default
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-14 100 \
    && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-14 100

# Clone and install cpp-httplib
RUN git clone --depth 1 https://github.com/yhirose/cpp-httplib.git /tmp/cpp-httplib \
    && cd /tmp/cpp-httplib \
    && cmake -B build -DCMAKE_BUILD_TYPE=Release -DHTTPLIB_COMPILE=ON \
    && cmake --build build \
    && cmake --install build \
    && rm -rf /tmp/cpp-httplib

# Clone and install Glaze JSON library
RUN git clone --depth 1 https://github.com/stephenberry/glaze.git /tmp/glaze \
    && cd /tmp/glaze \
    && cmake -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build \
    && cmake --install build \
    && rm -rf /tmp/glaze

# Create application directory
WORKDIR /app

# Copy CMake files first (for better Docker layer caching)
COPY CMakeLists.txt ./
COPY src/CMakeLists.txt ./src/

# Copy source files
COPY src/ ./src/

# Copy the cpp-httplib directory (if it exists locally)
# This ensures we use the same version as in development
COPY cpp-httplib/ ./cpp-httplib/

# Create build directory and configure
RUN cmake -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_STANDARD=26 \
    -DCMAKE_CXX_COMPILER=g++-14 \
    -DCMAKE_C_COMPILER=gcc-14 \
    -G Ninja

# Build the application
RUN cmake --build build --parallel

# Expose the port the app runs on
EXPOSE 8080

# Set the working directory to where the binary is built
WORKDIR /app/build

# Command to run the application
# Note: We need to copy the data.sqlite file to the correct location
CMD ["sh", "-c", "cp /app/src/data.sqlite /app/build/src/ 2>/dev/null || true; ./is_app"]

HEALTHCHECK --interval=30s --timeout=3s --start-period=5s --retries=3 \
    CMD curl -f http://localhost:8080/ || exit 1

LABEL maintainer="Steam Price Comparison App"
LABEL description="Steam price comparison web application with C++26, httplib, SQLite3, and Glaze"
LABEL version="1.0"
