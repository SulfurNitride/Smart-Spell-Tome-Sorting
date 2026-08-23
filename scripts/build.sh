#!/usr/bin/env bash
set -euo pipefail

cmake \
    -S /work \
    -B /work/build \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE="${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" \
    -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=/work/cmake/windows-msvc-clang.cmake \
    -DVCPKG_APPLOCAL_DEPS=OFF \
    -DVCPKG_OVERLAY_TRIPLETS=/work/cmake/triplets \
    -DVCPKG_TARGET_TRIPLET=x64-windows-clang-linux \
    -DVCPKG_HOST_TRIPLET=x64-linux \
    -DBUILD_TESTING=OFF

cmake --build /work/build --target package_mod --parallel
