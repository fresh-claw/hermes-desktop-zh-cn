#!/usr/bin/env python3
"""Rebuild distributable Hermes Chinese installer assets from tracked sources."""

from __future__ import annotations

import argparse
import stat
import zipfile
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


def write_embedded_powershell_header() -> None:
    source = (ROOT / "install.ps1").read_bytes()
    values = [f"0x{byte:02x}" for byte in source]
    rows = [values[index : index + 12] for index in range(0, len(values), 12)]
    content = "unsigned char kEmbeddedInstallPs1Utf8[] = {\n"
    content += "".join(f"  {', '.join(row)},\n" for row in rows)
    content += "};\n"
    content += f"unsigned int kEmbeddedInstallPs1Utf8_len = {len(source)};\n"
    (ROOT / "installer/windows/embedded_install_ps1.h").write_text(
        content, encoding="utf-8", newline="\n"
    )


def write_macos_archive() -> None:
    archive_path = ROOT / "hermes-macos-installer.zip"
    entries = ("install.command", "install.sh", "platforms.json", "README.md")
    with zipfile.ZipFile(archive_path, "w", compression=zipfile.ZIP_DEFLATED) as archive:
        for name in entries:
            data = (ROOT / name).read_bytes()
            info = zipfile.ZipInfo(name, date_time=(2026, 8, 21, 0, 0, 0))
            info.compress_type = zipfile.ZIP_DEFLATED
            info.external_attr = (stat.S_IFREG | 0o755) << 16 if name.endswith((".sh", ".command")) else (stat.S_IFREG | 0o644) << 16
            archive.writestr(info, data)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--header-only", action="store_true")
    parser.add_argument("--mac-only", action="store_true")
    args = parser.parse_args()
    if args.header_only and args.mac_only:
        parser.error("--header-only and --mac-only cannot be combined")
    if not args.mac_only:
        write_embedded_powershell_header()
    if not args.header_only:
        write_macos_archive()


if __name__ == "__main__":
    main()
