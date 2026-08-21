# 发布资产状态

源码安装器和内嵌中文包已更新至 `2026.08.21.1`，适配 Hermes Agent `v0.20.5`（`v2026.8.19`）。

`Hermes-zh-CN-Setup.exe`、`HermesZhCNSetup.exe` 与 `hermes-macos-installer.zip` 尚未在对应构建环境中重建。因此本分支仅供代码审阅，不能合并或作为正式下载发布，直到以下内容都完成：

- 从当前 `install.ps1` 重建 Windows 安装器及其嵌入脚本。
- 从当前 `install.sh` 与 `install.command` 重建 macOS 压缩包。
- 运行 `scripts/verify-release.sh 2026.08.21.1` 并核对产物哈希。
