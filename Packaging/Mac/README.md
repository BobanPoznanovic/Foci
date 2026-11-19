# Foci macOS Installer & Packaging

This directory contains the scripts and installer metadata to build, package, sign, notarize, and distribute **Foci** for macOS (`.pkg` and `.dmg`).

## Prerequisites (on macOS)
- **macOS 12.0+** with **Xcode** (15+ recommended) and Command Line Tools installed.
- **Unreal Engine 5.5** installed (default expected path: `/Users/Shared/Epic Games/UE_5.5`, or set `UE_ROOT`).
- *(Optional for distribution)* Apple Developer ID certificates (`Developer ID Application` and `Developer ID Installer`) and a `notarytool` keychain profile.

## Quick Start

### 1. Full Build + Cook + Package + Installer
Run on a Mac with Unreal Engine 5.5 installed:

```bash
./Packaging/Mac/build_and_package.sh --version 1.0.0
```

This runs `RunUAT.sh BuildCookRun` for `Mac` (`Shipping`) and outputs:
- `Dist/MacInstaller/Foci-1.0.0-macOS.pkg` (standard macOS installer placing `Foci.app` into `/Applications`)
- `Dist/MacInstaller/Foci-1.0.0-macOS.dmg` (drag-and-drop disk image with `/Applications` shortcut)

### 2. Create Installer from an Existing `Foci.app`
If you already packaged `Foci.app` from the Unreal Editor (**Platforms > Mac > Package Project**):

```bash
./Packaging/Mac/create_installer.sh --app /path/to/Foci.app --version 1.0.0
```

Use `--format pkg` or `--format dmg` to generate only one format.

### 3. Code Signing & Apple Notarization (for Gatekeeper distribution)
```bash
./Packaging/Mac/build_and_package.sh \
  --version 1.0.0 \
  --sign-app "Developer ID Application: Your Name (TEAMID)" \
  --sign-pkg "Developer ID Installer: Your Name (TEAMID)" \
  --notarize \
  --keychain-profile "AC_PASSWORD"
```

`Foci.entitlements` includes the Hardened Runtime entitlements required by Unreal Engine 5.5 (`allow-jit`, `allow-unsigned-executable-memory`, `disable-library-validation`) as well as USB/Bluetooth entitlements for PS5 DualSense RawInput controllers.
