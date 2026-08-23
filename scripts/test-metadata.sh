#!/usr/bin/env bash
set -euo pipefail

project_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
compiler="${CXX:-c++}"
test_binary="${TMPDIR:-/tmp}/smart-spell-tome-sorting-tests"

"${compiler}" \
    -std=c++20 \
    -Wall \
    -Wextra \
    -Wpedantic \
    -I "${project_root}/src" \
    "${project_root}/src/TomeMetadata.cpp" \
    "${project_root}/tests/TomeMetadataTests.cpp" \
    -o "${test_binary}"

"${test_binary}"
rm -f "${test_binary}"

echo "Tome metadata tests passed."
