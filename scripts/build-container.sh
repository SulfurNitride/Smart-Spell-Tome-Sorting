#!/usr/bin/env bash
set -euo pipefail

engine=""
if command -v podman >/dev/null 2>&1; then
    engine="podman"
elif command -v docker >/dev/null 2>&1; then
    engine="docker"
else
    echo "Podman or Docker is required." >&2
    exit 1
fi

project_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
mkdir -p "${project_root}/.cache/vcpkg"

"${engine}" build -t smart-spell-tome-sorting-builder -f "${project_root}/Containerfile" "${project_root}"
"${engine}" run --rm \
    -v "${project_root}:/work:Z" \
    -v "${project_root}/.cache/vcpkg:/root/.cache/vcpkg:Z" \
    -v smart-spell-tome-sorting-vcpkg-cache:/vcpkg-cache \
    smart-spell-tome-sorting-builder
