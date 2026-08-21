#!/usr/bin/env bash
set -euo pipefail

VERSION="${1:-2026.08.21.1}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SITE_WEB_DIR="${SITE_WEB_DIR:-$ROOT/../../promo_work/hermes-zh-cn/web}"

need() {
  command -v "$1" >/dev/null 2>&1 || {
    printf 'missing command: %s\n' "$1" >&2
    exit 1
  }
}

need bash
need python3
need unzip

if command -v shasum >/dev/null 2>&1; then
  sha256() { shasum -a 256 "$@"; }
elif command -v sha256sum >/dev/null 2>&1; then
  sha256() { sha256sum "$@"; }
else
  printf '%s\n' 'missing command: shasum or sha256sum' >&2
  exit 1
fi

cd "$ROOT"

bash -n install.sh
bash -n install.command
python3 -m json.tool platforms.json >/dev/null

python3 - "$VERSION" <<'PY'
import base64
import hashlib
import json
import re
import sys
import tarfile
import tempfile
import zipfile
from pathlib import Path

version = sys.argv[1]
root = Path.cwd()

install_sh = (root / "install.sh").read_text(encoding="utf-8")
if f'PACKAGE_VERSION="{version}"' not in install_sh:
    raise SystemExit("install.sh version mismatch")
for rel in ("install.ps1", "install.sh", "install.command", "install-windows.cmd", "installer/windows/HermesZhCNSetup.c", "installer/windows/HermesZhCNSetup.js"):
    text = (root / rel).read_text(encoding="utf-8", errors="ignore")
    if "fresh-claw/hermes-cn@v2026.06.12.1" in text:
        raise SystemExit(f"{rel} still uses stale pinned fallback tag")

verify_windows = (root / "scripts/verify-windows.ps1").read_text(encoding="utf-8", errors="ignore")
if "Start-Process -FilePath python" in verify_windows or "python -m http.server" in verify_windows:
    raise SystemExit("Windows verifier still depends on Python HTTP server")
if "Start-LocalFileServer" not in verify_windows:
    raise SystemExit("Windows verifier missing PowerShell local file server")

match = re.search(r'DATA = """\n(.*?)\n"""', install_sh, re.S)
if not match:
    raise SystemExit("embedded payload not found")
payload = base64.b64decode("".join(line.strip() for line in match.group(1).splitlines()))

with tempfile.TemporaryDirectory() as directory:
    payload_path = Path(directory) / "payload.tar.gz"
    payload_path.write_bytes(payload)
    with tarfile.open(payload_path, "r:gz") as tar:
        manifest = json.load(tar.extractfile("packages/0.20.x/zh-CN/manifest.json"))
        package = tar.extractfile("packages/0.20.x/zh-CN/zh-cn.min.json").read()
if manifest["version"] != version:
    raise SystemExit("embedded manifest version mismatch")
if hashlib.sha256(package).hexdigest() != manifest["files"][0]["sha256"]:
    raise SystemExit("embedded package sha mismatch")

with zipfile.ZipFile(root / "hermes-macos-installer.zip") as archive:
    zipped_install = archive.read("install.sh").decode("utf-8")
    zipped_command = archive.read("install.command").decode("utf-8")
if f'PACKAGE_VERSION="{version}"' not in zipped_install:
    raise SystemExit("mac zip install.sh version mismatch")
if version not in zipped_command:
    raise SystemExit("mac zip install.command version mismatch")

exe_bytes = (root / "Hermes-zh-CN-Setup.exe").read_bytes()
if version.encode() not in exe_bytes:
    raise SystemExit("windows exe embedded version mismatch")

header_text = (root / "installer/windows/embedded_install_ps1.h").read_text(encoding="utf-8")
header_values = [int(value, 16) for value in re.findall(r"0x([0-9a-fA-F]{2})", header_text)]
embedded_ps1 = bytes(header_values)
install_ps1 = (root / "install.ps1").read_bytes()
if embedded_ps1 != install_ps1:
    raise SystemExit("embedded install.ps1 header mismatch")
for marker in (
    b"Ensure-WindowsNode",
    b"Get-LocalInstallerAssetPaths",
    b"XIAOMA_HERMES_EXE_DIR",
    b"Invoke-CurlDownload",
    b"curl.exe",
    b"node-v22.22.3-win-$Arch.zip",
    b"Ensure-WindowsGitBash",
    b"registry.npmmirror.com/-/binary/git-for-windows/",
    b"MinGit-2.54.0-64-bit.zip",
):
    if marker not in embedded_ps1:
        raise SystemExit(f"embedded install.ps1 missing {marker.decode('utf-8')}")
    if marker not in exe_bytes:
        raise SystemExit(f"windows exe missing {marker.decode('utf-8')}")

print("embedded payload ok")
print("mac zip ok")
print("windows exe marker ok")
print("windows exe payload ok")
PY

if [ -d "$SITE_WEB_DIR" ]; then
  for file in \
    "$SITE_WEB_DIR/latest.json" \
    "$SITE_WEB_DIR/agent.json" \
    "$SITE_WEB_DIR/platforms.json" \
    "$SITE_WEB_DIR/api/resolve" \
    "$SITE_WEB_DIR/api/resolve.sample.json" \
    "$SITE_WEB_DIR/packages/0.20.x/zh-CN/manifest.json"; do
    python3 -m json.tool "$file" >/dev/null
  done
  bash -n "$SITE_WEB_DIR/install.sh"
  bash -n "$SITE_WEB_DIR/install.command"
  cmp -s Hermes-zh-CN-Setup.exe "$SITE_WEB_DIR/Hermes-zh-CN-Setup.exe"
  cmp -s hermes-macos-installer.zip "$SITE_WEB_DIR/hermes-macos-installer.zip"
  python3 - "$VERSION" "$SITE_WEB_DIR" <<'PY'
import json
import sys
from pathlib import Path

version, site = sys.argv[1], Path(sys.argv[2])
latest = json.loads((site / "latest.json").read_text(encoding="utf-8"))
manifest = json.loads((site / "packages/0.20.x/zh-CN/manifest.json").read_text(encoding="utf-8"))
if latest["latest"] != version:
    raise SystemExit("site latest version mismatch")
if manifest["version"] != version:
    raise SystemExit("site manifest version mismatch")
if latest["packages"][0]["sha256"] != manifest["files"][0]["sha256"]:
    raise SystemExit("site package sha mismatch")
if f"v=20260613-exe-offline-3" not in latest["install_windows_exe"]:
    raise SystemExit("windows cache marker missing")
if f"v=20260613-mac-main-1" not in latest["install_macos_zip"]:
    raise SystemExit("mac cache marker missing")
print("site files ok")
PY
fi

sha256 Hermes-zh-CN-Setup.exe hermes-macos-installer.zip
printf 'release verification passed: %s\n' "$VERSION"
