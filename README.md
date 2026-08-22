# Hermes Desktop 中文增强包

面向 Hermes Desktop、Hermes Agent TUI 和 CLI 的中文增强入口。

官网入口：[https://useai.live/hermes](https://useai.live/hermes)

## 安装

请从官网页面选择系统并按引导安装。GitHub 仓库仅用于查看源码、更新记录和问题追踪，不提供直接下载入口。

## 平台策略

| 平台 | 推荐方式 | 说明 |
| --- | --- | --- |
| Windows | 官网引导 | 补官方桌面端，再调用中文增强安装器 |
| macOS | 官网引导 | 生成官方 Hermes.app，再补中文增强 |
| Linux | 官网引导 | 生成官方桌面端，再补中文增强 |

## 翻译范围

- 官方语言配置：`display.language=zh`
- CLI：`hermes_cli/*.py`
- TUI：`ui-tui/src/**/*.ts`、`ui-tui/src/**/*.tsx`
- 网关：`gateway/platforms/*.py`
- ACP：`acp_adapter/*.py`
- 平台插件：`plugins/platforms/*`
- 桌面版关联：官方桌面 App 共享同一 Hermes 核心、配置、会话、技能和 TUI 后端

## 不改的内容

- 模型生成内容
- 第三方工具返回文本
- 用户文件
- API Key
- 官方安装包签名

## 版本

- 源码安装器：2026.08.21.1
- 中文包：2026.08.21.1
- 官方 Hermes Agent：v0.20.5（v2026.8.19）

二进制安装包仍保持上一版；正式替换前必须在 Windows 构建环境中重建并验证。

## 上游

Hermes Agent 是 NousResearch 的 MIT 开源项目：<https://github.com/NousResearch/hermes-agent>
