# syntax=docker/dockerfile:1.6
#
# QFault reproducibility Dockerfile (per ADR-0017).
#
# Two-stage build:
#   - "build"   stage compiles the project from source against pinned deps
#   - "runtime" stage carries only the test binary + scripts for CI / artifact eval
#
# Build:
#   docker build -t qfault:dev --target build .
#   docker build -t qfault:ci  --target runtime .
#
# Run all tests:
#   docker run --rm qfault:ci ctest --test-dir /qfault/build/gcc13-debug -j --output-on-failure

ARG UBUNTU_TAG=24.04

# ─────────────────────────────────────────────────────────────────────────────
# Stage 1: build
# ─────────────────────────────────────────────────────────────────────────────
FROM ubuntu:${UBUNTU_TAG} AS build

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=UTC

# Pinned compiler versions per cmake/dependency_versions.cmake (gcc-13, clang-18).
RUN apt-get update -qq && \
    apt-get install -y --no-install-recommends \
        ca-certificates \
        git \
        make \
        ninja-build \
        cmake \
        g++-13 \
        clang-18 \
        clang-tidy-18 \
        clang-format-18 \
        python3 \
        python3-pip \
        curl && \
    rm -rf /var/lib/apt/lists/*

# Pinned cmake (the apt-installed cmake on 24.04 is recent enough; we
# upgrade via pip only if we need a newer one for presets — keep this
# explicit so the Dockerfile remains the source of truth).
RUN cmake --version

WORKDIR /qfault

# Copy the project. .dockerignore excludes build/, _deps/, .git/, etc.
COPY . /qfault

# Build all four matrix presets in sequence so the image carries every
# CI binary. Each preset FetchContent's the pinned dependencies.
RUN cmake --preset gcc13-debug    && cmake --build build/gcc13-debug -j "$(nproc)"
RUN cmake --preset gcc13-release  && cmake --build build/gcc13-release -j "$(nproc)"
RUN cmake --preset clang18-debug  && cmake --build build/clang18-debug -j "$(nproc)"
RUN cmake --preset clang18-asan   && cmake --build build/clang18-asan -j "$(nproc)"

# Smoke-run debug tests so the build stage fails fast if anything regressed.
RUN ctest --test-dir build/gcc13-debug --output-on-failure -j "$(nproc)"

# ─────────────────────────────────────────────────────────────────────────────
# Stage 2: runtime  (slim image with only what's needed to re-run tests)
# ─────────────────────────────────────────────────────────────────────────────
FROM ubuntu:${UBUNTU_TAG} AS runtime

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=UTC

# Runtime needs cmake + ctest to drive the test runner; nothing else.
RUN apt-get update -qq && \
    apt-get install -y --no-install-recommends \
        ca-certificates \
        cmake \
        libgcc-s1 \
        libstdc++6 && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /qfault
COPY --from=build /qfault /qfault

# Default command: run the full debug test suite. Override at `docker run` time.
CMD ["ctest", "--test-dir", "/qfault/build/gcc13-debug", "-j", "--output-on-failure"]
