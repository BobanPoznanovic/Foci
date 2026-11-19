#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

echo "[1/2] Running UE 5.5 project & header integrity audit..."
python3 "${REPO_ROOT}/Tests/verify_ue_project.py"

echo "[2/2] Compiling and running Foci C++17 simulation test suite..."
clang++ -std=c++17 -O2 -Wall -Wextra -Werror \
  -I"${REPO_ROOT}/Source/Foci" \
  "${REPO_ROOT}/Tests/test_foci_simulation.cpp" \
  -o /tmp/test_foci_simulation

/tmp/test_foci_simulation
echo "ALL FOCI VERIFICATION CHECKS PASSED!"
