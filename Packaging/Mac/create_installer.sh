#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"

APP_PATH=""
VERSION=""
OUTPUT_DIR="${REPO_ROOT}/Dist/MacInstaller"
FORMAT="all"
SIGN_APP_IDENTITY=""
SIGN_PKG_IDENTITY=""
NOTARIZE=0
KEYCHAIN_PROFILE=""
BUNDLE_ID="com.foci.game"

usage() {
  cat <<EOF
Usage: $(basename "$0") [options]

Creates macOS .pkg and/or .dmg installers from a packaged Foci.app bundle.

Options:
  --app <path>                Path to packaged Foci.app (auto-detected if omitted)
  --version <ver>             Installer version (default: CFBundleShortVersionString or 1.0.0)
  --output <dir>              Output directory (default: Dist/MacInstaller)
  --format <pkg|dmg|all>      Installer format to generate (default: all)
  --sign-app <identity>       Developer ID Application identity for codesign
  --sign-pkg <identity>       Developer ID Installer identity for productbuild
  --notarize                  Submit generated installer(s) to Apple notarization
  --keychain-profile <name>   notarytool keychain profile (required with --notarize)
  -h, --help                  Show this help message
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --app) APP_PATH="${2:-}"; shift 2 ;;
    --version) VERSION="${2:-}"; shift 2 ;;
    --output) OUTPUT_DIR="${2:-}"; shift 2 ;;
    --format) FORMAT="${2:-}"; shift 2 ;;
    --sign-app) SIGN_APP_IDENTITY="${2:-}"; shift 2 ;;
    --sign-pkg) SIGN_PKG_IDENTITY="${2:-}"; shift 2 ;;
    --notarize) NOTARIZE=1; shift ;;
    --keychain-profile) KEYCHAIN_PROFILE="${2:-}"; shift 2 ;;
    -h|--help) usage; exit 0 ;;
    *) echo "Unknown option: $1" >&2; usage; exit 1 ;;
  esac
done

if [[ "$(uname -s)" != "Darwin" ]]; then
  echo "Error: $(basename "$0") must be run on macOS (Darwin)." >&2
  exit 1
fi

case "${FORMAT}" in
  pkg|dmg|all) ;;
  *) echo "Error: --format must be 'pkg', 'dmg', or 'all' (got '${FORMAT}')." >&2; exit 1 ;;
esac

if [[ ${NOTARIZE} -eq 1 && -z "${KEYCHAIN_PROFILE}" ]]; then
  echo "Error: --keychain-profile is required when --notarize is set." >&2
  exit 1
fi

if [[ -z "${APP_PATH}" ]]; then
  for candidate in \
    "${REPO_ROOT}/Dist/Mac/Mac/Foci.app" \
    "${REPO_ROOT}/Dist/Mac/Foci.app" \
    "${REPO_ROOT}/Saved/StagedBuilds/Mac/Foci.app" \
    "${REPO_ROOT}/Binaries/Mac/Foci.app"; do
    if [[ -d "${candidate}" ]]; then
      APP_PATH="${candidate}"
      break
    fi
  done
fi

if [[ -z "${APP_PATH}" || ! -d "${APP_PATH}" ]]; then
  echo "Error: Could not find Foci.app. Run Packaging/Mac/build_and_package.sh first or pass --app <path>." >&2
  exit 1
fi

APP_PATH="$(cd "$(dirname "${APP_PATH}")" && pwd)/$(basename "${APP_PATH}")"
INFO_PLIST="${APP_PATH}/Contents/Info.plist"
if [[ ! -f "${INFO_PLIST}" ]]; then
  echo "Error: Invalid app bundle (missing Contents/Info.plist): ${APP_PATH}" >&2
  exit 1
fi

if [[ -z "${VERSION}" ]]; then
  VERSION="$(/usr/libexec/PlistBuddy -c "Print :CFBundleShortVersionString" "${INFO_PLIST}" 2>/dev/null || echo "1.0.0")"
  if [[ -z "${VERSION}" ]]; then
    VERSION="1.0.0"
  fi
fi

mkdir -p "${OUTPUT_DIR}"
WORK_DIR="$(mktemp -d "${TMPDIR:-/tmp}/foci-installer.XXXXXX")"
cleanup() {
  rm -rf "${WORK_DIR}"
}
trap cleanup EXIT

SIGNED_APP="${WORK_DIR}/Foci.app"
ditto "${APP_PATH}" "${SIGNED_APP}"

if [[ -n "${SIGN_APP_IDENTITY}" ]]; then
  echo "[1/3] Code signing Foci.app with '${SIGN_APP_IDENTITY}'..."
  ENTITLEMENTS="${SCRIPT_DIR}/Foci.entitlements"
  # Sign nested frameworks, dylibs, and helper binaries inside-out before signing the outer bundle
  while IFS= read -r -d '' nested; do
    codesign --force --options runtime --timestamp \
      --sign "${SIGN_APP_IDENTITY}" \
      --entitlements "${ENTITLEMENTS}" \
      "${nested}"
  done < <(find "${SIGNED_APP}/Contents" -type f \( -name "*.dylib" -o -name "*.so" -o -perm -111 \) -print0)

  codesign --force --deep --options runtime --timestamp \
    --sign "${SIGN_APP_IDENTITY}" \
    --entitlements "${ENTITLEMENTS}" \
    "${SIGNED_APP}"

  codesign --verify --deep --strict --verbose=2 "${SIGNED_APP}"
else
  echo "[1/3] Skipping Developer ID app signing (--sign-app not provided)."
fi

notarize_and_staple() {
  local artifact="$1"
  if [[ ${NOTARIZE} -eq 1 ]]; then
    echo "Submitting $(basename "${artifact}") for Apple notarization..."
    xcrun notarytool submit "${artifact}" \
      --keychain-profile "${KEYCHAIN_PROFILE}" \
      --wait
    echo "Stapling notarization ticket to $(basename "${artifact}")..."
    xcrun stapler staple "${artifact}"
  fi
}

if [[ "${FORMAT}" == "pkg" || "${FORMAT}" == "all" ]]; then
  echo "[2/3] Building macOS .pkg installer..."
  PKG_STAGE="${WORK_DIR}/pkg_stage"
  mkdir -p "${PKG_STAGE}"

  COMPONENT_PLIST="${WORK_DIR}/Component.plist"
  pkgbuild --analyze --root "${SIGNED_APP}" "${COMPONENT_PLIST}" >/dev/null 2>&1 || true
  if [[ -f "${COMPONENT_PLIST}" ]]; then
    /usr/libexec/PlistBuddy -c "Set :0:BundleIsRelocatable false" "${COMPONENT_PLIST}" 2>/dev/null || true
    COMPONENT_PLIST_ARG=(--component-plist "${COMPONENT_PLIST}")
  else
    COMPONENT_PLIST_ARG=()
  fi

  COMPONENT_PKG="${PKG_STAGE}/Foci-component.pkg"
  pkgbuild \
    --component "${SIGNED_APP}" \
    "${COMPONENT_PLIST_ARG[@]}" \
    --install-location "/Applications" \
    --identifier "${BUNDLE_ID}.app" \
    --version "${VERSION}" \
    "${COMPONENT_PKG}"

  INSTALL_KBYTES="$(du -sk "${SIGNED_APP}" | awk '{print $1}')"
  DIST_XML="${WORK_DIR}/Distribution.xml"
  sed \
    -e "s/@VERSION@/${VERSION}/g" \
    -e "s/@INSTALL_KBYTES@/${INSTALL_KBYTES}/g" \
    "${SCRIPT_DIR}/Distribution.xml.in" > "${DIST_XML}"

  FINAL_PKG="${OUTPUT_DIR}/Foci-${VERSION}-macOS.pkg"
  PRODUCTBUILD_ARGS=(
    --distribution "${DIST_XML}"
    --resources "${SCRIPT_DIR}/Resources"
    --package-path "${PKG_STAGE}"
    --version "${VERSION}"
  )
  if [[ -n "${SIGN_PKG_IDENTITY}" ]]; then
    PRODUCTBUILD_ARGS+=(--sign "${SIGN_PKG_IDENTITY}")
  fi

  productbuild "${PRODUCTBUILD_ARGS[@]}" "${FINAL_PKG}"
  notarize_and_staple "${FINAL_PKG}"
  echo "Created .pkg installer: ${FINAL_PKG}"
fi

if [[ "${FORMAT}" == "dmg" || "${FORMAT}" == "all" ]]; then
  echo "[3/3] Building macOS .dmg disk image..."
  DMG_STAGE="${WORK_DIR}/dmg_stage"
  mkdir -p "${DMG_STAGE}"
  ditto "${SIGNED_APP}" "${DMG_STAGE}/Foci.app"
  ln -s /Applications "${DMG_STAGE}/Applications"

  FINAL_DMG="${OUTPUT_DIR}/Foci-${VERSION}-macOS.dmg"
  rm -f "${FINAL_DMG}"
  hdiutil create \
    -volname "Foci ${VERSION}" \
    -srcfolder "${DMG_STAGE}" \
    -ov \
    -format UDZO \
    -imagekey zlib-level=9 \
    "${FINAL_DMG}"

  if [[ -n "${SIGN_APP_IDENTITY}" ]]; then
    codesign --force --timestamp --sign "${SIGN_APP_IDENTITY}" "${FINAL_DMG}"
  fi
  notarize_and_staple "${FINAL_DMG}"
  echo "Created .dmg installer: ${FINAL_DMG}"
fi

echo "Done! Installer artifacts written to: ${OUTPUT_DIR}"
