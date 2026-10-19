# Use Ubuntu 24.04 LTS as the base image
FROM ubuntu:24.04

# Set the working directory in the container
WORKDIR /app

# Install cmake and libxml2
RUN apt-get update && \
    apt-get install -y cmake libxml2-dev g++ git && \
    git clone --depth=1 https://github.com/project-chip/connectedhomeip.git /app/matter && \
    git clone --depth=1 https://github.com/one-data-model/playground.git /app/sdf && \
    touch /app/sdf/empty-mapping.json && \
    rm -rf /var/lib/apt/lists/*

COPY . /app

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build

ENTRYPOINT ["/app/build/sdf_matter_converter"]
