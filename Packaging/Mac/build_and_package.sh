#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"

UE_ROOT="${UE_ROOT:-/Users/Shared/Epic Games/UE_5.5}"
CONFIG="Shipping"
ARCHIVE_DIR="${REPO_ROOT}/Dist/Mac"
FORMAT="all"
VERSION=""
SIGN_APP_IDENTITY=""
SIGN_PKG_IDENTITY=""
NOTARIZE=0
KEYCHAIN_PROFILE=""
EXTRA_UAT_ARGS=()

usage() {
  cat <<EOF
Usage: $(basename "$0") [options]

Builds, cooks, packages Foci.app using Unreal Engine 5.5 RunUAT.sh on macOS,
and then generates .pkg and/or .dmg installers via create_installer.sh.

Options:
  --ue-root <path>            Path to UE 5.5 engine root (default: /Users/Shared/Epic Games/UE_5.5)
  --config <Shipping|Development>  Build configuration (default: Shipping)
  --archive-dir <path>        Directory for packaged UE output (default: Dist/Mac)
  --version <ver>             Installer version override
  --format <pkg|dmg|all>      Installer format (default: all)
  --sign-app <identity>       Developer ID Application identity
  --sign-pkg <identity>       Developer ID Installer identity
  --notarize                  Notarize and staple generated installer(s)
  --keychain-profile <name>   notarytool keychain profile (required with --notarize)
  -h, --help                  Show this help message
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --ue-root) UE_ROOT="${2:-}"; shift 2 ;;
    --config) CONFIG="${2:-}"; shift 2 ;;
    --archive-dir) ARCHIVE_DIR="${2:-}"; shift 2 ;;
    --version) VERSION="${2:-}"; shift 2 ;;
    --format) FORMAT="${2:-}"; shift 2 ;;
    --sign-app) SIGN_APP_IDENTITY="${2:-}"; shift 2 ;;
    --sign-pkg) SIGN_PKG_IDENTITY="${2:-}"; shift 2 ;;
    --notarize) NOTARIZE=1; shift ;;
    --keychain-profile) KEYCHAIN_PROFILE="${2:-}"; shift 2 ;;
    -h|--help) usage; exit 0 ;;
    *) EXTRA_UAT_ARGS+=("$1"); shift ;;
  esac
done

if [[ "$(uname -s)" != "Darwin" ]]; then
  echo "Error: $(basename "$0") must be run on macOS (Darwin)." >&2
  exit 1
fi

RUN_UAT="${UE_ROOT}/Engine/Build/BatchFiles/RunUAT.sh"
if [[ ! -x "${RUN_UAT}" ]]; then
  echo "Error: RunUAT.sh not found at '${RUN_UAT}'." >&2
  echo "Set UE_ROOT or pass --ue-root <path-to-UE_5.5>." >&2
  exit 1
fi

UPROJECT="${REPO_ROOT}/Foci.uproject"
if [[ ! -f "${UPROJECT}" ]]; then
  echo "Error: Foci.uproject not found at '${UPROJECT}'." >&2
  exit 1
fi

mkdir -p "${ARCHIVE_DIR}"

echo "[1/2] Packaging Foci for macOS (${CONFIG}) via RunUAT..."
"${RUN_UAT}" BuildCookRun \
  -project="${UPROJECT}" \
  -noP4 \
  -platform=Mac \
  -clientconfig="${CONFIG}" \
  -serverconfig="${CONFIG}" \
  -cook \
  -allmaps \
  -build \
  -stage \
  -pak \
  -archive \
  -archivedirectory="${ARCHIVE_DIR}" \
  -utf8output \
  "${EXTRA_UAT_ARGS[@]}"

APP_PATH=""
for candidate in \
  "${ARCHIVE_DIR}/Mac/Foci.app" \
  "${ARCHIVE_DIR}/MacNoEditor/Foci.app" \
  "${ARCHIVE_DIR}/Foci.app"; do
  if [[ -d "${candidate}" ]]; then
    APP_PATH="${candidate}"
    break
  fi
done

if [[ -z "${APP_PATH}" ]]; then
  echo "Error: Packaged Foci.app not found under '${ARCHIVE_DIR}'." >&2
  exit 1
fi

echo "[2/2] Creating macOS installer from '${APP_PATH}'..."
INSTALLER_ARGS=(
  --app "${APP_PATH}"
  --format "${FORMAT}"
)
[[ -n "${VERSION}" ]] && INSTALLER_ARGS+=(--version "${VERSION}")
[[ -n "${SIGN_APP_IDENTITY}" ]] && INSTALLER_ARGS+=(--sign-app "${SIGN_APP_IDENTITY}")
[[ -n "${SIGN_PKG_IDENTITY}" ]] && INSTALLER_ARGS+=(--sign-pkg "${SIGN_PKG_IDENTITY}")
[[ ${NOTARIZE} -eq 1 ]] && INSTALLER_ARGS+=(--notarize --keychain-profile "${KEYCHAIN_PROFILE}")

"${SCRIPT_DIR}/create_installer.sh" "${INSTALLER_ARGS[@]}"
